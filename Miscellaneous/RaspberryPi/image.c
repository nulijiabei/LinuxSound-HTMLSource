
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <gtk/gtk.h>

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>

#include "mytimidity.h"

#include <pthread.h>

#include <OMX_Core.h>
#include <OMX_Component.h>
#include <OMX_Types.h>
#include <OMX_Audio.h>
#include <OMX_Video.h>

#ifdef RASPBERRY_PI
#include <bcm_host.h>
#endif

#define WIDTH 720
#define HEIGHT 480

#define NUM_LINES 4


/******************************** OMX stuff ************************/

#define NUM_BUFFERS_USED 32

FILE video_file;
char *video_file_name = "Tai Chi - Yang Style Complete Set.mp4";
//char *image_file_name = "cimg0135.jpg";
//char *image_file_name = "hype.jpg";
char *image_file_name = "box.jpg";

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
OMX_BUFFERHEADERTYPE *outDecodeBuffers[NUM_BUFFERS_USED] = {NULL};
//OMX_BUFFERHEADERTYPE **outDecodeBuffers = NULL;
OMX_BUFFERHEADERTYPE *inRenderBuffers[NUM_BUFFERS_USED];
OMX_BUFFERHEADERTYPE *outReadBuffers[NUM_BUFFERS_USED];

pthread_mutex_t mutex;
OMX_STATETYPE currentState = OMX_StateLoaded;
pthread_cond_t stateCond;

void waitFor(OMX_STATETYPE state) {
  printf("Waiting for %p\n", state);
    pthread_mutex_lock(&mutex);
    while (currentState != state)
	pthread_cond_wait(&stateCond, &mutex);
    printf("Wait successfully completed\n");
    pthread_mutex_unlock(&mutex);
}

void wakeUp(OMX_STATETYPE newState) {
  printf("Waking up %p\n", newState);
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

pthread_mutex_t component_changed_mutex;
int component_changed_State = 1;
OMX_BUFFERHEADERTYPE* pComponent_Changed_Buffer;
pthread_cond_t component_changed_StateCond;

void waitForComponentChanged() {
    pthread_mutex_lock(&component_changed_mutex);
    while (component_changed_State == 1)
	pthread_cond_wait(&component_changed_StateCond, &component_changed_mutex);
    component_changed_State = 1;
    pthread_mutex_unlock(&component_changed_mutex);
}

void wakeUpComponentChanged(int nBuffer) {
    pthread_mutex_lock(&component_changed_mutex);
    component_changed_State = 0;
    //pComponent_Changed_Buffer = pBuffer;
    pthread_cond_signal(&component_changed_StateCond);
    pthread_mutex_unlock(&component_changed_mutex);
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
    n = pthread_mutex_init(&component_changed_mutex, NULL);
    if ( n != 0) {
	fprintf(stderr, "Can't init component changed mutex\n");
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

static void setHeader(OMX_PTR header, OMX_U32 size) {
    memset(header, 0, size);
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

OMX_HANDLETYPE decodeImageHandle;

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
	    printf("Component State changed in ", 0);
	    switch ((int)Data2) {
	    case OMX_StateInvalid:
		printf("OMX_StateInvalid\n", 0);
		break;
	    case OMX_StateLoaded:
		printf("OMX_StateLoaded\n", 0);
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
	    printf("OMX State Port enabled %d\n", (int) Data2);
	     wakeUp((int) Data1);
	} else if (Data1 == OMX_CommandPortDisable){
	    printf("OMX State Port disabled %d\n", (int) Data2); 
	     wakeUp((int) Data1);
	}
    } else if(eEvent == OMX_EventBufferFlag) {
	if((int)Data2 == OMX_BUFFERFLAG_EOS) {
     
	}
    } else if(eEvent == OMX_EventError) {
      if (Data1 == OMX_ErrorSameState) {
	printf("Already in requested state\n");
      } else {
	printf("Event is Error %X\n", Data1);
      }
    } else  if(eEvent == OMX_EventMark) {
	printf("Event is Buffer Mark\n");
    } else  if(eEvent == OMX_EventPortSettingsChanged) {
	/* See 1.1 spec, section 8.9.3.1 Playback Use Case */
	OMX_PARAM_PORTDEFINITIONTYPE sPortDef;
	wakeUpComponentChanged((int) Data1);

	printf("Event is PortSettingsChanged\n");
  
	setHeader(&sPortDef, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
	sPortDef.nPortIndex = Data1;

	err = OMX_GetConfig(hComponent, OMX_IndexParamPortDefinition, &sPortDef);
	if(err != OMX_ErrorNone){
	    fprintf(stderr, "Error in getting OMX_PORT_DEFINITION_TYPE parameter\n", 0);
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
    // for now
    return OMX_ErrorNone;

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
      fprintf(stderr, "Error on emptying decode buffer %X\n", err);
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
	fprintf(stderr, "Error in getting OMX_PORT_DEFINITION_TYPE parameter\n", 0);
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
	  fprintf(stderr, "Error on AllocateBuffer %d is %X\n", n, err);
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
		fprintf(stderr, "OMX_GetHandle failed\n", 0);
		return err;
	      }

	      /* after 1.2?
	      err = OMX_GetComponentVersion(handle, name, &compVersion, &specVersion, &uid);
	      if(err != OMX_ErrorNone) {
		fprintf(stderr, "OMX_GetComponentVersion failed\n", 0);
		return err;
	      }
	      */

	      return OMX_ErrorNone;
            }
        }
    }
    printf("No more components\n");
    return  OMX_ErrorNone;
} 

int startDecodePortNumber;
int nDecodePorts;

static void get_image_decoder_port_info() {
  OMX_ERRORTYPE err;
  OMX_PORT_PARAM_TYPE portInfo;


    /* Get decode component info */
    setHeader(&portInfo, sizeof(OMX_PORT_PARAM_TYPE));
    err = OMX_GetParameter(decodeImageHandle, OMX_IndexParamImageInit, &portInfo);
    if(err != OMX_ErrorNone){
	fprintf(stderr, "Error in getting OMX_PORT_PARAM_TYPE parameter\n", 0);
	exit(1);
    }
    startDecodePortNumber = portInfo.nStartPortNumber;
    nDecodePorts = portInfo.nPorts;
    fprintf(stderr, "N decode ports %d, strating %d\n", nDecodePorts, startDecodePortNumber);
    
    if (nDecodePorts != 2) {
	fprintf(stderr, "Decode device has wrong number of ports: %d\n", nDecodePorts);
	exit(1);
    }
}

static void set_image_decoder_input_format() {
   // set input image format
    printf("Setting image decoder format\n");
    OMX_IMAGE_PARAM_PORTFORMATTYPE imagePortFormat;
    setHeader(&imagePortFormat,  sizeof(OMX_IMAGE_PARAM_PORTFORMATTYPE));
    imagePortFormat.nPortIndex = startDecodePortNumber;
    imagePortFormat.eCompressionFormat = OMX_IMAGE_CodingJPEG;
    OMX_SetParameter(decodeImageHandle,
                     OMX_IndexParamImagePortFormat, &imagePortFormat);
    printf("Set image decoder input format\n");

}

static void configure_image_decoder_ports() {
  OMX_ERRORTYPE err;

#if 1
    /* ensure decoder ports are enabled */
    err = OMX_SendCommand(decodeImageHandle, OMX_CommandPortEnable, startDecodePortNumber, NULL);
    if (err != OMX_ErrorNone) {
	fprintf(stderr, "Error on setting port to enabled\n");
	exit(1);
    }
    err = OMX_SendCommand(decodeImageHandle, OMX_CommandPortEnable, startDecodePortNumber+1, NULL);
    //err = OMX_SendCommand(decodeImageHandle, OMX_CommandPortDisable, startDecodePortNumber+1, NULL);
    if (err != OMX_ErrorNone) {
	fprintf(stderr, "Error on setting port to enabled\n");
	exit(1);
    }
#endif


    /* use default buffer sizes */
    inDecodeBufferSize = outDecodeBufferSize = 0;
    createBuffers(decodeImageHandle, startDecodePortNumber, 
		  &numInDecodeBuffers, &inDecodeBufferSize, inDecodeBuffers);
    /*
    createBuffers(decodeImageHandle, startDecodePortNumber+1, 
		  &numOutDecodeBuffers, &outDecodeBufferSize, outDecodeBuffers);
    */
    outDecodeBuffers[0] = NULL;
    printf("Configured decoder ports\n");
}

static void image_decode_port_setting_changed() {
  OMX_ERRORTYPE err;
  OMX_PARAM_PORTDEFINITIONTYPE sPortDef;
  
  printf("PortSettingsChanged - configure decoder out\n");

  err = OMX_SendCommand(decodeImageHandle, OMX_CommandStateSet, OMX_StateIdle, NULL);
  if (err != OMX_ErrorNone) {
    fprintf(stderr, "Error on setting state to idle\n");
    exit(1);
  } 

  
  setHeader(&sPortDef, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
  sPortDef.nPortIndex = startDecodePortNumber+1;

  err = OMX_GetConfig(decodeImageHandle, OMX_IndexParamPortDefinition, &sPortDef);
  if(err != OMX_ErrorNone){
    fprintf(stderr, "Error in getting OMX_PORT_DEFINITION_TYPE parameter\n", 0);
    exit(1);
  }

  outDecodeBufferSize = sPortDef.nBufferSize;
  printf("New buffer size %d\n", outDecodeBufferSize);
  // just one buffer
  //outDecodeBuffers =  malloc(sizeof(OMX_BUFFERHEADERTYPE *));
  createBuffers(decodeImageHandle, startDecodePortNumber+1, 
		&numOutDecodeBuffers, &outDecodeBufferSize, outDecodeBuffers);
  /*
	printf("Event is Port Settings Changed on port %d in component %p\n",
	       Data1, hComponent);
	printf("Port has %d buffers of size is %d\n",  sPortDef.nBufferCountMin, sPortDef.nBufferSize);
  */   

    waitFor(OMX_StateIdle);
    printf("Image decoder reached Idle state\n");

    /* Now try to switch to Executing state */
    err = OMX_SendCommand(decodeImageHandle, OMX_CommandStateSet, OMX_StateExecuting, NULL);
    if(err != OMX_ErrorNone){
	fprintf(stderr, "Error changing to Executing state from port setting changed\n");
	exit(1);
    }
}

static void setup_image_decoder() {
  OMX_ERRORTYPE err;

  printf("Setting up image decoder\n");

    find_component("image_decoder.jpeg", &decodeImageHandle, &decodeCallbacks);
    printState(decodeImageHandle);

    get_image_decoder_port_info();


#if 1
    /* ensure decoder ports are enabled */
    //err = OMX_SendCommand(decodeImageHandle, OMX_CommandPortEnable, startDecodePortNumber, NULL);   
    err = OMX_SendCommand(decodeImageHandle, OMX_CommandPortDisable, startDecodePortNumber, NULL);
    if (err != OMX_ErrorNone) {
	fprintf(stderr, "Error on setting port 320 to disabled\n");
	exit(1);
    }
    //err = OMX_SendCommand(decodeImageHandle, OMX_CommandPortEnable, startDecodePortNumber+1, NULL);
    err = OMX_SendCommand(decodeImageHandle, OMX_CommandPortDisable, startDecodePortNumber+1, NULL);
    if (err != OMX_ErrorNone) {
	fprintf(stderr, "Error on setting port 321 to disabled\n");
	exit(1);
    }
#endif

    /* call to put decoder state into idle before allocating buffers */
    err = OMX_SendCommand(decodeImageHandle, OMX_CommandStateSet, OMX_StateIdle, NULL);
    if (err != OMX_ErrorNone) {
	fprintf(stderr, "Error on setting state to idle\n");
	exit(1);
    } 

    /* Make sure we've reached Idle state */
    waitFor(OMX_StateIdle);
    printf("Image decoder reached Idle state\n");

    configure_image_decoder_ports();
    set_image_decoder_input_format();


    /* Now try to switch to Executing state */
    err = OMX_SendCommand(decodeImageHandle, OMX_CommandStateSet, OMX_StateExecuting, NULL);
    if(err != OMX_ErrorNone){
	fprintf(stderr, "Error changing to Executing state %X in setting up image decoder\n", err);
	exit(1);
    }
    /* end decode image setting */
}


/*ARGSUSED*/
//static int ctl_open(int using_stdin, int using_stdout)
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
	fprintf(stderr, "OMX_Init() failed\n", 0);
	exit(1);
    }


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
	inDecodeBuffers[n]->nOffset = 0;

	//inDecodeBuffers[n]->nFlags = OMX_BUFFERFLAG_EOS;

	printf("Read %d into buffer %p\n", data_read, inDecodeBuffers[n]);

	//break;


	if (data_read < inDecodeBufferSize) {
	    printf("In the %s no more input data available\n", __func__);
	    // inDecodeBuffers[n]->nFilledLen = 0;
	    inDecodeBuffers[n]->nFlags = OMX_BUFFERFLAG_EOS;
	    //bEOS=OMX_TRUE;
	}

	err = OMX_EmptyThisBuffer(decodeImageHandle, inDecodeBuffers[n]);
	if (err != OMX_ErrorNone) {
	    fprintf(stderr, "Error on emptying buffer\n");
	    exit(1);
	}

	if (outDecodeBuffers[0] == NULL) {
	  printf("Waiting for port setting change\n");
	  waitForComponentChanged(OMX_EventPortSettingsChanged);
	  image_decode_port_setting_changed();
	}
    }

#if 0    
    /* fill the decoder output buffers */
    for (n = 0; n < numOutDecodeBuffers; n++) {
	outDecodeBuffers[n]->nFilledLen = 0;
	err = OMX_FillThisBuffer(decodeImageHandle, outDecodeBuffers[n]);
	if (err != OMX_ErrorNone) {
	    fprintf(stderr, "Error on filling buffer\n");
	    exit(1);
	}
    }
#endif
#if 0
    /* empty the decoder input bufers */
    for (n = 0; n < numInDecodeBuffers; n++) {
	err = OMX_EmptyThisBuffer(decodeImageHandle, inDecodeBuffers[n]);
	if (err != OMX_ErrorNone) {
	    fprintf(stderr, "Error on emptying buffer\n");
	    exit(1);
	}

	if (outDecodeBuffers == NULL) {
	  printf("Waiting for port setting change\n");
	  waitForComponentChanged(OMX_EventPortSettingsChanged);
	  image_decode_port_setting_changed();
	}
	//break;
    }
#endif
    pEmptyBuffer = inDecodeBuffers[0];
    emptyState = 1;

    waitFor(OMX_StateLoaded);
    printf("Buffers emptied\n");

    return 0;
}

