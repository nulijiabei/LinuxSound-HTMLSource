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

/* Callbacks implementation */
OMX_ERRORTYPE volcEventHandler(
  OMX_HANDLETYPE hComponent,
  OMX_PTR pAppData,
  OMX_EVENTTYPE eEvent,
  OMX_U32 Data1,
  OMX_U32 Data2,
  OMX_PTR pEventData) {

  fprintf(stderr, "Hi there, I am in the %s callback\n", __func__);
  if(eEvent == OMX_EventCmdComplete) {
    if (Data1 == OMX_CommandStateSet) {
	fprintf(stderr, "Volume Component State changed in ", 0);
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
      
    } else  if (Data1 == OMX_CommandPortEnable){
     
    } else if (Data1 == OMX_CommandPortDisable){
     
    }
  } else if(eEvent == OMX_EventBufferFlag) {
    if((int)Data2 == OMX_BUFFERFLAG_EOS) {
     
    }
  } else {
    fprintf(stderr, "Param1 is %i\n", (int)Data1);
    fprintf(stderr, "Param2 is %i\n", (int)Data2);
  }

  return OMX_ErrorNone;
}

OMX_ERRORTYPE volcEmptyBufferDone(
  OMX_HANDLETYPE hComponent,
  OMX_PTR pAppData,
  OMX_BUFFERHEADERTYPE* pBuffer) {

  int data_read;
  static int iBufferDropped=0;

  fprintf(stderr, "Hi there, I am in the %s callback.\n", __func__);
  data_read = read(fd, pBuffer->pBuffer, BUFFER_IN_SIZE);
  pBuffer->nFilledLen = data_read;
  pBuffer->nOffset = 0;
  filesize -= data_read;
  if (data_read <= 0) {
    fprintf(stderr, "In the %s no more input data available\n", __func__);
    iBufferDropped++;
    if(iBufferDropped>=2) {
	//tsem_up(appPriv->eofSem);
      return OMX_ErrorNone;
    }
    pBuffer->nFilledLen=0;
    pBuffer->nFlags = OMX_BUFFERFLAG_EOS;
    bEOS=OMX_TRUE;
    err = OMX_EmptyThisBuffer(hComponent, pBuffer);
    return OMX_ErrorNone;
  }
  if(!bEOS) {
      fprintf(stderr, "Emptying again buffer %p %d bytes\n", pBuffer, data_read);
    err = OMX_EmptyThisBuffer(hComponent, pBuffer);
  }else {
    fprintf(stderr, "In %s Dropping Empty This buffer to Audio Dec\n", __func__);
  }

  return OMX_ErrorNone;
}

OMX_ERRORTYPE volcFillBufferDone(
  OMX_HANDLETYPE hComponent,
  OMX_PTR pAppData,
  OMX_BUFFERHEADERTYPE* pBuffer) {

  OMX_ERRORTYPE err;
  int i;

  /*
  fprintf(stderr, "Hi there, I am in the %s callback. Got buflen %i for buffer at 0x%p\n",
                          __func__, (int)pBuffer->nFilledLen, pBuffer);
  */
  /* Output data to standard output */
  if(pBuffer != NULL) {
    if (pBuffer->nFilledLen == 0) {
      fprintf(stderr, "Ouch! In %s: no data in the output buffer!\n", __func__);
      return OMX_ErrorNone;
    }

    if(pBuffer->nFilledLen > 0) {
        fwrite(pBuffer->pBuffer, 1, pBuffer->nFilledLen, outfile);
    }

    pBuffer->nFilledLen = 0;
  } else {
    fprintf(stderr, "Ouch! In %s: had NULL buffer to output...\n", __func__);
  }
  /* Reschedule the fill buffer request */
  if(!bEOS) {
    err = OMX_FillThisBuffer(hComponent, pBuffer);
  }
  return OMX_ErrorNone;
}



OMX_CALLBACKTYPE callbacks = { .EventHandler = volcEventHandler,
                               .EmptyBufferDone = volcEmptyBufferDone,
                               .FillBufferDone = volcFillBufferDone,
};


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
  OMX_AUDIO_CONFIG_VOLUMETYPE sVolume;
  int gain=100;
  int argn_dec;
  OMX_INDEXTYPE custom_index;

  /* Obtain file descriptor */
  if(argc < 3){
    display_help();
    exit(1);
  }
  output_file = argv[2];
  input_file = argv[1];

  fd = open(input_file, O_RDONLY);
  if(fd < 0){
    perror("Error opening input file\n");
    exit(1);
  }
  filesize = getFileSize(fd);

  outfile = fopen(output_file,"wb");
  if(outfile == NULL) {
      fprintf(stderr, "Error at opening the output file");
      exit(1);
  }


  /* Initialize application private data 
  appPriv = malloc(sizeof(appPrivateType));
  pthread_cond_init(&appPriv->condition, NULL);
  pthread_mutex_init(&appPriv->mutex, NULL);
  appPriv->eventSem = malloc(sizeof(tsem_t));
  tsem_init(appPriv->eventSem, 0);
  appPriv->eofSem = malloc(sizeof(tsem_t));
  tsem_init(appPriv->eofSem, 0);
  */

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

  gain = 50;
  if((gain >= 0) && (gain <100)) {
    err = OMX_GetConfig(handle, OMX_IndexConfigAudioVolume, &sVolume);
    if(err!=OMX_ErrorNone) {
      fprintf(stderr,"Error %08x In OMX_GetConfig 0 \n",err);
    }
    sVolume.sVolume.nValue = gain;
    fprintf(stderr, "Setting Gain %d \n", gain);
    err = OMX_SetConfig(handle, OMX_IndexConfigAudioVolume, &sVolume);
    if(err!=OMX_ErrorNone) {
      fprintf(stderr,"Error %08x In OMX_SetConfig 0 \n",err);
    }
  }

  /** Set the number of ports for the parameter structure */
  param.nPorts = 2;
  setHeader(&param, sizeof(OMX_PORT_PARAM_TYPE));
  err = OMX_GetParameter(handle, OMX_IndexParamAudioInit, &param);
  if(err != OMX_ErrorNone){
      fprintf(stderr, "Error in getting OMX_PORT_PARAM_TYPE parameter\n", 0);
    exit(1);
  }

  setHeader(&sPortDef, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
  sPortDef.nPortIndex = 0;
  err = OMX_GetParameter(handle, OMX_IndexParamPortDefinition, &sPortDef);

  sPortDef.nBufferCountActual = 2;
  err = OMX_SetParameter(handle, OMX_IndexParamPortDefinition, &sPortDef);
  if(err != OMX_ErrorNone){
      fprintf(stderr, "Error in getting OMX_PORT_PARAM_TYPE parameter\n", 0);
    exit(1);
  }
  sPortDef.nPortIndex = 1;
  err = OMX_GetParameter(handle, OMX_IndexParamPortDefinition, &sPortDef);

  sPortDef.nBufferCountActual = 2;
  err = OMX_SetParameter(handle, OMX_IndexParamPortDefinition, &sPortDef);
  if(err != OMX_ErrorNone){
      fprintf(stderr, "Error in getting OMX_PORT_PARAM_TYPE parameter\n", 0);
    exit(1);
  }

  err = OMX_SendCommand(handle, OMX_CommandStateSet, OMX_StateIdle, NULL);

  inBuffer1 = inBuffer2 = outBuffer1 = outBuffer2 = NULL;
  err = OMX_AllocateBuffer(handle, &inBuffer1, 0, NULL, BUFFER_IN_SIZE);
  if (err != OMX_ErrorNone) {
    fprintf(stderr, "Error on AllocateBuffer in 1%i\n", err);
    exit(1);
  }
  err = OMX_AllocateBuffer(handle, &inBuffer2, 0, NULL, BUFFER_IN_SIZE);
  if (err != OMX_ErrorNone) {
    fprintf(stderr, "Error on AllocateBuffer in 2 %i\n", err);
    exit(1);
  }
  err = OMX_AllocateBuffer(handle, &outBuffer1, 1, NULL, BUFFER_IN_SIZE);
  if (err != OMX_ErrorNone) {
    fprintf(stderr, "Error on AllocateBuffer out 1 %i\n", err);
    exit(1);
  }
  err = OMX_AllocateBuffer(handle, &outBuffer2, 1, NULL, BUFFER_IN_SIZE);
  if (err != OMX_ErrorNone) {
    fprintf(stderr, "Error on AllocateBuffer out 2 %i\n", err);
    exit(1);
  }

  err = OMX_SendCommand(handle, OMX_CommandStateSet, OMX_StateExecuting, NULL);

  /* Wait for commands to complete */
  //tsem_down(appPriv->eventSem);

  /*
  fprintf(stderr, "Had buffers at:\n0x%p\n0x%p\n0x%p\n0x%p\n",
                inBuffer1->pBuffer, inBuffer2->pBuffer, outBuffer1->pBuffer, outBuffer2->pBuffer);
  fprintf(stderr, "After switch to executing\n");
  */

  data_read1 = read(fd, inBuffer1->pBuffer, BUFFER_IN_SIZE);
  inBuffer1->nFilledLen = data_read1;
  filesize -= data_read1;

  data_read2 = read(fd, inBuffer2->pBuffer, BUFFER_IN_SIZE);
  inBuffer2->nFilledLen = data_read2;
  filesize -= data_read2;

  fprintf(stderr, "Empty first  buffer %p %d bytes\n", inBuffer1, data_read1);
  err = OMX_EmptyThisBuffer(handle, inBuffer1);
  fprintf(stderr, "Empty second buffer %p %d bytes\n", inBuffer2, data_read2);
  err = OMX_EmptyThisBuffer(handle, inBuffer2);

  /** Schedule a couple of buffers to be filled on the output port
    * The callback itself will re-schedule them.
    */
  err = OMX_FillThisBuffer(handle, outBuffer1);
  err = OMX_FillThisBuffer(handle, outBuffer2);

  sleep(1000);
  /*
  err = OMX_SendCommand(handle, OMX_CommandStateSet, OMX_StateLoaded, NULL);
  err = OMX_FreeBuffer(handle, 0, inBuffer1);
  err = OMX_FreeBuffer(handle, 0, inBuffer2);
  err = OMX_FreeBuffer(handle, 1, outBuffer1);
  err = OMX_FreeBuffer(handle, 1, outBuffer2);

  OMX_FreeHandle(handle);

  free(appPriv->eventSem);
  free(appPriv);

  if (flagOutputReceived) {
    if(fclose(outfile) != 0) {
      fprintf(stderr,"Error in closing output file\n");
      exit(1);
    }
    free(output_file);
  }

  close(fd);
  free(input_file);

  return 0;
  */
}
