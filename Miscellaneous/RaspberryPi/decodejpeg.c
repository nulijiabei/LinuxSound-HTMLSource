
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

#include <pthread.h>

#include <OMX_Core.h>
#include <OMX_Component.h>
#include <OMX_Types.h>
#include <OMX_Audio.h>
#include <OMX_Video.h>

#ifdef RASPBERRY_PI
#include <bcm_host.h>
#endif


/******************************** OMX stuff ************************/

#define NUM_BUFFERS_USED 32

FILE video_file;
char *video_file_name = "Tai Chi - Yang Style Complete Set.mp4";
char *image_file_name = "jan.jpg";

int inFd = 0;

static OMX_BOOL bEOS=OMX_FALSE;

OMX_U32 inDecodeBufferSize;
OMX_U32 outDecodeBufferSize;
int numInDecodeBuffers;
int numOutDecodeBuffers;

OMX_U32 outReadBufferSize;
int numOutReadBuffers;

OMX_U32 inRenderBufferSize;
int numInRenderBuffers;

OMX_BUFFERHEADERTYPE *inDecodeBuffers[NUM_BUFFERS_USED];
OMX_BUFFERHEADERTYPE *outDecodeBuffers[NUM_BUFFERS_USED];
OMX_BUFFERHEADERTYPE *inRenderBuffers[NUM_BUFFERS_USED];
OMX_BUFFERHEADERTYPE *outReadBuffers[NUM_BUFFERS_USED];

pthread_mutex_t mutex;
OMX_STATETYPE currentState = OMX_StateLoaded;
pthread_cond_t stateCond;

char *state2str(OMX_STATETYPE state) {
    switch (state) {
    case OMX_StateLoaded: return "OMX_StateLoaded";
    case OMX_StateIdle: return "OMX_StateIdle";
    case OMX_StateExecuting: return "OMX_StateExecuting";
    default: return "state";
    }
}
 
void waitFor(OMX_STATETYPE state) {
    pthread_mutex_lock(&mutex);
    while (currentState != state)
	pthread_cond_wait(&stateCond, &mutex);
    printf("Wait successfully completed\n");
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

OMX_VERSIONTYPE specVersion, compVersion;
OMX_UUIDTYPE uid;

static const struct
{
    const char *role;
    const char *name;
} role_mappings[] =
{
#ifdef RASPBERRY_PI
    { "video_decoder.avc", "OMX.broadcom.video_decode" },
    { "video_decoder.mpeg2", "OMX.broadcom.video_decode" },
    { "image_decoder.jpeg", "OMX.broadcom.image_decode" },
    { "image_reader.jpeg", "OMX.broadcom.image_read" },
    { "iv_renderer", "OMX.broadcom.video_render" },
#endif
    { 0, 0 }
};

char *error2str(int err) {
    switch (err) {
    case 0x80001000: return "OMX_ErrorInsufficientResources";
    case 0x80001017: return "OMX_ErrorIncorrectStateTransition";
    case 0x80001018: return "OMX_ErrorIncorrectStateOperation";
    case 0x8000100B: return "OMX_ErrorStreamCorrupt";

    default: return "err";
    }

}

static void setHeader(OMX_PTR header, OMX_U32 size) {
    /* header->nVersion */
    OMX_VERSIONTYPE* ver = (OMX_VERSIONTYPE*)(header + sizeof(OMX_U32));
    /* header->nSize */
    *((OMX_U32*)header) = size;

    /* for 1.2 */
       ver->s.nVersionMajor = OMX_VERSION_MAJOR;
       ver->s.nVersionMinor = OMX_VERSION_MINOR;
       ver->s.nRevision = OMX_VERSION_REVISION;
       ver->s.nStep = OMX_VERSION_STEP;
       /*
    ver->s.nVersionMajor = specVersion.s.nVersionMajor;
    ver->s.nVersionMinor = specVersion.s.nVersionMinor;
    ver->s.nRevision = specVersion.s.nRevision;
    ver->s.nStep = specVersion.s.nStep;
       */
}

//OMX_HANDLETYPE decodeHandle;
OMX_HANDLETYPE decodeImageHandle;
//OMX_HANDLETYPE renderHandle;
//OMX_HANDLETYPE readImageHandle;

OMX_ERRORTYPE cEventHandler(
                            OMX_HANDLETYPE hComponent,
                            OMX_PTR pAppData,
                            OMX_EVENTTYPE eEvent,
                            OMX_U32 Data1,
                            OMX_U32 Data2,
                            OMX_PTR pEventData) {
   OMX_ERRORTYPE err;

    printf("Hi there, I am in the %s callback\n", __func__);
    if(eEvent == OMX_EventCmdComplete) {
	if (Data1 == OMX_CommandStateSet) {
	    printf("Component State changed in ");
	    switch ((int)Data2) {
	    case OMX_StateInvalid:
		printf("OMX_StateInvalid\n");
		break;
	    case OMX_StateLoaded:
		printf("OMX_StateLoaded\n");
		break;
	    case OMX_StateIdle:
		printf("OMX_StateIdle\n",0);
		break;
	    case OMX_StateExecuting:
		printf("OMX_StateExecuting\n",0);
		break;
	    case OMX_StatePause:
		printf("OMX_StatePause\n",0);
		break;
	    case OMX_StateWaitForResources:
		printf("OMX_StateWaitForResources\n",0);
		break;
	    }
	    wakeUp((int) Data2);
	} else  if (Data1 == OMX_CommandPortEnable){
	    printf("OMX State Port enabled\n");
	} else if (Data1 == OMX_CommandPortDisable){
	    printf("OMX State Port disabled\n");     
	}
    } else if(eEvent == OMX_EventBufferFlag) {
	if((int)Data2 == OMX_BUFFERFLAG_EOS) {
     
	}
    } else if(eEvent == OMX_EventError) {
      if (Data1 == OMX_ErrorSameState) {
	printf("Already in requested state\n");
      } else {
	  printf("Event is Error %X %s\n", Data1, error2str(Data1));
      }
    } else  if(eEvent == OMX_EventMark) {
	printf("Event is Buffer Mark\n");
    } else  if(eEvent == OMX_EventPortSettingsChanged) {
	/* See 1.1 spec, section 8.9.3.1 Playback Use Case */
	OMX_PARAM_PORTDEFINITIONTYPE sPortDef;

	setHeader(&sPortDef, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
	sPortDef.nPortIndex = Data1;

	err = OMX_GetConfig(hComponent, OMX_IndexParamPortDefinition, &sPortDef);
	if(err != OMX_ErrorNone){
	    fprintf(stderr, "Error in getting OMX_PORT_DEFINITION_TYPE parameter\n");
	    exit(1);
	}
	printf("Event is Port Settings Changed on port %d in component %p\n",
	       Data1, hComponent);
	printf("Port has %d buffers of size is %d\n",  sPortDef.nBufferCountMin, sPortDef.nBufferSize);
    } else {
	printf("Event is %i\n", (int)eEvent);
	printf("Param1 is %i\n", (int)Data1);
	printf("Param2 is %i\n", (int)Data2);
    }

    return OMX_ErrorNone;
}

OMX_ERRORTYPE cDecodeEmptyBufferDone(
                               OMX_HANDLETYPE hComponent,
                               OMX_PTR pAppData,
                               OMX_BUFFERHEADERTYPE* pBuffer) {

      int n;
    OMX_ERRORTYPE err;

    printf("Hi there, I am in the %s callback, buffer %p.\n", __func__, pBuffer);
    if (bEOS) {
	printf("Buffers emptied, exiting\n");
	wakeUp(OMX_StateLoaded);
	exit(0);
    }
    printf("  left in buffer: %d\n", pBuffer->nFilledLen);

    /* put data into the buffer, and then empty it */
    int data_read = read(inFd, pBuffer->pBuffer, inDecodeBufferSize);
    if (data_read <= 0) {
	bEOS = 1;
    }
    printf("  filled buffer %p with %d\n", pBuffer, data_read);
    pBuffer->nFilledLen = data_read;

    err = OMX_EmptyThisBuffer(decodeImageHandle, pBuffer);
    if (err != OMX_ErrorNone) {
	fprintf(stderr, "Error on emptying decode buffer %X %s\n", err, error2str(err));
	exit(1);
    }

    return OMX_ErrorNone;
}

OMX_ERRORTYPE cDecodeFillBufferDone(
			      OMX_HANDLETYPE hComponent,
			      OMX_PTR pAppData,
			      OMX_BUFFERHEADERTYPE* pBuffer) {
    OMX_BUFFERHEADERTYPE* pRenderBuffer;
    int n;
    OMX_ERRORTYPE err;

    printf("Hi there, I am in the %s callback, buffer %p with %d bytes.\n", 
	   __func__, pBuffer, pBuffer->nFilledLen);
    if (bEOS) {
	printf("Buffers filled, exiting\n");
    }


    return OMX_ErrorNone;
}


OMX_CALLBACKTYPE decodeCallbacks  = { .EventHandler = cEventHandler,
				.EmptyBufferDone = cDecodeEmptyBufferDone,
				.FillBufferDone = cDecodeFillBufferDone
};


void printState(OMX_HANDLETYPE handle) {
    OMX_STATETYPE state;
    OMX_ERRORTYPE err;

    err = OMX_GetState(handle, &state);
    if (err != OMX_ErrorNone) {
	fprintf(stderr, "Error on getting state\n");
	exit(1);
    }
    switch (state) {
    case OMX_StateLoaded: printf("StateLoaded\n"); break;
    case OMX_StateIdle: printf("StateIdle\n"); break;
    case OMX_StateExecuting: printf("StateExecuting\n"); break;
    case OMX_StatePause: printf("StatePause\n"); break;
    case OMX_StateWaitForResources: printf("StateWiat\n"); break;
    default:  printf("State unknown\n"); break;
    }
}


/*
   *pBuffer is set to non-zero if a particular buffer size is required by the client
 */
void createBuffers(OMX_HANDLETYPE handle, int portNumber, OMX_U32 *pNumBuffers, 
		   OMX_U32 *pBufferSize, OMX_BUFFERHEADERTYPE **ppBuffers) {
    OMX_PARAM_PORTDEFINITIONTYPE sPortDef;
    int n;
    int nBuffers;
    OMX_ERRORTYPE err;

    setHeader(&sPortDef, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
    sPortDef.nPortIndex = portNumber;
    err = OMX_GetParameter(handle, OMX_IndexParamPortDefinition, &sPortDef);
    if(err != OMX_ErrorNone){
	fprintf(stderr, "Error in getting OMX_PORT_DEFINITION_TYPE parameter\n");
	exit(1);
    }

    /* if no size pre-allocated, use the minimum */
    if (*pBufferSize == 0) {
	*pBufferSize = sPortDef.nBufferSize;
    } else {
	sPortDef.nBufferSize = *pBufferSize;
    }
    *pBufferSize = sPortDef.nBufferSize;

    nBuffers =  sPortDef.nBufferCountActual;
    nBuffers = (nBuffers > NUM_BUFFERS_USED ? NUM_BUFFERS_USED : nBuffers);
    //nBuffers = (nBuffers == 1 ? 2 : 1);
    *pNumBuffers = nBuffers;
    printf("Port %d has %d buffers of size is %d\n", portNumber, nBuffers, *pBufferSize);

    for (n = 0; n < nBuffers; n++) {
	err = OMX_AllocateBuffer(handle, ppBuffers+n, portNumber, NULL,
				 *pBufferSize);
	if (err != OMX_ErrorNone) {
	    fprintf(stderr, "Error on AllocateBuffer %d is %X %s\n", n, err, error2str(err));
	    exit(1);
	} else {
	  fprintf(stderr, "Buffer %d allocated ok\n", n);
	}
    }
}



static OMX_ERRORTYPE find_component(char *role, OMX_HANDLETYPE *handle, struct OMX_CALLBACKTYPE *callbacks) {
  OMX_ERRORTYPE err;
    int i;
    unsigned char name[OMX_MAX_STRINGNAME_SIZE];

    err = OMX_ErrorNone;
    for (i = 0; OMX_ErrorNoMore != err; i++) {
	err = OMX_ComponentNameEnum(name, OMX_MAX_STRINGNAME_SIZE, i);
	if (OMX_ErrorNone == err) {
	  // printf("Component is %s\n", name);
	    // listroles(name);
	}
	unsigned int j;
        for (j = 0; role_mappings[j].role; j++ ) {
            if ( !strcmp( role, role_mappings[j].role ) &&
                 !strcmp( name, role_mappings[j].name ) ) {
	      printf("Found match\n");

	      err = OMX_GetHandle(handle, name, NULL /*app private data */, 
				  callbacks);
	      if (err != OMX_ErrorNone) {
		fprintf(stderr, "OMX_GetHandle failed\n");
		return err;
	      }

	      /* after 1.2?
	      err = OMX_GetComponentVersion(handle, name, &compVersion, &specVersion, &uid);
	      if(err != OMX_ErrorNone) {
		fprintf(stderr, "OMX_GetComponentVersion failed\n");
		return err;
	      }
	      */

	      return OMX_ErrorNone;
            }
        }
    }
    printf("No more components\n");
} 

static int inited_video = 0;


void setImageEncoding(OMX_HANDLETYPE handle, int portNumber, OMX_IMAGE_CODINGTYPE encoding) {
    OMX_IMAGE_PARAM_PORTFORMATTYPE sImagePortFormat;
    OMX_ERRORTYPE err;


    setHeader(&sImagePortFormat, sizeof(OMX_IMAGE_PARAM_PORTFORMATTYPE));
    sImagePortFormat.nIndex = 0;
    sImagePortFormat.nPortIndex = portNumber;

    err = OMX_GetParameter(handle, OMX_IndexParamImagePortFormat, &sImagePortFormat);
    if (err == OMX_ErrorNoMore) {
	printf("Can't get format\n");
	return;
    }

    // no such field???
    //sImagePortFormat.eEncoding = encoding;
    sImagePortFormat.eCompressionFormat = encoding;
    err = OMX_SetParameter(handle, OMX_IndexParamImagePortFormat, &sImagePortFormat);
    if (err == OMX_ErrorNoMore) {
	printf("Can't set format\n");
	return;
    }
    printf("Set format on port %d\n", portNumber);
}


static void setup_image_decoder() {
  OMX_ERRORTYPE err;
  OMX_PORT_PARAM_TYPE param;

  int startDecodePortNumber;
  int nDecodePorts;

    find_component("image_decoder.jpeg", &decodeImageHandle, &decodeCallbacks);
    printState(decodeImageHandle);

    /* Get decode component info */
    setHeader(&param, sizeof(OMX_PORT_PARAM_TYPE));
    err = OMX_GetParameter(decodeImageHandle, OMX_IndexParamImageInit, &param);
    if(err != OMX_ErrorNone){
	fprintf(stderr, "Error in getting OMX_PORT_PARAM_TYPE parameter\n");
	exit(1);
    }
    startDecodePortNumber = ((OMX_PORT_PARAM_TYPE)param).nStartPortNumber;
    nDecodePorts = ((OMX_PORT_PARAM_TYPE)param).nPorts;
    fprintf(stderr, "N decode ports %d, strating %d\n", nDecodePorts, startDecodePortNumber);
    /*
    if (nDecodePorts != 2) {
	fprintf(stderr, "Decode device has wrong number of ports: %d\n", nDecodePorts);
	exit(1);
    }

    */
    setImageEncoding(decodeImageHandle, startDecodePortNumber, OMX_IMAGE_CodingJPEG);
    fprintf(stderr, "Encoding1 set\n");
    setImageEncoding(decodeImageHandle, startDecodePortNumber+1, OMX_IMAGE_CodingAutoDetect);
    fprintf(stderr, "Encoding2 set\n");

    /* call to put decoder state into idle before allocating buffers */
    err = OMX_SendCommand(decodeImageHandle, OMX_CommandStateSet, OMX_StateIdle, NULL);
    if (err != OMX_ErrorNone) {
	fprintf(stderr, "Error on setting state to idle\n");
	exit(1);
    }
 
    /* ensure decoder ports are enabled */
    err = OMX_SendCommand(decodeImageHandle, OMX_CommandPortEnable, startDecodePortNumber, NULL);
    if (err != OMX_ErrorNone) {
	fprintf(stderr, "Error on setting port to enabled\n");
	exit(1);
    }
    err = OMX_SendCommand(decodeImageHandle, OMX_CommandPortEnable, startDecodePortNumber+1, NULL);
    if (err != OMX_ErrorNone) {
	fprintf(stderr, "Error on setting port to enabled\n");
	exit(1);
    }

    /* use default buffer sizes */
    inDecodeBufferSize = outDecodeBufferSize = 0;
    createBuffers(decodeImageHandle, startDecodePortNumber, 
		  &numInDecodeBuffers, &inDecodeBufferSize, inDecodeBuffers);
    createBuffers(decodeImageHandle, startDecodePortNumber+1, 
		  &numOutDecodeBuffers, &outDecodeBufferSize, outDecodeBuffers);


    /* Make sure we've reached Idle state */
    waitFor(OMX_StateIdle);
    printf("Image decodeer reached Idle state\n");

    /* Now try to switch to Executing state */
    err = OMX_SendCommand(decodeImageHandle, OMX_CommandStateSet, OMX_StateExecuting, NULL);
    if(err != OMX_ErrorNone){
	fprintf(stderr, "Error changing to Executing state\n");
	exit(1);
    }
    /* end decode image setting */
}

/*ARGSUSED*/
int main(int argc, char **argv)
{
  OMX_ERRORTYPE err;

  int startRenderPortNumber;
  int nRenderPorts;
  OMX_PORT_PARAM_TYPE param;

# ifdef RASPBERRY_PI
    bcm_host_init();
# endif

    err = OMX_Init();
    if (err != OMX_ErrorNone) {
	fprintf(stderr, "OMX_Init() failed\n");
	exit(1);
    }

    //setup_video_decoder();
    //setup_renderer();
    //setup_image_reader(image_file);
    setup_image_decoder();


    inFd = open(image_file_name, O_RDONLY);
    if (inFd <=0) {
      perror("Can't open\n");
      exit(1);
    }

    /* no buffers emptied yet */
    pEmptyBuffer = NULL;

    /* load  the decoder input buffers */
    int n;
    for (n = 0; n < numInDecodeBuffers; n++) {
	int data_read = read(inFd, inDecodeBuffers[n]->pBuffer, inDecodeBufferSize);
	inDecodeBuffers[n]->nFilledLen = data_read;
	printf("Read %d into buffer %p\n", data_read, inDecodeBuffers[n]);
	if (data_read != inDecodeBufferSize) {
	    printf("In the %s no more input data available\n", __func__);
	    // inDecodeBuffers[n]->nFilledLen = 0;
	    inDecodeBuffers[n]->nFlags = OMX_BUFFERFLAG_EOS;
	    bEOS=OMX_TRUE;
	    break;
	}
    }
    
    /* fill the decoder output buffers */
    for (n = 0; n < numOutDecodeBuffers; n++) {
	outDecodeBuffers[n]->nFilledLen = 0;
	err = OMX_FillThisBuffer(decodeImageHandle, outDecodeBuffers[n]);
	if (err != OMX_ErrorNone) {
	    fprintf(stderr, "Error on filling buffer\n");
	    exit(1);
	}
    }

    /* empty the decoder input bufers */
    for (n = 0; n < numInDecodeBuffers; n++) {
	err = OMX_EmptyThisBuffer(decodeImageHandle, inDecodeBuffers[n]);
	if (err != OMX_ErrorNone) {
	    fprintf(stderr, "Error on emptying buffer\n");
	    exit(1);
	}
    }

    pEmptyBuffer = inDecodeBuffers[0];
    emptyState = 1;

    waitFor(OMX_StateLoaded);
    printf("Buffers emptied\n");


    exit(0);
    return 0;
}

