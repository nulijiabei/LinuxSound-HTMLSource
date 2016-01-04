/**
   Based on code
   Copyright (C) 2007-2009 STMicroelectronics
   Copyright (C) 2007-2009 Nokia Corporation and/or its subsidiary(-ies).
   under the LGPL
*/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/stat.h>
#include <pthread.h>

#include <OMX_Core.h>
#include <OMX_Component.h>
#include <OMX_Types.h>
#include <OMX_Audio.h>

#ifdef RASPBERRY_PI
#include <bcm_host.h>
#endif


#define SPEAKER_FRONT_LEFT            0x00001
#define SPEAKER_FRONT_RIGHT           0x00002
#define WAVE_FORMAT_PCM               0x0001

typedef unsigned char   BYTE;
typedef unsigned short  WORD;
typedef unsigned int  DWORD;

typedef struct tGUID
{
  DWORD Data1;
  WORD  Data2, Data3;
  BYTE  Data4[8];
} __attribute__((__packed__)) GUID;

typedef struct tWAVEFORMATEX
{
    WORD    wFormatTag;
    WORD    nChannels;
    DWORD   nSamplesPerSec;
    DWORD   nAvgBytesPerSec;
    WORD    nBlockAlign;
    WORD    wBitsPerSample;
    WORD    cbSize;
} __attribute__((__packed__)) WAVEFORMATEX, *PWAVEFORMATEX, *LPWAVEFORMATEX;

typedef struct tWAVEFORMATEXTENSIBLE
{
  WAVEFORMATEX Format;
  union
  {
    WORD wValidBitsPerSample;
    WORD wSamplesPerBlock;
    WORD wReserved;
  } Samples;
  DWORD dwChannelMask;
  GUID SubFormat;
} __attribute__((__packed__)) WAVEFORMATEXTENSIBLE;

WAVEFORMATEXTENSIBLE m_wave_header;
static const GUID KSDATAFORMAT_SUBTYPE_PCM = {
  WAVE_FORMAT_PCM,
  0x0000, 0x0010,
  {0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71}
};


OMX_ERRORTYPE err;
OMX_HANDLETYPE handle;
OMX_VERSIONTYPE specVersion, compVersion;

int inFd = 0;
int outFd = 0;
unsigned int filesize;
static OMX_BOOL bEOS=OMX_FALSE;

OMX_U32 inBufferSize;
OMX_U32 outBufferSize;
int numInBuffers;
int numOutBuffers;

pthread_mutex_t mutex;
OMX_STATETYPE currentState = OMX_StateLoaded;
pthread_cond_t stateCond;

void waitFor(OMX_STATETYPE state) {
    pthread_mutex_lock(&mutex);
    while (currentState != state)
	pthread_cond_wait(&stateCond, &mutex);
    fprintf(stderr, "Wait successfully completed\n");
    pthread_mutex_unlock(&mutex);
}

void wakeUp(OMX_STATETYPE newState) {
    pthread_mutex_lock(&mutex);
    currentState = newState;
    pthread_cond_signal(&stateCond);
    pthread_mutex_unlock(&mutex);
}
pthread_mutex_t empty_mutex;
int emptyState = 0;
OMX_BUFFERHEADERTYPE* pEmptyBuffer;
pthread_cond_t emptyStateCond;

void waitForEmpty() {
    pthread_mutex_lock(&empty_mutex);
    while (emptyState == 1)
	pthread_cond_wait(&emptyStateCond, &empty_mutex);
    emptyState = 1;
    pthread_mutex_unlock(&empty_mutex);
}

void wakeUpEmpty(OMX_BUFFERHEADERTYPE* pBuffer) {
    pthread_mutex_lock(&empty_mutex);
    emptyState = 0;
    pEmptyBuffer = pBuffer;
    pthread_cond_signal(&emptyStateCond);
    pthread_mutex_unlock(&empty_mutex);
}

void mutex_init() {
    int n = pthread_mutex_init(&mutex, NULL);
    if ( n != 0) {
	fprintf(stderr, "Can't init state mutex\n");
    }
    n = pthread_mutex_init(&empty_mutex, NULL);
    if ( n != 0) {
	fprintf(stderr, "Can't init empty mutex\n");
    }
}

static void display_help() {
    fprintf(stderr, "Usage: ogg_decode input_file output_file");
}


/** Gets the file descriptor's size
 * @return the size of the file. If size cannot be computed
 * (i.e. stdin, zero is returned)
 */
static int getFileSize(int fd) {

    struct stat input_file_stat;
    int err;

    /* Obtain input file length */
    err = fstat(fd, &input_file_stat);
    if(err){
	fprintf(stderr, "fstat failed",0);
	exit(-1);
    }
    return input_file_stat.st_size;
}

OMX_ERRORTYPE cEventHandler(
			    OMX_HANDLETYPE hComponent,
			    OMX_PTR pAppData,
			    OMX_EVENTTYPE eEvent,
			    OMX_U32 Data1,
			    OMX_U32 Data2,
			    OMX_PTR pEventData) {

    fprintf(stderr, "Hi there, I am in the %s callback\n", __func__);
    if(eEvent == OMX_EventCmdComplete) {
	if (Data1 == OMX_CommandStateSet) {
	    fprintf(stderr, "Component State changed in ", 0);
	    switch ((int)Data2) {
	    case OMX_StateInvalid:
		fprintf(stderr, "OMX_StateInvalid\n", 0);
		break;
	    case OMX_StateLoaded:
		fprintf(stderr, "OMX_StateLoaded\n", 0);
		break;
	    case OMX_StateIdle:
		fprintf(stderr, "OMX_StateIdle\n",0);
		break;
	    case OMX_StateExecuting:
		fprintf(stderr, "OMX_StateExecuting\n",0);
		break;
	    case OMX_StatePause:
		fprintf(stderr, "OMX_StatePause\n",0);
		break;
	    case OMX_StateWaitForResources:
		fprintf(stderr, "OMX_StateWaitForResources\n",0);
		break;
	    }
	    wakeUp((int) Data2);
	} else  if (Data1 == OMX_CommandPortEnable){
     
	} else if (Data1 == OMX_CommandPortDisable){
     
	}
    } else if(eEvent == OMX_EventBufferFlag) {
	if((int)Data2 == OMX_BUFFERFLAG_EOS) {
     
	}
    } else {
	fprintf(stderr, "Event is %i\n", (int)eEvent);
	fprintf(stderr, "Param1 is %i\n", (int)Data1);
	fprintf(stderr, "Param2 is %i\n", (int)Data2);
    }

    return OMX_ErrorNone;
}

OMX_ERRORTYPE cEmptyBufferDone(
			       OMX_HANDLETYPE hComponent,
			       OMX_PTR pAppData,
			       OMX_BUFFERHEADERTYPE* pBuffer) {

    fprintf(stderr, "Hi there, I am in the %s callback.\n", __func__);
    if (bEOS) {
	fprintf(stderr, "Buffers emptied, exiting\n");
	wakeUp(OMX_StateLoaded);
    }
    //wakeUpEmpty(pBuffer);
    //fprintf(stderr, "Exiting callback\n");

    int data_read = read(inFd, pBuffer->pBuffer, inBufferSize);
    pBuffer->nFilledLen = data_read;
    err = OMX_EmptyThisBuffer(handle, pBuffer);

    return OMX_ErrorNone;
}

OMX_ERRORTYPE cFillBufferDone(
			      OMX_HANDLETYPE hComponent,
			      OMX_PTR pAppData,
			      OMX_BUFFERHEADERTYPE* pBuffer) {

    fprintf(stderr, "Hi there, I am in the %s callback.\n", __func__);
    if (bEOS) {
	fprintf(stderr, "Buffers filled, exiting\n");
    }
    write(outFd, pBuffer->pBuffer, pBuffer->nFilledLen);
    err = OMX_FillThisBuffer(handle, pBuffer);
    
    return OMX_ErrorNone;
}


OMX_CALLBACKTYPE callbacks  = { .EventHandler = cEventHandler,
				.EmptyBufferDone = cEmptyBufferDone,
				.FillBufferDone = cFillBufferDone
};

void printState() {
    OMX_STATETYPE state;
    err = OMX_GetState(handle, &state);
    if (err != OMX_ErrorNone) {
	fprintf(stderr, "Error on getting state\n");
	exit(1);
    }
    switch (state) {
    case OMX_StateLoaded: fprintf(stderr, "StateLoaded\n"); break;
    case OMX_StateIdle: fprintf(stderr, "StateIdle\n"); break;
    case OMX_StateExecuting: fprintf(stderr, "StateExecuting\n"); break;
    case OMX_StatePause: fprintf(stderr, "StatePause\n"); break;
    case OMX_StateWaitForResources: fprintf(stderr, "StateWiat\n"); break;
    default:  fprintf(stderr, "State unknown\n"); break;
    }
}


static void setHeader(OMX_PTR header, OMX_U32 size) {
    /* header->nVersion */
    OMX_VERSIONTYPE* ver = (OMX_VERSIONTYPE*)(header + sizeof(OMX_U32));
    /* header->nSize */
    *((OMX_U32*)header) = size;

    /* for 1.2
       ver->s.nVersionMajor = OMX_VERSION_MAJOR;
       ver->s.nVersionMinor = OMX_VERSION_MINOR;
       ver->s.nRevision = OMX_VERSION_REVISION;
       ver->s.nStep = OMX_VERSION_STEP;
    */
    ver->s.nVersionMajor = specVersion.s.nVersionMajor;
    ver->s.nVersionMinor = specVersion.s.nVersionMinor;
    ver->s.nRevision = specVersion.s.nRevision;
    ver->s.nStep = specVersion.s.nStep;
}

void setPCMMode(int portNumber) {
    OMX_AUDIO_PARAM_PCMMODETYPE pcm;
    int num_channels = 2;//1;
    int sample_rate = 0;//44100;
    int bit_depth = 0;//16;

    setHeader(&pcm, sizeof(OMX_AUDIO_PARAM_PCMMODETYPE));
    pcm.nPortIndex = portNumber;

    err = OMX_GetParameter(handle, OMX_IndexParamAudioPcm, &pcm);
    if (err != OMX_ErrorNone) {
	fprintf(stderr, "Error getting PCM mode on port %d\n", portNumber);
	exit(1);
    }

    /*
    pcm.nChannels = num_channels;
    pcm.eNumData = OMX_NumericalDataSigned;
    pcm.eEndian = OMX_EndianLittle;
    pcm.nSamplingRate = sample_rate;
    pcm.bInterleaved = OMX_TRUE;
    pcm.nBitPerSample = bit_depth;
    pcm.ePCMMode = OMX_AUDIO_PCMModeLinear;

    switch(num_channels) {
    case 1:
	pcm.eChannelMapping[0] = OMX_AUDIO_ChannelCF;
	break;
    case 8:
	pcm.eChannelMapping[0] = OMX_AUDIO_ChannelLF;
	pcm.eChannelMapping[1] = OMX_AUDIO_ChannelRF;
	pcm.eChannelMapping[2] = OMX_AUDIO_ChannelCF;
	pcm.eChannelMapping[3] = OMX_AUDIO_ChannelLFE;
	pcm.eChannelMapping[4] = OMX_AUDIO_ChannelLR;
	pcm.eChannelMapping[5] = OMX_AUDIO_ChannelRR;
	pcm.eChannelMapping[6] = OMX_AUDIO_ChannelLS;
	pcm.eChannelMapping[7] = OMX_AUDIO_ChannelRS;
	break;
    case 4:
	pcm.eChannelMapping[0] = OMX_AUDIO_ChannelLF;
	pcm.eChannelMapping[1] = OMX_AUDIO_ChannelRF;
	pcm.eChannelMapping[2] = OMX_AUDIO_ChannelLR;
	pcm.eChannelMapping[3] = OMX_AUDIO_ChannelRR;
	break;
    case 2:
	pcm.eChannelMapping[0] = OMX_AUDIO_ChannelLF;
	pcm.eChannelMapping[1] = OMX_AUDIO_ChannelRF;
	break;
    }
    */

    err = OMX_SetParameter(handle, OMX_IndexParamAudioPcm, &pcm);
    if (err != OMX_ErrorNone) {
	fprintf(stderr, "Error setting PCM mode on port %d\n", portNumber);
	exit(1);
    }
}

void setEncoding(int portNumber, OMX_AUDIO_CODINGTYPE encoding) {
    OMX_AUDIO_PARAM_PORTFORMATTYPE sAudioPortFormat;


    setHeader(&sAudioPortFormat, sizeof(OMX_AUDIO_PARAM_PORTFORMATTYPE));
    sAudioPortFormat.nIndex = 0;
    sAudioPortFormat.nPortIndex = portNumber;
    //sAudioPortFormat.eEncoding = OMX_AUDIO_CodingPCM;
    

    err = OMX_GetParameter(handle, OMX_IndexParamAudioPortFormat, &sAudioPortFormat);
    if (err == OMX_ErrorNoMore) {
	printf("Can't get format\n");
	return;
    }
    sAudioPortFormat.eEncoding = encoding; //OMX_AUDIO_CodingPcm;
    err = OMX_SetParameter(handle, OMX_IndexParamAudioPortFormat, &sAudioPortFormat);
    if (err == OMX_ErrorNoMore) {
	printf("Can't set format\n");
	return;
    }
    printf("Set format on port %d to PCM\n", portNumber);
}

int setNumBuffers(int portNumber) {
    /* Get and check input port information */
    OMX_PARAM_PORTDEFINITIONTYPE sPortDef;
    int nBuffers;
    
    setHeader(&sPortDef, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
    sPortDef.nPortIndex = portNumber;
    err = OMX_GetParameter(handle, OMX_IndexParamPortDefinition, &sPortDef);
    if(err != OMX_ErrorNone){
	fprintf(stderr, "Error in getting OMX_PORT_DEFINITION_TYPE parameter\n", 0);
	exit(1);
    }

    /* Create minimum number of buffers for the port */
    nBuffers = sPortDef.nBufferCountActual = sPortDef.nBufferCountMin;
    fprintf(stderr, "Minimum number of buffers is %d\n", nBuffers);
    err = OMX_SetParameter(handle, OMX_IndexParamPortDefinition, &sPortDef);
    if(err != OMX_ErrorNone){
	fprintf(stderr, "Error in setting OMX_PORT_PARAM_TYPE parameter\n", 0);
	exit(1);
    }
    return nBuffers;
}

void createMinBuffers(int portNumber, OMX_U32 *pBufferSize, OMX_BUFFERHEADERTYPE ***pppBuffers) {
    OMX_PARAM_PORTDEFINITIONTYPE sPortDef;
    OMX_BUFFERHEADERTYPE **ppBuffers;
    int n;
    int nBuffers;

    setHeader(&sPortDef, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
    sPortDef.nPortIndex = portNumber;
    err = OMX_GetParameter(handle, OMX_IndexParamPortDefinition, &sPortDef);
    if(err != OMX_ErrorNone){
	fprintf(stderr, "Error in getting OMX_PORT_DEFINITION_TYPE parameter\n", 0);
	exit(1);
    }

    *pBufferSize = sPortDef.nBufferSize;
    nBuffers = sPortDef.nBufferCountMin;
    fprintf(stderr, "Port %d has %d buffers of size is %d\n", portNumber, nBuffers, *pBufferSize);

    ppBuffers = malloc(nBuffers * sizeof(OMX_BUFFERHEADERTYPE *));
    if (ppBuffers == NULL) {
	fprintf(stderr, "Can't allocate buffers\n");
	exit(1);
    }

    for (n = 0; n < nBuffers; n++) {
	err = OMX_AllocateBuffer(handle, ppBuffers+n, portNumber, NULL,
				 *pBufferSize);
	if (err != OMX_ErrorNone) {
	    fprintf(stderr, "Error on AllocateBuffer is %d\n", err);
	    exit(1);
	}
    }
    *pppBuffers = ppBuffers;
}

int main(int argc, char** argv) {

    OMX_PORT_PARAM_TYPE param;
    OMX_PARAM_PORTDEFINITIONTYPE sPortDef;
    OMX_AUDIO_PORTDEFINITIONTYPE sAudioPortDef;
    OMX_AUDIO_PARAM_PORTFORMATTYPE sAudioPortFormat;
    OMX_AUDIO_PARAM_PCMMODETYPE sPCMMode;
    OMX_BUFFERHEADERTYPE **inBuffers;
    OMX_BUFFERHEADERTYPE **outBuffers;
    OMX_AUDIO_PARAM_MP3TYPE sMP3Mode;

#ifdef RASPBERRY_PI
    char *componentName = "OMX.broadcom.audio_decode";
#endif
#ifdef LIM
    char *componentName = "OMX.limoi.ffmpeg.decode.audio";
#endif
    unsigned char name[OMX_MAX_STRINGNAME_SIZE];
    OMX_UUIDTYPE uid;
    int startPortNumber;
    int nPorts;
    int n;

# ifdef RASPBERRY_PI
    bcm_host_init();
# endif

    fprintf(stderr, "Thread id is %p\n", pthread_self());
    if(argc < 2){
	display_help();
	exit(1);
    }

    inFd = open(argv[1], O_RDONLY);
    if(inFd < 0){
	perror("Error opening input file\n");
	exit(1);
    }
    filesize = getFileSize(inFd);

    outFd = open(argv[2], (O_WRONLY | O_CREAT));
    if(outFd < 0){
	perror("Error opening output file\n");
	exit(1);
    }

    err = OMX_Init();
    if(err != OMX_ErrorNone) {
	fprintf(stderr, "OMX_Init() failed\n", 0);
	exit(1);
    }
    /** Ask the core for a handle to the audio render component
     */
    err = OMX_GetHandle(&handle, componentName, NULL /*app private data */, &callbacks);
    if(err != OMX_ErrorNone) {
	fprintf(stderr, "OMX_GetHandle failed\n", 0);
	exit(1);
    }
    err = OMX_GetComponentVersion(handle, name, &compVersion, &specVersion, &uid);
    if(err != OMX_ErrorNone) {
	fprintf(stderr, "OMX_GetComponentVersion failed\n", 0);
	exit(1);
    }

    /** no other ports to disable */

    /** Get audio port information */
    setHeader(&param, sizeof(OMX_PORT_PARAM_TYPE));
    err = OMX_GetParameter(handle, OMX_IndexParamAudioInit, &param);
    if(err != OMX_ErrorNone){
	fprintf(stderr, "Error in getting OMX_PORT_PARAM_TYPE parameter\n", 0);
	exit(1);
    }
    startPortNumber = ((OMX_PORT_PARAM_TYPE)param).nStartPortNumber;
    nPorts = ((OMX_PORT_PARAM_TYPE)param).nPorts;
    if (nPorts != 2) {
	fprintf(stderr, "Decode device has wrong number of ports: %d\n", nPorts);
	exit(1);
    }

    setEncoding(startPortNumber, OMX_AUDIO_CodingPCM);
    setEncoding(startPortNumber+1, OMX_AUDIO_CodingPCM);
    setPCMMode(startPortNumber);
    //setPCMMode(startPortNumber+1);

    // set passthrough - or send a wav header
#ifdef RASPBERRY_PI
    OMX_CONFIG_BOOLEANTYPE boolType;
    setHeader(&boolType, sizeof( OMX_CONFIG_BOOLEANTYPE));
    boolType.bEnabled = OMX_TRUE;
    err = OMX_SetParameter(handle, OMX_IndexParamBrcmDecoderPassThrough, &boolType);
    if(err != OMX_ErrorNone)
    {
	fprintf(stderr, "OMX_IndexParamBrcmDecoderPassThrough omx_err\n");
	exit(1);
    }
#endif

    printState();;
    
    numInBuffers = setNumBuffers(startPortNumber);
    numOutBuffers = setNumBuffers(startPortNumber+1);

    /* call to put state into idle before allocating buffers */
    err = OMX_SendCommand(handle, OMX_CommandStateSet, OMX_StateIdle, NULL);
    if (err != OMX_ErrorNone) {
	fprintf(stderr, "Error on setting state to idle\n");
	exit(1);
    }
 
    err = OMX_SendCommand(handle, OMX_CommandPortEnable, startPortNumber, NULL);
    if (err != OMX_ErrorNone) {
	fprintf(stderr, "Error on setting port to enabled\n");
	exit(1);
    }
    err = OMX_SendCommand(handle, OMX_CommandPortEnable, startPortNumber+1, NULL);
    if (err != OMX_ErrorNone) {
	fprintf(stderr, "Error on setting port to enabled\n");
	exit(1);
    }

    createMinBuffers(startPortNumber, &inBufferSize, &inBuffers);
    createMinBuffers(startPortNumber+1, &outBufferSize, &outBuffers);


    /* Make sure we've reached Idle state */
    waitFor(OMX_StateIdle);
    fprintf(stderr, "Reached Idle state\n");
    //exit(0);

    /* Now try to switch to Executing state */
    err = OMX_SendCommand(handle, OMX_CommandStateSet, OMX_StateExecuting, NULL);
    if(err != OMX_ErrorNone){
	exit(1);
    }

    /* no buffers emptied yet */
    pEmptyBuffer = NULL;

    /* fill and empty the input buffers */
    
    for (n = 0; n < numInBuffers; n++) {
	//int data_read = read(inFd, inBuffers[n]->pBuffer, inBufferSize);
	// just read the 44 bytes WAV header
	int data_read = read(inFd, inBuffers[n]->pBuffer, 44);
	inBuffers[n]->nFilledLen = data_read;
	printf("Read %d into buffer\n", data_read);
	if (data_read <= 0) {
	    fprintf(stderr, "In the %s no more input data available\n", __func__);
	    inBuffers[n]->nFilledLen = 0;
	    inBuffers[n]->nFlags = OMX_BUFFERFLAG_EOS;
	    bEOS=OMX_TRUE;
	}
	inBuffers[0]->nFlags = OMX_BUFFERFLAG_CODECCONFIG | OMX_BUFFERFLAG_ENDOFFRAME;

	break; // try one buffer only
    }
    

    /* put wav header out first */
    m_wave_header.Samples.wValidBitsPerSample = 32;
    m_wave_header.Samples.wSamplesPerBlock    = 0;
    m_wave_header.Format.nChannels            = 2;
    m_wave_header.dwChannelMask     = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT;
    m_wave_header.Format.nBlockAlign          = 2 * (32
								   >> 3);
    m_wave_header.Format.wFormatTag           = WAVE_FORMAT_PCM;
    m_wave_header.Format.nSamplesPerSec       = 48000;
    m_wave_header.Format.nAvgBytesPerSec      = 2*48000;
    m_wave_header.Format.wBitsPerSample       = 16;
    m_wave_header.Samples.wValidBitsPerSample = 16;
    m_wave_header.Format.cbSize               = 0;
    m_wave_header.SubFormat                   = KSDATAFORMAT_SUBTYPE_PCM;
    
    inBuffers[0]->nOffset = 0;
    inBuffers[0]->nFilledLen = sizeof(m_wave_header);
    memset((unsigned char *)inBuffers[0]->pBuffer, 0x0, inBuffers[0]->nAllocLen);
    memcpy(inBuffers[0]->pBuffer, &m_wave_header,  sizeof(m_wave_header));
    inBuffers[0]->nFlags = OMX_BUFFERFLAG_CODECCONFIG | OMX_BUFFERFLAG_ENDOFFRAME;
    

    for (n = 0; n < numInBuffers; n++) {
	err = OMX_EmptyThisBuffer(handle, inBuffers[n]);
	if (err != OMX_ErrorNone) {
	    fprintf(stderr, "Error on emptying buffer\n");
	    exit(1);
	}
	break; // just the first one
    }

    /* empty and fill the output buffers */
    for (n = 0; n < numOutBuffers; n++) {
	err = OMX_FillThisBuffer(handle, outBuffers[n]);
	if (err != OMX_ErrorNone) {
	    fprintf(stderr, "Error on filling buffer\n");
	    exit(1);
	}
    }

    pEmptyBuffer = inBuffers[0];
    emptyState = 1;
    /* Fill and empty buffer */
    /*
      for (;;) {
      int data_read = read(inFd, pEmptyBuffer->pBuffer, nBufferSize);
      pEmptyBuffer->nFilledLen = data_read;
      pEmptyBuffer->nOffset = 0;
      filesize -= data_read;
      if (data_read <= 0) {
      fprintf(stderr, "In the %s no more input data available\n", __func__);
      pEmptyBuffer->nFilledLen=0;
      pEmptyBuffer->nFlags = OMX_BUFFERFLAG_EOS;
      bEOS=OMX_TRUE;
      }
      fprintf(stderr, "Emptying again buffer %p %d bytes, %d to go\n", pEmptyBuffer, data_read, filesize);
      err = OMX_EmptyThisBuffer(handle, pEmptyBuffer);
      waitForEmpty();
      fprintf(stderr, "Waited for empty\n");
      if (bEOS) {
      fprintf(stderr, "Exiting loop\n");
      break;
      }
      }
    */
    sleep(10);
    waitFor(OMX_StateLoaded);
    fprintf(stderr, "Buffers emptied\n");
    exit(0);
}
