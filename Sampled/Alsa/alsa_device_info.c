/**
 * Paul Davis
 * http://equalarea.com/paul/alsa-audio.html#howto
 */

#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>
#include <locale.h>

#define _(STR) STR

// static snd_pcm_stream_t stream = SND_PCM_STREAM_CAPTURE;  // SND_PCM_STREAM_PLAYBACK;
	      

/*
 * Borrowed from aplay.c
 * http://alsa-utils.sourcearchive.com/documentation/1.0.15/aplay_8c-source.html
 */
static void device_list(snd_pcm_stream_t stream)
{
      snd_ctl_t *handle;
      int card, err, dev, idx;
      snd_ctl_card_info_t *info;
      snd_pcm_info_t *pcminfo;
      snd_ctl_card_info_alloca(&info);
      snd_pcm_info_alloca(&pcminfo);

      card = -1;
      if (snd_card_next(&card) < 0 || card < 0) {
            error(_("no soundcards found..."));
            return;
      }
      printf(_("**** List of %s Hardware Devices ****\n"),
             snd_pcm_stream_name(stream));
      while (card >= 0) {
            char name[32];
            sprintf(name, "hw:%d", card);
            if ((err = snd_ctl_open(&handle, name, 0)) < 0) {
                  error("control open (%i): %s", card, snd_strerror(err));
                  goto next_card;
            }
            if ((err = snd_ctl_card_info(handle, info)) < 0) {
                  error("control hardware info (%i): %s", card, snd_strerror(err));
                  snd_ctl_close(handle);
                  goto next_card;
            }
            dev = -1;
            while (1) {
                  unsigned int count;
                  if (snd_ctl_pcm_next_device(handle, &dev)<0)
                        error("snd_ctl_pcm_next_device");
                  if (dev < 0)
                        break;
                  snd_pcm_info_set_device(pcminfo, dev);
                  snd_pcm_info_set_subdevice(pcminfo, 0);
                  snd_pcm_info_set_stream(pcminfo, stream);
                  if ((err = snd_ctl_pcm_info(handle, pcminfo)) < 0) {
                        if (err != -ENOENT)
                              error("control digital audio info (%i): %s", card, snd_strerror(err));
                        continue;
                  }
                  printf(_("card %i: [%s,%i] %s [%s], device %i: %s [%s]\n"),
			 card, name, dev, snd_ctl_card_info_get_id(info), snd_ctl_card_info_get_name(info),
                        dev,
                        snd_pcm_info_get_id(pcminfo),
                        snd_pcm_info_get_name(pcminfo));
                  count = snd_pcm_info_get_subdevices_count(pcminfo);
                  printf( _("  Subdevices: %i/%i\n"),
                        snd_pcm_info_get_subdevices_avail(pcminfo), count);
                  for (idx = 0; idx < (int)count; idx++) {
                        snd_pcm_info_set_subdevice(pcminfo, idx);
                        if ((err = snd_ctl_pcm_info(handle, pcminfo)) < 0) {
                              error("control digital audio playback info (%i): %s", card, snd_strerror(err));
                        } else {
                              printf(_("  Subdevice #%i: %s\n"),
                                    idx, snd_pcm_info_get_subdevice_name(pcminfo));
                        }
                  }
            }
            snd_ctl_close(handle);
      next_card:
            if (snd_card_next(&card) < 0) {
                  error("snd_card_next");
                  break;
            }
      }
}


main (int argc, char *argv[])
{
  int i;
  int err;
  short buf[128];
  snd_pcm_t *playback_handle;
  snd_pcm_hw_params_t *hw_params;

  int card_id, ret;
  char *card_name;
  void **hints;
  int rc;
  snd_pcm_t *handle;



  card_id = -1;
  while (snd_card_next(&card_id) == 0) {
    if (card_id == -1) {
      printf("No more cards\n");
      break;
    }

    if (snd_card_get_name(card_id, &card_name) == 0) {
      printf("Sound card name %d %s\n", card_id, card_name);
    } else {
      printf("No name for %d\n", card_id);
    }

    if (snd_card_get_longname(card_id, &card_name) == 0) {
      printf("Sound card long name %d %s\n", card_id, card_name);
    } else {
      printf("No name for %d\n", card_id);
    }

    if (snd_device_name_hint(card_id, "pcm", &hints) == 0) {
      char *val;
      while (*hints != NULL) {
	val = snd_device_name_get_hint(*hints, "NAME");
	printf("   name hint %s\n", val);
	val = snd_device_name_get_hint(*hints, "DESC");
	printf("   desc hint %s\n", val);
	val = snd_device_name_get_hint(*hints, "IOID");
	printf("   i/o hint %s\n\n", val);
	hints++;
      }
      printf("\n");
    }
    
  }

  printf("DEVICE LIST\n");
  device_list(SND_PCM_STREAM_CAPTURE);
  device_list(SND_PCM_STREAM_PLAYBACK);

  /*
  if ((err = snd_pcm_open (&playback_handle, argv[1], SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
    fprintf (stderr, "cannot open audio device %s (%s)\n", 
	     argv[1],
	     snd_strerror (err));
    exit (1);
  }
		   
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
	
  if ((err = snd_pcm_hw_params_set_rate_near (playback_handle, hw_params, 44100, 0)) < 0) {
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
	
  snd_pcm_hw_params_free (hw_params);
	
  if ((err = snd_pcm_prepare (playback_handle)) < 0) {
    fprintf (stderr, "cannot prepare audio interface for use (%s)\n",
	     snd_strerror (err));
    exit (1);
  }
	
  for (i = 0; i < 10; ++i) {
    if ((err = snd_pcm_writei (playback_handle, buf, 128)) != 128) {
      fprintf (stderr, "write to audio interface failed (%s)\n",
	       snd_strerror (err));
      exit (1);
    }
  }
	
  snd_pcm_close (playback_handle);
  exit (0);

  */
}
