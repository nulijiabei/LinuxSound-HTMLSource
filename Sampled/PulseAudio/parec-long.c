


#include <stdio.h>
#include <string.h>
#include <pulse/pulseaudio.h>

// ???
#define CLEAR_LINE "\n"

// From pulsecore/macro.h JN
#define pa_memzero(x,l) (memset((x), 0, (l)))
#define pa_zero(x) (pa_memzero(&(x), sizeof(x)))

int fdout;
char *fname = "tmp.pcm";

int verbose = 1;
int ret;

static pa_sample_spec sample_spec = {
  .format = PA_SAMPLE_S16LE,
  .rate = 44100,
  .channels = 2
};

static pa_stream *stream = NULL;
static pa_channel_map channel_map;
static pa_proplist *proplist = NULL;

void *buffer = NULL; // pa_xmalloc(4096);

static char *device = "alsa_input.pci-0000_00_1b.0.analog-stereo";

static pa_stream_flags_t flags = 0;

static size_t latency = 0, process_time = 0;
static int32_t latency_msec = 0, process_time_msec = 0;

static int raw = 1;

void stream_state_callback(pa_stream *s, void *userdata) {
  assert(s);

  switch (pa_stream_get_state(s)) {
  case PA_STREAM_CREATING:
  case PA_STREAM_TERMINATED:
    break;

  case PA_STREAM_READY:

    if (verbose) {
      const pa_buffer_attr *a;
      char cmt[PA_CHANNEL_MAP_SNPRINT_MAX], sst[PA_SAMPLE_SPEC_SNPRINT_MAX];

      printf("Stream successfully created.");

      if (!(a = pa_stream_get_buffer_attr(s)))
	printf("pa_stream_get_buffer_attr() failed: %s", pa_strerror(pa_context_errno(pa_stream_get_context(s))));
      else {
	printf("Buffer metrics: maxlength=%u, fragsize=%u", a->maxlength, a->fragsize);
                    
      }
      /*
	printf("Using sample spec '%s', channel map '%s'.",
	pa_sample_spec_snprint(sst, sizeof(sst), pa_stream_get_sample_spec(s)),
	pa_channel_map_snprint(cmt, sizeof(cmt), pa_stream_get_channel_map(s)));
      */

      printf("Connected to device %s (%u, %ssuspended).",
	     pa_stream_get_device_name(s),
	     pa_stream_get_device_index(s),
	     pa_stream_is_suspended(s) ? "" : "not ");
    }

    break;

  case PA_STREAM_FAILED:
  default:
    printf("Stream error: %s", pa_strerror(pa_context_errno(pa_stream_get_context(s))));
    exit(1); //quit(1);
  }
}


/*********** Stream callbacks **************/

/* Write some data to the stream */
static void do_stream_write(size_t length) {
  size_t l;
  assert(length);

  /*
    if (!buffer || !buffer_length)
    return;

    l = length;
    if (l > buffer_length)
    l = buffer_length;

    if (pa_stream_write(stream, (uint8_t*) buffer + buffer_index, l, NULL, 0, PA_SEEK_RELATIVE) < 0) {
    printf("pa_stream_write() failed: %s", pa_strerror(pa_context_errno(context)));
    quit(1);
    return;
    }

    buffer_length -= l;
    buffer_index += l;

    if (!buffer_length) {
    pa_xfree(buffer);
    buffer = NULL;
    buffer_index = buffer_length = 0;
    }
  */
}

/* This is called whenever new data may be written to the stream */
static void stream_write_callback(pa_stream *s, size_t length, void *userdata) {
  assert(s);
  assert(length > 0);

  /*
    if (raw) {
    assert(!sndfile);

    if (stdio_event)
    mainloop_api->io_enable(stdio_event, PA_IO_EVENT_INPUT);

    if (!buffer)
    return;

    do_stream_write(length);

    } else {
    sf_count_t bytes;
    void *data;

    assert(sndfile);

    for (;;) {
    size_t data_length = length;

    if (pa_stream_begin_write(s, &data, &data_length) < 0) {
    printf("pa_stream_begin_write() failed: %s", pa_strerror(pa_context_errno(context)));
    quit(1);
    return;
    }

    if (readf_function) {
    size_t k = pa_frame_size(&sample_spec);

    if ((bytes = readf_function(sndfile, data, (sf_count_t) (data_length/k))) > 0)
    bytes *= (sf_count_t) k;

    } else
    bytes = sf_read_raw(sndfile, data, (sf_count_t) data_length);

    if (bytes > 0)
    pa_stream_write(s, data, (size_t) bytes, NULL, 0, PA_SEEK_RELATIVE);
    else
    pa_stream_cancel_write(s);

    /* EOF?
    if (bytes < (sf_count_t) data_length) {
    start_drain();
    break;
    }

    /* Request fulfilled 
    if ((size_t) bytes >= length)
    break;

    length -= bytes;
    }
    }
  */
}

/* This is called whenever new data may is available */
static void stream_read_callback(pa_stream *s, size_t length, void *userdata) {

  assert(s);
  assert(length > 0);

  printf("Ready to read data\n");

  while (pa_stream_readable_size(s) > 0) {
    const void *data;
    size_t length;
    
    if (pa_stream_peek(s, &data, &length) < 0) {
      //printf("pa_stream_peek() failed: %s", pa_strerror(pa_context_errno(context)));
      fprintf(stderr, "Read failed\n");
      exit(1); //quit(1);
      return;
    }
    write(fdout, data, length);
    pa_stream_drop(s);
  }
  /*
    if (raw) {
    assert(!sndfile);

    if (stdio_event)
    mainloop_api->io_enable(stdio_event, PA_IO_EVENT_OUTPUT);

    while (pa_stream_readable_size(s) > 0) {
    const void *data;

    if (pa_stream_peek(s, &data, &length) < 0) {
    printf("pa_stream_peek() failed: %s", pa_strerror(pa_context_errno(context)));
    quit(1);
    return;
    }

    assert(data);
    assert(length > 0);

    if (buffer) {
    buffer = pa_xrealloc(buffer, buffer_length + length);
    memcpy((uint8_t*) buffer + buffer_length, data, length);
    buffer_length += length;
    } else {
    buffer = pa_xmalloc(length);
    memcpy(buffer, data, length);
    buffer_length = length;
    buffer_index = 0;
    }

    pa_stream_drop(s);
    }

    } else {
    assert(sndfile);

    while (pa_stream_readable_size(s) > 0) {
    sf_count_t bytes;
    const void *data;

    if (pa_stream_peek(s, &data, &length) < 0) {
    printf("pa_stream_peek() failed: %s", pa_strerror(pa_context_errno(context)));
    quit(1);
    return;
    }

    assert(data);
    assert(length > 0);

    if (writef_function) {
    size_t k = pa_frame_size(&sample_spec);

    if ((bytes = writef_function(sndfile, data, (sf_count_t) (length/k))) > 0)
    bytes *= (sf_count_t) k;

    } else
    bytes = sf_write_raw(sndfile, data, (sf_count_t) length);

    if (bytes < (sf_count_t) length)
    quit(1);

    pa_stream_drop(s);
    }
    }
  */
}


static void stream_suspended_callback(pa_stream *s, void *userdata) {
  assert(s);

  if (verbose) {
    if (pa_stream_is_suspended(s))
      printf("Stream device suspended.%s", CLEAR_LINE);
    else
      printf("Stream device resumed.%s", CLEAR_LINE);
  }
}

static void stream_underflow_callback(pa_stream *s, void *userdata) {
  assert(s);

  if (verbose)
    printf("Stream underrun.%s",  CLEAR_LINE);
}

static void stream_overflow_callback(pa_stream *s, void *userdata) {
  assert(s);

  if (verbose)
    printf("Stream overrun.%s", CLEAR_LINE);
}

static void stream_started_callback(pa_stream *s, void *userdata) {
  assert(s);

  if (verbose)
    printf("Stream started.%s", CLEAR_LINE);
}

static void stream_moved_callback(pa_stream *s, void *userdata) {
  assert(s);

  if (verbose)
    printf("Stream moved to device %s (%u, %ssuspended).%s", pa_stream_get_device_name(s), pa_stream_get_device_index(s), pa_stream_is_suspended(s) ? "" : "not ",  CLEAR_LINE);
}

static void stream_buffer_attr_callback(pa_stream *s, void *userdata) {
  assert(s);

  if (verbose)
    printf("Stream buffer attributes changed.%s",  CLEAR_LINE);
}

static void stream_event_callback(pa_stream *s, const char *name, pa_proplist *pl, void *userdata) {
  char *t;

  assert(s);
  assert(name);
  assert(pl);

  t = pa_proplist_to_string_sep(pl, ", ");
  printf("Got event '%s', properties '%s'", name, t);

  /* don't know what htis is
     if (pa_streq(name, PA_STREAM_EVENT_REQUEST_CORK)) {
     if (cork_requests == 0) {
     printf("Cork request stack is empty: corking stream");
     pa_operation_unref(pa_stream_cork(s, 1, NULL, NULL));
     }
     cork_requests++;
     } else if (pa_streq(name, PA_STREAM_EVENT_REQUEST_UNCORK)) {
     if (cork_requests == 1) {
     printf("Cork request stack is empty: uncorking stream");
     pa_operation_unref(pa_stream_cork(s, 0, NULL, NULL));
     }
     if (cork_requests == 0)
     printf("Warning: Received more uncork requests than cork requests!");
     else
     cork_requests--;
     }
  */

  pa_xfree(t);
}




// This callback gets called when our context changes state.  We really only
// care about when it's ready or if it has failed
void state_cb(pa_context *c, void *userdata) {
  pa_context_state_t state;
  int *pa_ready = userdata;

  printf("State changed\n");
  state = pa_context_get_state(c);
  switch  (state) {
    // There are just here for reference
  case PA_CONTEXT_UNCONNECTED:
  case PA_CONTEXT_CONNECTING:
  case PA_CONTEXT_AUTHORIZING:
  case PA_CONTEXT_SETTING_NAME:
  default:
    break;
  case PA_CONTEXT_FAILED:
  case PA_CONTEXT_TERMINATED:
    *pa_ready = 2;
    break;
  case PA_CONTEXT_READY: {
    pa_buffer_attr buffer_attr;

    if (verbose)
      printf("Connection established.%s\n", CLEAR_LINE);

    if (!(stream = pa_stream_new(c, "JanCapture", &sample_spec, NULL))) {
      printf("pa_stream_new() failed: %s", pa_strerror(pa_context_errno(c)));
      exit(1); // goto fail;
    }

    pa_stream_set_state_callback(stream, stream_state_callback, NULL);
    
    pa_stream_set_write_callback(stream, stream_write_callback, NULL);
    pa_stream_set_read_callback(stream, stream_read_callback, NULL);
    pa_stream_set_suspended_callback(stream, stream_suspended_callback, NULL);
    pa_stream_set_moved_callback(stream, stream_moved_callback, NULL);
    pa_stream_set_underflow_callback(stream, stream_underflow_callback, NULL);
    pa_stream_set_overflow_callback(stream, stream_overflow_callback, NULL);
    pa_stream_set_started_callback(stream, stream_started_callback, NULL);
    pa_stream_set_event_callback(stream, stream_event_callback, NULL);
    pa_stream_set_buffer_attr_callback(stream, stream_buffer_attr_callback, NULL);
    

    pa_zero(buffer_attr);
    buffer_attr.maxlength = (uint32_t) -1;
    buffer_attr.prebuf = (uint32_t) -1;

    if (latency_msec > 0) {
      buffer_attr.fragsize = buffer_attr.tlength = pa_usec_to_bytes(latency_msec * PA_USEC_PER_MSEC, &sample_spec);
      flags |= PA_STREAM_ADJUST_LATENCY;
    } else if (latency > 0) {
      buffer_attr.fragsize = buffer_attr.tlength = (uint32_t) latency;
      flags |= PA_STREAM_ADJUST_LATENCY;
    } else
      buffer_attr.fragsize = buffer_attr.tlength = (uint32_t) -1;

    if (process_time_msec > 0) {
      buffer_attr.minreq = pa_usec_to_bytes(process_time_msec * PA_USEC_PER_MSEC, &sample_spec);
    } else if (process_time > 0)
      buffer_attr.minreq = (uint32_t) process_time;
    else
      buffer_attr.minreq = (uint32_t) -1;

      if (pa_stream_connect_record(stream, device, &buffer_attr, flags) < 0) {
	printf("pa_stream_connect_record() failed: %s", pa_strerror(pa_context_errno(c)));
	exit(1); //goto fail;
      }
    /*
      if (mode == PLAYBACK) {
      pa_cvolume cv;
      if (pa_stream_connect_playback(stream, device, &buffer_attr, flags, volume_is_set ? pa_cvolume_set(&cv, sample_spec.channels, volume) : NULL, NULL) < 0) {
      printf("pa_stream_connect_playback() failed: %s", pa_strerror(pa_context_errno(c)));
      goto fail;
      }

      } else {
      if (pa_stream_connect_record(stream, device, &buffer_attr, flags) < 0) {
      printf("pa_stream_connect_record() failed: %s", pa_strerror(pa_context_errno(c)));
      goto fail;
      }
      }
    */
  }

    break;
  }
}

int main(int argc, char *argv[]) {

  // Define our pulse audio loop and connection variables
  pa_mainloop *pa_ml;
  pa_mainloop_api *pa_mlapi;
  pa_operation *pa_op;
  pa_context *pa_ctx;

  // We'll need these state variables to keep track of our requests
  int state = 0;
  int pa_ready = 0;

  fdout = creat(fname,  0711);

  // Create a mainloop API and connection to the default server
  pa_ml = pa_mainloop_new();
  pa_mlapi = pa_mainloop_get_api(pa_ml);
  pa_ctx = pa_context_new(pa_mlapi, "test");

  // This function connects to the pulse server
  pa_context_connect(pa_ctx, NULL, 0, NULL);
  printf("Connecting\n");

  // This function defines a callback so the server will tell us it's state.
  // Our callback will wait for the state to be ready.  The callback will
  // modify the variable to 1 so we know when we have a connection and it's
  // ready.
  // If there's an error, the callback will set pa_ready to 2
  pa_context_set_state_callback(pa_ctx, state_cb, &pa_ready);


  if (pa_mainloop_run(pa_ml, &ret) < 0) {
    printf("pa_mainloop_run() failed.");
    exit(1); // goto quit
  }


  /*
  // Now we'll enter into an infinite loop until we get the data we receive
  // or if there's an error
  for (;;) {
  // We can't do anything until PA is ready, so just iterate the mainloop
  // and continue
  if (pa_ready == 0) {
  pa_mainloop_iterate(pa_ml, 1, NULL);
  continue;
  }
  // We couldn't get a connection to the server, so exit out
  if (pa_ready == 2) {
  pa_context_disconnect(pa_ctx);
  pa_context_unref(pa_ctx);
  pa_mainloop_free(pa_ml);
  return -1;
  }
  // At this point, we're connected to the server and ready to make
  // requests
  switch (state) {
  // State 0: we haven't done anything yet
  case 0:
  // This sends an operation to the server.  pa_sinklist_info is
  // our callback function and a pointer to our devicelist will
  // be passed to the callback The operation ID is stored in the
  // pa_op variable
  pa_op = pa_context_get_sink_info_list(pa_ctx,
  pa_sinklist_cb,
  output
  );

  // Update state for next iteration through the loop
  state++;
  break;
  case 1:
  // Now we wait for our operation to complete.  When it's
  // complete our pa_output_devicelist is filled out, and we move
  // along to the next state
  if (pa_operation_get_state(pa_op) == PA_OPERATION_DONE) {
  pa_operation_unref(pa_op);

  // Now we perform another operation to get the source
  // (input device) list just like before.  This time we pass
  // a pointer to our input structure
  pa_op = pa_context_get_source_info_list(pa_ctx,
  pa_sourcelist_cb,
  input
  );
  // Update the state so we know what to do next
  state++;
  }
  break;
  case 2:
  if (pa_operation_get_state(pa_op) == PA_OPERATION_DONE) {
  // Now we're done, clean up and disconnect and return
  pa_operation_unref(pa_op);
  pa_context_disconnect(pa_ctx);
  pa_context_unref(pa_ctx);
  pa_mainloop_free(pa_ml);
  return 0;
  }
  break;
  default:
  // We should never see this state
  fprintf(stderr, "in state %d\n", state);
  return -1;
  }
  // Iterate the main loop and go again.  The second argument is whether
  // or not the iteration should block until something is ready to be
  // done.  Set it to zero for non-blocking.
  pa_mainloop_iterate(pa_ml, 1, NULL);
  }
  */
}
