#include <stdio.h>
#include <stdlib.h>
#include "OpenSLES.h"
#define MAX_NUMBER_INTERFACES 5
#define MAX_NUMBER_INPUT_DEVICES 3
#define POSITION_UPDATE_PERIOD 1000 /* 1 sec */
/* Checks for error. If any errors exit the application! */
void CheckErr( SLresult res )
{
    if ( res != SL_RESULT_SUCCESS )
	{
	    // Debug printing to be placed here
	    exit(1);
	}
}
void RecordEventCallback(SLRecordItf caller,
			 void *pContext, 
			 SLuint32 recordevent)

{
    /* Callback code goes here */
}
/*
 * Test recording of audio from a microphone into a specified file
 */
void TestAudioRecording(SLObjectItf sl)
{
    SLObjectItf
	recorder;
    SLRecordItf
	recordItf;
    SLEngineItf
	EngineItf;
    SLAudioIODeviceCapabilitiesItf AudioIODeviceCapabilitiesItf;
    SLAudioInputDescriptor
	AudioInputDescriptor;
    SLresult
	res;
    SLDataSource audioSource;
    SLDataLocator_IODevice locator_mic;
    SLDeviceVolumeItf devicevolumeItf;
    SLDataSink audioSink;
    SLDataLocator_URI uri;
    SLDataFormat_MIME mime;
    int i;
    SLboolean required[MAX_NUMBER_INTERFACES];
    SLInterfaceID iidArray[MAX_NUMBER_INTERFACES];
    SLuint32 InputDeviceIDs[MAX_NUMBER_INPUT_DEVICES];
    SLint32
	numInputs = 0;
    SLboolean mic_available = SL_BOOLEAN_FALSE;
    SLuint32 mic_deviceID = 0;
    /* Get the SL Engine Interface which is implicit */
    res = (*sl)->GetInterface(sl, SL_IID_ENGINE, (void*)&EngineItf);
    CheckErr(res);
    /* Get the Audio IO DEVICE CAPABILITIES interface, which is also
       implicit */
    res = (*sl)->GetInterface(sl, SL_IID_AUDIOIODEVICECAPABILITIES,
			      (void*)&AudioIODeviceCapabilitiesItf); CheckErr(res);
    numInputs = MAX_NUMBER_INPUT_DEVICES;
    res = (*AudioIODeviceCapabilitiesItf)->GetAvailableAudioInputs(
								   AudioIODeviceCapabilitiesItf, &numInputs, InputDeviceIDs); CheckErr(res);
    /* Search for either earpiece microphone or headset microphone input
       device - with a preference for the latter */
    for (i=0;i<numInputs; i++)
	{
	    res = (*AudioIODeviceCapabilitiesItf)-
		>QueryAudioInputCapabilities(AudioIODeviceCapabilitiesItf,
					     InputDeviceIDs[i], &AudioInputDescriptor); CheckErr(res);
	    if((AudioInputDescriptor.deviceConnection ==
		SL_DEVCONNECTION_ATTACHED_WIRED)&&
	       (AudioInputDescriptor.deviceScope == SL_DEVSCOPE_USER)&&
	       (AudioInputDescriptor.deviceLocation ==
		SL_DEVLOCATION_HEADSET))
		{
		    mic_deviceID = InputDeviceIDs[i];
		    mic_available = SL_BOOLEAN_TRUE;
		    break;
		}
	    else if((AudioInputDescriptor.deviceConnection ==
		     SL_DEVCONNECTION_INTEGRATED)&&
		    (AudioInputDescriptor.deviceScope ==
		     SL_DEVSCOPE_USER)&&
		    (AudioInputDescriptor.deviceLocation ==
		     SL_DEVLOCATION_HANDSET))
		{
		    mic_deviceID = InputDeviceIDs[i];
		    mic_available = SL_BOOLEAN_TRUE;
		    break;
		}
	}
    /* If neither of the preferred input audio devices is available, no
       point in continuing */
    if (!mic_available) {
	/* Appropriate error message here */
	exit(1);
    }
    /* Initialize arrays required[] and iidArray[] */
    for (i=0;i<MAX_NUMBER_INTERFACES;i++)
	{
	    required[i] = SL_BOOLEAN_FALSE;
	    iidArray[i] = SL_IID_NULL;
	}
    /* Get the optional DEVICE VOLUME interface from the engine */
    res = (*sl)->GetInterface(sl, SL_IID_DEVICEVOLUME,
			      (void*)&devicevolumeItf); CheckErr(res);
    /* Set recording volume of the microphone to -3 dB */
    res = (*devicevolumeItf)->SetVolume(devicevolumeItf, mic_deviceID, -
					300); CheckErr(res);
    /* Setup the data source structure */
    locator_mic.locatorType	= SL_DATALOCATOR_IODEVICE;
    locator_mic.deviceType	= SL_IODEVICE_AUDIOINPUT;
    locator_mic.deviceID	= mic_deviceID;
    locator_mic.device	        = NULL;
    audioSource.pLocator 	= (void *)&locator_mic;
    audioSource.pFormat         = NULL;
    /* Setup the data sink structure */
    uri.locatorType	= SL_DATALOCATOR_URI;
    uri.URI	= (SLchar *) "file:///recordsample.wav";
    mime.formatType	= SL_DATAFORMAT_MIME;
    mime.mimeType	= (SLchar *) "audio/x-wav";
    mime.containerType	= SL_CONTAINERTYPE_WAV;
    audioSink.pLocator	= (void *)&uri;
    audioSink.pFormat	= (void *)&mime;
    /* Create audio recorder */
    res = (*EngineItf)->CreateAudioRecorder(EngineItf, &recorder,
					    &audioSource, &audioSink, 0, iidArray, required); CheckErr(res);
    /* Realizing the recorder in synchronous mode. */
    res = (*recorder)->Realize(recorder, SL_BOOLEAN_FALSE); CheckErr(res);
    /* Get the RECORD interface - it is an implicit interface */
    res = (*recorder)->GetInterface(recorder, SL_IID_RECORD,
				    (void*)&recordItf); CheckErr(res);
    /* Setup to receive position event callbacks */
    res = (*recordItf)->RegisterCallback(recordItf, RecordEventCallback,
					 NULL);
    CheckErr(res);
    /* Set notifications to occur after every second - may be useful in
       updating a recording progress bar */
    res = (*recordItf)->SetPositionUpdatePeriod( recordItf,
						 POSITION_UPDATE_PERIOD); CheckErr(res);
    res = (*recordItf)->SetCallbackEventsMask( recordItf,
					       SL_RECORDEVENT_HEADATNEWPOS); CheckErr(res);
    /* Set the duration of the recording - 30 seconds (30,000
       milliseconds) */
    res = (*recordItf)->SetDurationLimit(recordItf, 30000); CheckErr(res);
    /* Record the audio */
    res = (*recordItf)->SetRecordState(recordItf,SL_RECORDSTATE_RECORDING);
    /* Destroy the recorder object */
    (*recorder)->Destroy(recorder);
}
int sl_main( void )
{
    SLresult
	res;
    SLObjectItf sl;
    /* Create OpenSL ES engine in thread-safe mode */
    SLEngineOption EngineOption[] = {(SLuint32)
				     SL_ENGINEOPTION_THREADSAFE, (SLuint32) SL_BOOLEAN_TRUE};
    res = slCreateEngine( &sl, 1, EngineOption, 0, NULL, NULL);
    CheckErr(res);
    /* Realizing the SL Engine in synchronous mode. */
    res = (*sl)->Realize(sl, SL_BOOLEAN_FALSE); CheckErr(res);
    TestAudioRecording(sl);
    /* Shutdown OpenSL ES */
    (*sl)->Destroy(sl);
    exit(0);
}
