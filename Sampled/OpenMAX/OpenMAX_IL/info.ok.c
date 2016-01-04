/**
  Based on code
  Copyright (C) 2007-2009 STMicroelectronics
  Copyright (C) 2007-2009 Nokia Corporation and/or its subsidiary(-ies).
  under the LGPL
*/

#undef RASPBERRY_PI

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/stat.h>

#include <OMX_Core.h>
#include <OMX_Component.h>
#include <OMX_Types.h>
#include <OMX_Audio.h>

#ifdef RASPBERRY_PI
#include <bcm_host.h>
#endif

OMX_ERRORTYPE err;
OMX_HANDLETYPE handle;
OMX_VERSIONTYPE specVersion, compVersion;

OMX_CALLBACKTYPE callbacks;

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

void getSupportedFormats(int portNumber) {
    OMX_AUDIO_PARAM_PORTFORMATTYPE sAudioPortFormat;

    setHeader(&sAudioPortFormat, sizeof(OMX_AUDIO_PARAM_PORTFORMATTYPE));
    sAudioPortFormat.nIndex = 0;
    sAudioPortFormat.nPortIndex = portNumber;
    //sAudioPortFormat.eEncoding = OMX_AUDIO_CodingPCM;
    
    printf("Supported formats are:\n");
    for(;;) {
	/* try to get the possible audio encoding */    
        err = OMX_GetParameter(handle, OMX_IndexParamAudioPortFormat, &sAudioPortFormat);
        if (err == OMX_ErrorNoMore) {
	    printf("No more formats supported\n");
	    break;
        }
        if (err == OMX_AUDIO_CodingUnused) {
	    printf("No more formats: coding unused\n");
	    break;
        }
        printf("Port supports format %d at index %d returns %d\n", 
               sAudioPortFormat.eEncoding, sAudioPortFormat.nIndex, err);
        sAudioPortFormat.nIndex++;
      }
}


void getPortInfo(int portNumber) {
    /* Get and check input port information */
    OMX_PARAM_PORTDEFINITIONTYPE sPortDef;

    setHeader(&sPortDef, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
    sPortDef.nPortIndex = portNumber;
    err = OMX_GetParameter(handle, OMX_IndexParamPortDefinition, &sPortDef);
    if(err != OMX_ErrorNone){
	fprintf(stderr, "Error in getting OMX_PORT_DEFINITION_TYPE parameter\n", 0);
	exit(1);
    }

    switch (sPortDef.eDomain) {
    case OMX_PortDomainAudio:
	printf("Port %d is an audio port\n", portNumber);
    default:
	printf("Port %d is a different type of port\n", portNumber);
    }

    if (sPortDef.eDir == OMX_DirInput) {
	fprintf(stderr, "Port %d is an input port\n", portNumber);
    } else {
	fprintf(stderr, "Port %d is an output port\n", portNumber);
    }


    getSupportedFormats(portNumber);
    setFormat(portNumber, OMX_AUDIO_CodingPCM);

    switch (sPortDef.format.audio.eEncoding) {
    case  OMX_AUDIO_CodingPCM: 
	fprintf(stderr, "Port encoding is PCM\n");
	break;
    case  OMX_AUDIO_CodingMP3: 
	fprintf(stderr, "Port encoding is MP3\n");
	break;
    case  OMX_AUDIO_CodingVORBIS: 
	fprintf(stderr, "Port encoding is Vorbis\n");
	break;
    default: 
	fprintf(stderr, "Port has unknown encoding\n");
    }
    getSupportedFormats(portNumber);

    
    if (sPortDef.bEnabled) {
	fprintf(stderr, "Port is enabled\n");
    } else {
	fprintf(stderr, "Port is not enabled\n");
    }
}

int main(int argc, char** argv) {

  OMX_PORT_PARAM_TYPE param;
  OMX_PARAM_PORTDEFINITIONTYPE sPortDef;
  OMX_AUDIO_PORTDEFINITIONTYPE sAudioPortDef;
  OMX_AUDIO_PARAM_PORTFORMATTYPE sAudioPortFormat;
  OMX_AUDIO_PARAM_PCMMODETYPE sPCMMode;

#ifdef RASPBERRY_PI
  char *componentName = "OMX.broadcom.audio_mixer";;
#else
  char *componentName = "OMX.st.volume.component";
#endif
  unsigned char name[128]; /* spec says 128 is max name length */
  OMX_UUIDTYPE uid;
  int startPortNumber;
  int nPorts;
  int n;

  /* ovveride component name by command line argument */
  if (argc == 2) {
      componentName = argv[1];
  }

# ifdef RASPBERRY_PI
  bcm_host_init();
# endif

  err = OMX_Init();
  if(err != OMX_ErrorNone) {
      fprintf(stderr, "OMX_Init() failed\n", 0);
    exit(1);
  }
  /** Ask the core for a handle to the volume control component
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
  printf("Component name: %s version %d.%d, Spec version %d.%d\n",
	 name, compVersion.s.nVersionMajor,
	 compVersion.s.nVersionMinor,
	 specVersion.s.nVersionMajor,
	 specVersion.s.nVersionMinor);

  /** Get port information */
  setHeader(&param, sizeof(OMX_PORT_PARAM_TYPE));
  //param.nVersion.nVersion = OMX_VERSION;
  err = OMX_GetParameter(handle, OMX_IndexParamAudioInit, &param);
  if(err != OMX_ErrorNone){
      fprintf(stderr, "Error in getting OMX_PORT_PARAM_TYPE parameter\n", 0);
    exit(1);
  }
  startPortNumber = ((OMX_PORT_PARAM_TYPE)param).nStartPortNumber;
  nPorts = ((OMX_PORT_PARAM_TYPE)param).nPorts;
  printf("Ports start on %d\n", startPortNumber);
  printf("There are %d open ports\n", nPorts);

  for (n = 0; n < nPorts; n++) {
      setHeader(&sPortDef, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
      sPortDef.nPortIndex = startPortNumber + n;
      err = OMX_GetParameter(handle, OMX_IndexParamPortDefinition, &sPortDef);
      if(err != OMX_ErrorNone){
	fprintf(stderr, "Error in getting OMX_PORT_DEFINITION_TYPE parameter\n", 0);
	exit(1);
      }
      if (sPortDef.eDomain == OMX_PortDomainAudio) {
	printf("Port %d is an audio port\n", startPortNumber + n);
      } else {
	printf("Port %d is other device port\n",  startPortNumber + n);
      }
      
      if (sPortDef.eDir == OMX_DirInput) {
	printf("Port %d is an input port\n", startPortNumber + n);
      } else {
	printf("Port %d is an output port\n",  startPortNumber + n);
      }

      printf("Port min buffers %d,  mimetype %s, encoding %d\n",
	     sPortDef.nBufferCountMin,
	     sPortDef.format.audio.cMIMEType,
	     sPortDef.format.audio.eEncoding);
      if (sPortDef.format.audio.eEncoding == OMX_AUDIO_CodingPCM) {
	printf("Encoding is PCM\n");
      } else {
	printf("Encoding is not PCM\n");
      }

      /* create minimum number of buffers for the port */
      sPortDef.nBufferCountActual = sPortDef.nBufferCountMin;
      err = OMX_SetParameter(handle, OMX_IndexParamPortDefinition, &sPortDef);
      if(err != OMX_ErrorNone){
	fprintf(stderr, "Error in setting OMX_PORT_PARAM_TYPE parameter\n", 0);
	exit(1);
      }
      
      setHeader(&sAudioPortFormat, sizeof(OMX_AUDIO_PARAM_PORTFORMATTYPE));
      sAudioPortFormat.nIndex = 0;
      sAudioPortFormat.nPortIndex = startPortNumber + n;
      sAudioPortFormat.eEncoding = OMX_AUDIO_CodingPCM;

      for(;;) {
	  /* try to get the possible audio encoding */	  
	err = OMX_GetParameter(handle, OMX_IndexParamAudioPortFormat, &sAudioPortFormat);
	if (err == OMX_ErrorNoMore) {
	  printf("No more formats supported\n");
	  break;
	}
	if (err == OMX_AUDIO_CodingUnused) {
	  printf("No more formats: coding unused\n");
	  break;
	}
	printf("Port supports format %d at index %d returns %d\n", 
	       sAudioPortFormat.eEncoding, sAudioPortFormat.nIndex, err);
	sAudioPortFormat.nIndex++;
      }

      /* try setting the encoding to PCM mode anyway */
      setHeader(&sPCMMode, sizeof(OMX_AUDIO_PARAM_PCMMODETYPE));
      sPCMMode.nPortIndex = startPortNumber + n;
      err = OMX_GetParameter(handle, OMX_IndexParamAudioPcm, &sPCMMode);
      if(err != OMX_ErrorNone){
	printf("PCM mode unsupported\n");
	exit(1);
      } else {
	printf("PCM mode supported\n");
	printf("PCM sampling rate %d\n", sPCMMode.nSamplingRate);
      }      
  }
  exit(0);
}
