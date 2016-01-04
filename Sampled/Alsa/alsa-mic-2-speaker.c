/**
 * Paul Davis
 * http://equalarea.com/paul/alsa-audio.html#howto
 */

/**
 * Jan New march
 */

#define BUF_SIZE 1024

#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>


int setparams(snd_pcm_t *handle) {
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

  unsigned int rate = 44100;	
  if ((err = snd_pcm_hw_params_set_rate_near (handle, hw_params, &rate, 0)) < 0) {
    fprintf (stderr, "cannot set sample rate (%s)\n",
	     snd_strerror (err));
    exit (1);
  }
	
  if ((err = snd_pcm_hw_params_set_channels (handle, hw_params, 2)) < 0) {
    fprintf (stderr, "cannot set channel count (%s)\n",
	     snd_strerror (err));
    exit (1);
  }
	
  if ((err = snd_pcm_hw_params (handle, hw_params)) < 0) {
    fprintf (stderr, "cannot set parameters (%s)\n",
	     snd_strerror (err));
    exit (1);
  }

  snd_pcm_uframes_t periodsize = BUF_SIZE * 2;
  if ((err = snd_pcm_hw_params_set_buffer_size_near(handle, hw_params, &periodsize)) < 0) {
    printf("Unable to set buffer size %li: %s\n", BUF_SIZE * 2, snd_strerror(err));
    exit (1);;
  }
    periodsize /= 2;
    if ((err = snd_pcm_hw_params_set_period_size_near(handle, hw_params, &periodsize, 0)) < 0) {
    printf("Unable to set period size %li: %s\n", periodsize, snd_strerror(err));
    exit (1);
  }

  {
    snd_pcm_sw_params_t *sw_params;
    err = snd_pcm_sw_params_set_start_threshold(handle, sw_params, BUF_SIZE);
  }

  snd_pcm_hw_params_free (hw_params);
	
  if ((err = snd_pcm_prepare (handle)) < 0) {
    fprintf (stderr, "cannot prepare audio interface for use (%s)\n",
	     snd_strerror (err));
    exit (1);
  }
  return 0;
}
	      
main (int argc, char *argv[])
{
  int i;
  int err;
  int buf[BUF_SIZE];
  snd_pcm_t *playback_handle;
  snd_pcm_t *capture_handle;
  snd_pcm_hw_params_t *hw_params;
  FILE *fin;
  size_t nread;

  if (argc != 3) {
    fprintf(stderr, "Usage: %s in-card out-card\n", argv[0]);
    exit(1);
  }

 

 

  /**** Out card *******/
  if ((err = snd_pcm_open (&playback_handle, argv[2], SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
    fprintf (stderr, "cannot open audio device %s (%s)\n", 
	     argv[1],
	     snd_strerror (err));
    exit (1);
  }

  setparams(playback_handle);
  /*
  if ((err = snd_pcm_hw_params_malloc (&hw_params)) < 0) {
    fprintf (stderr, "cannot allocate hardware parameter structure (%s)\n",
	     snd_strerror (err));
    exit (1);
  }
				 
  if ((err = snd_pcm_hw_params_any (playback_handle, hw_params)) < 0) {
    fprintf (stderr, "cannot initialize hardware parameter structure (%s)\n",
	     snd_strerror (err));
    exit (1);
  }
	
  if ((err = snd_pcm_hw_params_set_access (playback_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
    fprintf (stderr, "cannot set access type (%s)\n",
	     snd_strerror (err));
    exit (1);
  }
	
  if ((err = snd_pcm_hw_params_set_format (playback_handle, hw_params, SND_PCM_FORMAT_S16_LE)) < 0) {
    fprintf (stderr, "cannot set sample format (%s)\n",
	     snd_strerror (err));
    exit (1);
  }

  unsigned int rate = 44100;	
  if ((err = snd_pcm_hw_params_set_rate_near (playback_handle, hw_params, &rate, 0)) < 0) {
    fprintf (stderr, "cannot set sample rate (%s)\n",
	     snd_strerror (err));
    exit (1);
  }
	
  if ((err = snd_pcm_hw_params_set_channels (playback_handle, hw_params, 2)) < 0) {
    fprintf (stderr, "cannot set channel count (%s)\n",
	     snd_strerror (err));
    exit (1);
  }
	
  if ((err = snd_pcm_hw_params (playback_handle, hw_params)) < 0) {
    fprintf (stderr, "cannot set parameters (%s)\n",
	     snd_strerror (err));
    exit (1);
  }

  snd_pcm_uframes_t periodsize = BUF_SIZE * 2;
  if ((err = snd_pcm_hw_params_set_buffer_size_near(playback_handle, hw_params, &periodsize)) < 0) {
    printf("Unable to set buffer size %li: %s\n", BUF_SIZE * 2, snd_strerror(err));
    exit (1);;
  }
    periodsize /= 2;
    if ((err = snd_pcm_hw_params_set_period_size_near(playback_handle, hw_params, &periodsize, 0)) < 0) {
    printf("Unable to set period size %li: %s\n", periodsize, snd_strerror(err));
    exit (1);
  }

  {
    snd_pcm_sw_params_t *sw_params;
    err = snd_pcm_sw_params_set_start_threshold(playback_handle, sw_params, BUF_SIZE);
  }

  snd_pcm_hw_params_free (hw_params);
	
  if ((err = snd_pcm_prepare (playback_handle)) < 0) {
    fprintf (stderr, "cannot prepare audio interface for use (%s)\n",
	     snd_strerror (err));
    exit (1);
  }
  */


  /*********** In card **********/

  if ((err = snd_pcm_open (&capture_handle, argv[1], SND_PCM_STREAM_CAPTURE, 0)) < 0) {
    fprintf (stderr, "cannot open audio device %s (%s)\n", 
	     argv[1],
	     snd_strerror (err));
    exit (1);
  }

  setparams(capture_handle);

  /*		   
  if ((err = snd_pcm_hw_params_malloc (&hw_params)) < 0) {
    fprintf (stderr, "cannot allocate hardware parameter structure (%s)\n",
	     snd_strerror (err));
    exit (1);
  }
				 
  if ((err = snd_pcm_hw_params_any (capture_handle, hw_params)) < 0) {
    fprintf (stderr, "cannot initialize hardware parameter structure (%s)\n",
	     snd_strerror (err));
    exit (1);
  }
	
  if ((err = snd_pcm_hw_params_set_access (capture_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
    fprintf (stderr, "cannot set access type (%s)\n",
	     snd_strerror (err));
    exit (1);
  }
	

  if ((err = snd_pcm_hw_params_set_format (capture_handle, hw_params, SND_PCM_FORMAT_S16_LE)) < 0) {
    fprintf (stderr, "cannot set sample format (%s)\n",
	     snd_strerror (err));
    exit (1);
  }

  rate = 44100;	
  if ((err = snd_pcm_hw_params_set_rate_near (capture_handle, hw_params, &rate, 0)) < 0) {
    fprintf (stderr, "cannot set sample rate (%s)\n",
	     snd_strerror (err));
    exit (1);
  }
	
  if ((err = snd_pcm_hw_params_set_channels (capture_handle, hw_params, 2)) < 0) {
    fprintf (stderr, "cannot set channel count (%s)\n",
	     snd_strerror (err));
    exit (1);
  }
	
  if ((err = snd_pcm_hw_params (capture_handle, hw_params)) < 0) {
    fprintf (stderr, "cannot set parameters (%s)\n",
	     snd_strerror (err));
    exit (1);
  }

  periodsize = BUF_SIZE * 2;
  if ((err = snd_pcm_hw_params_set_buffer_size_near(capture_handle, hw_params, &periodsize)) < 0) {
    printf("Unable to set buffer size %li: %s\n", BUF_SIZE * 2, snd_strerror(err));
    exit (1);;
  }
    periodsize /= 2;
    if ((err = snd_pcm_hw_params_set_period_size_near(capture_handle, hw_params, &periodsize, 0)) < 0) {
    printf("Unable to set period size %li: %s\n", periodsize, snd_strerror(err));
    exit (1);
  }
	
  snd_pcm_hw_params_free (hw_params);
	
  if ((err = snd_pcm_prepare (capture_handle)) < 0) {
    fprintf (stderr, "cannot prepare audio interface for use (%s)\n",
	     snd_strerror (err));
    exit (1);
  }
  */

  /************* COPY ************/
  while (1) {
    if ((err = snd_pcm_readi (capture_handle, buf, BUF_SIZE)) != BUF_SIZE) {
      fprintf (stderr, "read from audio interface failed (%s)\n",
	       snd_strerror (err));
      break;
    }
    printf("copying\n");
    if ((err = snd_pcm_writei (playback_handle, buf, BUF_SIZE)) != BUF_SIZE) {
      fprintf (stderr, "write to audio interface failed (%s)\n",
	       snd_strerror (err));
      //exit (1);
    }
  }


  snd_pcm_drain(playback_handle);	
  snd_pcm_close (playback_handle);
  exit (0);
}
