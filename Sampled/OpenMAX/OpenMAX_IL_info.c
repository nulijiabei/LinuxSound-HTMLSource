/**
  test/components/audio_effects/omxvolcontroltest.c

  This simple test application provides a testing stream for the volume control component.
  It will be added in the more complex audio test application in the next release.

  Copyright (C) 2007-2009 STMicroelectronics
  Copyright (C) 2007-2009 Nokia Corporation and/or its subsidiary(-ies).

  This library is free software; you can redistribute it and/or modify it under
  the terms of the GNU Lesser General Public License as published by the Free
  Software Foundation; either version 2.1 of the License, or (at your option)
  any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
  details.

  You should have received a copy of the GNU Lesser General Public License
  along with this library; if not, write to the Free Software Foundation, Inc.,
  51 Franklin St, Fifth Floor, Boston, MA
  02110-1301  USA

*/

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

#define DEBUG(x, y, z) fprintf(stderr, y, z)
/* Size of the buffers requested to the component */
#define BUFFER_IN_SIZE 2*8192*2

/** Specification version*/
#define VERSIONMAJOR    1
#define VERSIONMINOR    1
#define VERSIONREVISION 0
#define VERSIONSTEP     0

int fd = 0;
unsigned int filesize;
OMX_ERRORTYPE err;
OMX_HANDLETYPE handle;

char *input_file, *output_file;
static OMX_BOOL bEOS=OMX_FALSE;
FILE *outfile;

OMX_CALLBACKTYPE callbacks;


/* Application's private data */
typedef struct appPrivateType{
    //pthread_cond_t condition;
    //pthread_mutex_t mutex;
  void* input_data;
  OMX_BUFFERHEADERTYPE* currentInputBuffer;
    //tsem_t* eventSem;
    //tsem_t* eofSem;
}appPrivateType;


/* Application private date: should go in the component field (segs...) */
appPrivateType* appPriv;


static void setHeader(OMX_PTR header, OMX_U32 size) {
  OMX_VERSIONTYPE* ver = (OMX_VERSIONTYPE*)(header + sizeof(OMX_U32));
  *((OMX_U32*)header) = size;

  ver->s.nVersionMajor = VERSIONMAJOR;
  ver->s.nVersionMinor = VERSIONMINOR;
  ver->s.nRevision = VERSIONREVISION;
  ver->s.nStep = VERSIONSTEP;
}

static void display_help() {
    fprintf(stderr, "Usage: OpenMAX_IL_volume input_file output_file");
}

int main(int argc, char** argv) {

  OMX_PORT_PARAM_TYPE param;
  OMX_BUFFERHEADERTYPE *inBuffer1, *inBuffer2, *outBuffer1, *outBuffer2;
  int data_read1;
  int data_read2;
  OMX_PARAM_PORTDEFINITIONTYPE sPortDef;
  OMX_AUDIO_PORTDEFINITIONTYPE sAudioPortDef;
  OMX_AUDIO_PARAM_PORTFORMATTYPE sAudioPortFormat;
  OMX_AUDIO_PARAM_PCMMODETYPE sPCMMode;
  OMX_AUDIO_PARAM_MP3TYPE sMP3Mode;
  OMX_AUDIO_CONFIG_VOLUMETYPE sVolume;
  int gain=100;
  int argn_dec;
  OMX_INDEXTYPE custom_index;

  err = OMX_Init();
  if(err != OMX_ErrorNone) {
      fprintf(stderr, "OMX_Init() failed\n", 0);
    exit(1);
  }
  /** Ask the core for a handle to the volume control component
    */
  err = OMX_GetHandle(&handle, "OMX.st.volume.component", NULL /*appPriv */, &callbacks);
  if(err != OMX_ErrorNone) {
      fprintf(stderr, "OMX_GetHandle failed\n", 0);
    exit(1);
  }

  /** Get port information */
  setHeader(&param, sizeof(OMX_PORT_PARAM_TYPE));
  err = OMX_GetParameter(handle, OMX_IndexParamAudioInit, &param);
  if(err != OMX_ErrorNone){
      fprintf(stderr, "Error in getting OMX_PORT_PARAM_TYPE parameter\n", 0);
    exit(1);
  }
 
  printf("Ports start on %d\n",
	 ((OMX_PORT_PARAM_TYPE)param).nStartPortNumber);
  printf("There are %d open ports\n",
	 ((OMX_PORT_PARAM_TYPE)param).nStartPortNumber);

  setHeader(&sPortDef, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
  sPortDef.nPortIndex = 0;
  err = OMX_GetParameter(handle, OMX_IndexParamPortDefinition, &sPortDef);
  if(err != OMX_ErrorNone){
      fprintf(stderr, "Error in getting OMX_PORT_PARAM_TYPE parameter\n", 0);
    exit(1);
  }
  if (sPortDef.eDomain == OMX_PortDomainAudio) {
      printf("Is an audio port\n");
  } else {
      printf("Is other device port\n");
  }

  if (sPortDef.eDir == OMX_DirInput) {
      printf("Port is an input port\n");
  } else {
      printf("Port is an output port\n");
  }

  /* returns raw/audio format 0 
     PORTDEFTYPE.format.audio gets to AUDIO_PORTTYPE */
  printf("Port min buffers %d,  mimetype %s, encoding %d\n",
	 sPortDef.nBufferCountMin,
	 sPortDef.format.audio.cMIMEType,
	 sPortDef.format.audio.eEncoding);

  sPortDef.nBufferCountActual = 2;
  err = OMX_SetParameter(handle, OMX_IndexParamPortDefinition, &sPortDef);
  if(err != OMX_ErrorNone){
      fprintf(stderr, "Error in getting OMX_PORT_PARAM_TYPE parameter\n", 0);
    exit(1);
  }
  sPortDef.nPortIndex = 1;
  err = OMX_GetParameter(handle, OMX_IndexParamPortDefinition, &sPortDef);
  if (sPortDef.eDomain == OMX_PortDomainAudio) {
      printf("Is an audio port\n");
  } else {
      printf("Is other device port\n");
  }

  if (sPortDef.eDir == OMX_DirInput) {
      printf("Port is an input port\n");
  } else {
      printf("Port is an output port\n");
  }

  /* returns raw/audio format 0 
     PORTDEFTYPE.format.audio gets to AUDIO_PORTTYPE */
  printf("Port min buffers %d,  mimetype %s, encoding %d\n",
	 sPortDef.nBufferCountMin,
	 sPortDef.format.audio.cMIMEType,
	 sPortDef.format.audio.eEncoding);

  sPortDef.nBufferCountActual = 2;
  err = OMX_SetParameter(handle, OMX_IndexParamPortDefinition, &sPortDef);
  if(err != OMX_ErrorNone){
      fprintf(stderr, "Error in getting OMX_PORT_PARAM_TYPE parameter\n", 0);
    exit(1);
  }

  setHeader(&sAudioPortFormat, sizeof(OMX_AUDIO_PARAM_PORTFORMATTYPE));
  sAudioPortFormat.nIndex = 0;
  sAudioPortFormat.nPortIndex = 0;
  //sAudioPortFormat.eEncoding = OMX_AUDIO_CodingAutoDetect;
  sAudioPortFormat.eEncoding = OMX_AUDIO_CodingPCM;
  //sAudioPortFormat.eEncoding = OMX_AUDIO_CodingAutoDetect;
  for(;;) {
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

  setHeader(&sPCMMode, sizeof(OMX_AUDIO_PARAM_PCMMODETYPE));
  sPCMMode.nPortIndex = 0;
  err = OMX_GetParameter(handle, OMX_IndexParamAudioPcm, &sPCMMode);
  if(err != OMX_ErrorNone){
      printf("PCM mode unsupported\n");
      exit(1);
  } else {
      printf("PCM mode supported\n");
      printf("PCM sampling rate %d\n", sPCMMode.nSamplingRate);
  }

  setHeader(&sMP3Mode, sizeof(OMX_AUDIO_PARAM_MP3TYPE));
  sPCMMode.nPortIndex = 0;
  err = OMX_GetParameter(handle, OMX_IndexParamAudioMp3, &sMP3Mode);
  if(err != OMX_ErrorNone){
      printf("MP3 mode unsupported\n");
      exit(1);
  } else {
      printf("MP3 mode supported\n");
      //printf("PCM sampling rate %d\n", sPCMMode.nSamplingRate);
  }

  return 0;
}
