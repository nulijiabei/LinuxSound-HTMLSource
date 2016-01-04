
#define PERIOD_SIZE 1024
#define BUF_SIZE (PERIOD_SIZE * 2)

#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>


int setparams(snd_pcm_t *handle, char *name) {
  snd_pcm_hw_params_t *hw_params;
  int err;


  if ((err = snd_pcm_hw_params_malloc (&hw_params)) < 0) {
    fprintf (stderr, "cannot allocate hardware parameter structure (%s)\n",
	     snd_strerror (err));
    exit (1);
  }
				 
  if ((err = snd_pcm_hw_params_any (handle, hw_params)) < 0) {
    fprintf (stderr, "cannot initialize hardware parameter structure (%s)\n",
	     snd_strerror (err));
    exit (1);
  }
	
  if ((err = snd_pcm_hw_params_set_access (handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
    fprintf (stderr, "cannot set access type (%s)\n",
	     snd_strerror (err));
    exit (1);
  }
	
  if ((err = snd_pcm_hw_params_set_format (handle, hw_params, SND_PCM_FORMAT_S16_LE)) < 0) {
    fprintf (stderr, "cannot set sample format (%s)\n",
	     snd_strerror (err));
    exit (1);
  }

  unsigned int rate = 48000;	
  if ((err = snd_pcm_hw_params_set_rate_near (handle, hw_params, &rate, 0)) < 0) {
    fprintf (stderr, "cannot set sample rate (%s)\n",
	     snd_strerror (err));
    exit (1);
  }
  printf("Rate for %s is %d\n", name, rate);
	
  if ((err = snd_pcm_hw_params_set_channels (handle, hw_params, 2)) < 0) {
    fprintf (stderr, "cannot set channel count (%s)\n",
	     snd_strerror (err));
    exit (1);
  }
	
  snd_pcm_uframes_t buffersize = BUF_SIZE;
  if ((err = snd_pcm_hw_params_set_buffer_size_near(handle, hw_params, &buffersize)) < 0) {
    printf("Unable to set buffer size %i: %s\n", BUF_SIZE, snd_strerror(err));
    exit (1);;
  }

  snd_pcm_uframes_t periodsize = PERIOD_SIZE;
  fprintf(stderr, "period size now %d\n", periodsize);
  if ((err = snd_pcm_hw_params_set_period_size_near(handle, hw_params, &periodsize, 0)) < 0) {
    printf("Unable to set period size %li: %s\n", periodsize, snd_strerror(err));
    exit (1);
  }

  if ((err = snd_pcm_hw_params (handle, hw_params)) < 0) {
    fprintf (stderr, "cannot set parameters (%s)\n",
	     snd_strerror (err));
    exit (1);
  }

  snd_pcm_uframes_t p_psize;
  snd_pcm_hw_params_get_period_size(hw_params, &p_psize, NULL);
  fprintf(stderr, "period size %d\n", p_psize);

  snd_pcm_hw_params_get_buffer_size(hw_params, &p_psize);
  fprintf(stderr, "buffer size %d\n", p_psize);

  snd_pcm_hw_params_free (hw_params);
	
  if ((err = snd_pcm_prepare (handle)) < 0) {
    fprintf (stderr, "cannot prepare audio interface for use (%s)\n",
	     snd_strerror (err));
    exit (1);
  }

  return 0;
}

int set_sw_params(snd_pcm_t *handle, char *name) {
  snd_pcm_sw_params_t *swparams;
  int err;

  snd_pcm_sw_params_alloca(&swparams);

  err = snd_pcm_sw_params_current(handle, swparams);
  if (err < 0) {
    fprintf(stderr, "Broken configuration for this PCM: no configurations available\n");
    exit(1);
  }
  
  err = snd_pcm_sw_params_set_start_threshold(handle, swparams, PERIOD_SIZE);
  if (err < 0) {
    printf("Unable to set start threshold: %s\n", snd_strerror(err));
    return err;
  }
  err = snd_pcm_sw_params_set_avail_min(handle, swparams, PERIOD_SIZE);
  if (err < 0) {
    printf("Unable to set avail min: %s\n", snd_strerror(err));
    return err;
  }

  if (snd_pcm_sw_params(handle, swparams) < 0) {
    fprintf(stderr, "unable to install sw params:\n");
    exit(1);
  }

  return 0;
}

snd_pcm_t *playback_handle;
snd_pcm_t *capture_handle;


void init_audio() {
    //char *in_device = "default";
    //char *out_device = "default";
    char *in_device = "hw:0,0";
    //char *out_device = "hw:0,0";
    char *out_device = "default";
    //char *out_device = "plug:dmix";

    int err;

    /**** Out card *******/
    if ((err = snd_pcm_open (&playback_handle, out_device, SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
	fprintf (stderr, "cannot open audio device %s (%s)\n", 
		 out_device,
		 snd_strerror (err));
	exit (1);
    }

    setparams(playback_handle, "playback");
    set_sw_params(playback_handle, "playback");


    /*********** In card **********/

    if ((err = snd_pcm_open (&capture_handle, in_device, SND_PCM_STREAM_CAPTURE, 0)) < 0) {
	fprintf (stderr, "cannot open audio device %s (%s)\n", 
		 in_device,
		 snd_strerror (err));
	exit (1);
    }

    setparams(capture_handle, "capture");
    set_sw_params(capture_handle, "capture");

    printf("Audio: set input ok\n");

    
    if ((err = snd_pcm_link(capture_handle, playback_handle)) < 0) {
	printf("Cannot link input and output audio streams: %s\n", snd_strerror(err));
	//exit(0);
    }
   

    if ((err = snd_pcm_prepare (playback_handle)) < 0) {
	fprintf (stderr, "cannot prepare playback audio interface for use (%s)\n",
		 snd_strerror (err));
	exit (1);
    }
    printf("Audio: set output ok\n");

}

void *play_audio(void *args) {
    int i;
  int err;
  int buf[BUF_SIZE];
  //snd_pcm_hw_params_t *hw_params;
  //FILE *fin;
  size_t nread;
  snd_pcm_format_t format = SND_PCM_FORMAT_S16_LE;

  // sound is buggered unless I return here with no audio
  //return;


  printf("Audio starting\n");
    /**************** stuff something into the playback buffer ****************/
    if (snd_pcm_format_set_silence(format, buf, 2*BUF_SIZE) < 0) {
	fprintf(stderr, "silence error\n");
	exit(1);
    }
  
    int n = 0;
    while (n++ < 2) {
	if (snd_pcm_writei (playback_handle, buf, BUF_SIZE) < 0) {
	    fprintf(stderr, "write error\n");
	    exit(1);
	}
    }
  
    /************* COPY ************/
    while (1) {
	int nread;
	if ((nread = snd_pcm_readi (capture_handle, buf, BUF_SIZE)) != BUF_SIZE) {
	    printf("Audio: read %d\n", nread);
	    if (nread < 0) {
		fprintf (stderr, "read from audio interface failed (%s)\n",
			 snd_strerror (nread));
	    } else {
		fprintf (stderr, "read from audio interface failed after %d frames\n", nread);
	    }	
	    snd_pcm_prepare(capture_handle);
	    continue;
	}
        
	if ((err = snd_pcm_writei (playback_handle, buf, nread)) != nread) {
	    if (err < 0) {
		fprintf (stderr, "write to audio interface failed (%s)\n",
			 snd_strerror (err));
	    } else {
		fprintf (stderr, "write to audio interface failed after %d frames\n", err);
	    }
	    snd_pcm_prepare(playback_handle);
	}
    }

}
