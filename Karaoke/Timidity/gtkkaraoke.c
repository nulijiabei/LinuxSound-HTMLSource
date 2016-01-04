
#include <gtk/gtk.h>

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>

#include "mytimidity.h"

//#define OVERLAY_IMAGE "jan-small.png"

GtkWidget *image;
GtkWidget *window;

#if GTK_MAJOR_VERSION == 2
GdkPixmap *pixmap;
#endif

/* FFMpeg vbls */
AVFormatContext *pFormatCtx = NULL;
AVCodecContext *pCodecCtx = NULL;
int videoStream;
struct SwsContext *sws_ctx = NULL;
AVCodec *pCodec = NULL;



static void pixmap_destroy_notify(guchar *pixels,
				  gpointer data) {
    printf("Destroy pixmap - not sure how\n");
}

static void *play_background(void *args) {
    /* based on code from
       http://www.cs.dartmouth.edu/~xy/cs23/gtk.html
       http://cdry.wordpress.com/2009/09/09/using-custom-io-callbacks-with-ffmpeg/
    */

    int i;
    AVPacket packet;
    int frameFinished;
    AVFrame *pFrame = NULL;

    int bytesDecoded;
    GdkPixbuf *pixbuf;
    //GdkPixbuf *overlay_pixbuf;
    AVFrame *picture_RGB;
    char *buffer;
    
    // Pango marked up text, half red, half black
    gchar *markup_text = "<span foreground=\"red\">hello</span><span foreground=\"black\">world</span>";
    PangoAttrList *attrs;
    gchar *text;

    pango_parse_markup (markup_text, -1,0, &attrs, &text, NULL, NULL);


    GError *error = NULL;
    /*
    overlay_pixbuf = gdk_pixbuf_new_from_file(OVERLAY_IMAGE, &error);
    if (!overlay_pixbuf) {
	fprintf(stderr, "%s\n", error->message);
	g_error_free(error);
	exit(1);
    }
    */

    /*
    // add an alpha layer for a white background
    overlay_pixbuf = gdk_pixbuf_add_alpha(overlay_pixbuf, TRUE, 255, 255, 255);

    int overlay_width = gdk_pixbuf_get_width(overlay_pixbuf);
    int overlay_height =  gdk_pixbuf_get_height(overlay_pixbuf);
    */

    pFrame=avcodec_alloc_frame();

    i=0;
    picture_RGB = avcodec_alloc_frame();
    buffer = malloc (avpicture_get_size(PIX_FMT_RGB24, 720, 576));
    avpicture_fill((AVPicture *)picture_RGB, buffer, PIX_FMT_RGB24, 720, 576);

    while(av_read_frame(pFormatCtx, &packet)>=0) {
	if(packet.stream_index==videoStream) {
	    usleep(33670);  // 29.7 frames per second
	    // Decode video frame
	    avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished,
				  &packet);

	    if (frameFinished) {
		printf("Frame %d\n", i++);
		int width = pCodecCtx->width;
		int height = pCodecCtx->height;

		sws_ctx = sws_getContext(width, height, pCodecCtx->pix_fmt, width, height, PIX_FMT_RGB24, SWS_BICUBIC, NULL, NULL, NULL);
		
		sws_scale(sws_ctx,  (uint8_t const * const *) pFrame->data, pFrame->linesize, 0, height, picture_RGB->data, picture_RGB->linesize);
		
		printf("old width %d new width %d\n",  pCodecCtx->width, picture_RGB->width);
		pixbuf = gdk_pixbuf_new_from_data(picture_RGB->data[0], GDK_COLORSPACE_RGB,
						  0, 8, width, height, 
						  picture_RGB->linesize[0], pixmap_destroy_notify,
						  NULL);

		// Create the destination surface
		cairo_surface_t *surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, 
								       width, height);
		cairo_t *cr = cairo_create(surface);

		// draw the background image
		gdk_cairo_set_source_pixbuf(cr, pixbuf, 0, 0);
		cairo_paint (cr);

		// overlay an image on top
		// alpha blending will be done by Cairo
		//gdk_cairo_set_source_pixbuf(cr, overlay_pixbuf, 300, 200);
		//cairo_paint (cr);

		// draw some white text on top
		cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
		// this is a standard font for Cairo
		cairo_select_font_face (cr, "cairo:serif",
					CAIRO_FONT_SLANT_NORMAL, 
					CAIRO_FONT_WEIGHT_BOLD);
		cairo_set_font_size (cr, 20);
		cairo_move_to(cr, 10.0, 50.0);
		cairo_show_text (cr, "hello");

		// draw Pango text
		PangoLayout *layout;
		PangoFontDescription *desc;

		cairo_move_to(cr, 300.0, 50.0);
		layout = pango_cairo_create_layout (cr);
		pango_layout_set_text (layout, text, -1);
		pango_layout_set_attributes(layout, attrs);
		pango_cairo_update_layout (cr, layout);
		pango_cairo_show_layout (cr, layout);


		/* start GTK thread lock for drawing */
		gdk_threads_enter();    

#if  GTK_MAJOR_VERSION == 2
		int pwidth, pheight, stride;
		unsigned char *data;

		data = cairo_image_surface_get_data (surface);
		pwidth = cairo_image_surface_get_width (surface);
		pheight = cairo_image_surface_get_height (surface);
		stride = cairo_image_surface_get_stride (surface);
		
		// this function doesn't work properly
		// code doesn't work
		pixmap = gdk_pixmap_create_from_data(NULL, data,
					      pwidth, pheight,
					      8, NULL, NULL);

		gtk_image_set_from_pixmap((GtkImage*) image, pixmap, NULL);

		gtk_widget_queue_draw(image);
#elif GTK_MAJOR_VERSION == 3
		pixbuf = gdk_pixbuf_get_from_surface(surface,
						     0,
						     0,
						     width,
						     height);

		gtk_image_set_from_pixbuf((GtkImage*) image, pixbuf); 

		gtk_widget_queue_draw(image);
#endif	      
		g_object_unref(pixbuf);
		/* release GTK thread lock */
		gdk_threads_leave();
	    }
	}
	av_free_packet(&packet);
    }

    printf("Video over!\n");
    exit(0);
}

extern ControlMode  *ctl;

ControlMode karaoke_ctl=
    {
	"karaoke interface", 'k',
	"karaoke",
	1,          /* verbosity */
	0,          /* trace playing */
	0,          /* opened */
	0,          /* flags */
	ctl_open,
	ctl_close,
	pass_playing_list,
	ctl_read,
	NULL,       /* write */
	cmsg,
	ctl_event
    };

static void init_timidity() {
   int err;

    timidity_start_initialize();

    if ((err = timidity_pre_load_configuration()) != 0) {
	printf("couldn't pre-load configuration file\n");
	exit(1);
    }

    err += timidity_post_load_configuration();

    if (err) {
	printf("couldn't post-load configuration file\n");
	exit(1);
    }

    timidity_init_player();

    ctl = &karaoke_ctl;
}

static FILE *outfp;
int karaoke_error_count;
static char *current_file;
struct midi_file_info *current_file_info;

static int pass_playing_list(int number_of_files, char *list_of_files[]) {
    int n;

    for (n = 0; n < number_of_files; n++) {
	printf("Playing list %s\n", list_of_files[n]);
	
	current_file = list_of_files[n];
	/*
	  current_file_info = get_midi_file_info(current_file, 1);
	  if (current_file_info != NULL) {
	  printf("file info not NULL\n");
	  } else {
	  printf("File info is NULL\n");
	  }
	*/
	play_midi_file( list_of_files[n]);
    }
    return 0;
}

/*ARGSUSED*/
static int ctl_open(int using_stdin, int using_stdout)
{
    // dont know what this function does
    /*
    if (current_file != NULL) {
	current_file_info = get_midi_file_info(current_file, 1);
	printf("Opening info for %s\n", current_file);
    } else {
	printf("Current is NULL\n");
    }
    */
    return 0;
}

static void ctl_close(void)
{
    fflush(outfp);
    karaoke_ctl.opened=0;
}

/*ARGSUSED*/
static int ctl_read(int32 *valp)
{
    return RC_NONE;
}

static int cmsg(int type, int verbosity_level, char *fmt, ...)
{
    va_list ap;

    if ((type==CMSG_TEXT || type==CMSG_INFO || type==CMSG_WARNING) &&
	karaoke_ctl.verbosity<verbosity_level)
	return 0;
    va_start(ap, fmt);
    if(type == CMSG_WARNING || type == CMSG_ERROR || type == CMSG_FATAL)
	karaoke_error_count++;
    if (!karaoke_ctl.opened)
	{
	    vfprintf(stderr, fmt, ap);
	    fputs(NLS, stderr);
	}
    else
	{
	    vfprintf(outfp, fmt, ap);
	    fputs(NLS, outfp);
	    fflush(outfp);
	}
    va_end(ap);
    return 0;
}

static void ctl_total_time(long tt)
{
    /*
    int mins, secs;
    if (karaoke_ctl.trace_playing)
	{
	    secs=(int)(tt/play_mode->rate);
	    mins=secs/60;
	    secs-=mins*60;
	    cmsg(CMSG_INFO, VERB_NORMAL,
		 "Total playing time: %3d min %02d s", mins, secs);
	}
    */
}

static void ctl_file_name(char *name)
{
    current_file = name;

    if (karaoke_ctl.verbosity>=0 || karaoke_ctl.trace_playing)
	cmsg(CMSG_INFO, VERB_NORMAL, "Playing %s", name);
}

static void ctl_current_time(int secs)
{
    int mins;
    static int prev_secs = -1;

#ifdef __W32__
    if(wrdt->id == 'w')
	return;
#endif /* __W32__ */
    if (karaoke_ctl.trace_playing && secs != prev_secs)
	{
	    prev_secs = secs;
	    mins=secs/60;
	    secs-=mins*60;
	    fprintf(outfp, "\r%3d:%02d", mins, secs);
	    fflush(outfp);
	}
}


#ifdef GTK_MAJOR_VERSION
struct _lyric_t {
    gchar *lyric;
    long tick;

};
typedef struct _lyric_t lyric_t;

struct _lyric_lines_t {
    char *language;
    char *title;
    char *performer;
    GArray *lines; // array of GString *
};
typedef struct _lyric_lines_t lyric_lines_t;

GArray *lyrics;
lyric_lines_t lyric_lines;

static void build_lyric_lines() {
    int n;
    lyric_t *plyric;
    GString *line = g_string_new("");
    GArray *lines =  g_array_sized_new(FALSE, FALSE, sizeof(GString *), 64);

    lyric_lines.title = NULL;

    //for (n = 0; n < lyrics->len; n++) {
    n = 1;
    char *evt_str;
    while ((evt_str = event2string(n++)) != NULL) {
        //plyric = g_array_index(lyrics, lyric_t *, n);
        //gchar *lyric = plyric->lyric;
        //int tick = plyric->tick;

	gchar *lyric = evt_str+1;

        if ((strlen(lyric) >= 2) && (lyric[0] == '@') && (lyric[1] == 'L')) {
            lyric_lines.language =  lyric + 2;
            continue;
        }

        if ((strlen(lyric) >= 2) && (lyric[0] == '@') && (lyric[1] == 'T')) {
            if (lyric_lines.title == NULL) {
                lyric_lines.title = lyric + 2;
            } else {
                lyric_lines.performer = lyric + 2;
            }
            continue;
        }

        if (lyric[0] == '@') {
            // some other stuff like @KMIDI KARAOKE FILE
            continue;
	}

	if ((lyric[0] == '/') || (lyric[0] == '\\')) {
	    // start of a new line
	    // add to lines
	    g_array_append_val(lines, line);
	    line = g_string_new(lyric + 1);
	}  else {
	    line = g_string_append(line, lyric);
	}
    }
    lyric_lines.lines = lines;
    
    printf("Title is %s, performer is %s, language is %s\n", 
	   lyric_lines.title, lyric_lines.performer, lyric_lines.language);
    for (n = 0; n < lines->len; n++) {
	printf("Line is %s\n", g_array_index(lines, GString *, n)->str);
    }
}
#else
void build_lyric_lines() {}
#endif


static void ctl_lyric(int lyricid)
{
    char *lyric;
   current_file_info = get_midi_file_info(current_file, 1);

    lyric = event2string(lyricid);
    if(lyric != NULL)
	{
	    if(lyric[0] == ME_KARAOKE_LYRIC)
		{
		    if(lyric[1] == '/' || lyric[1] == '\\')
			{
			    fprintf(outfp, "\n%s", lyric + 2);
			    fflush(outfp);
			}
		    else if(lyric[1] == '@')
			{
			    if(lyric[2] == 'L')
				fprintf(outfp, "\nLanguage: %s\n", lyric + 3);
			    else if(lyric[2] == 'T')
				fprintf(outfp, "Title: %s\n", lyric + 3);
			    else
				fprintf(outfp, "%s\n", lyric + 1);
			}
		    else
			{
			    fputs(lyric + 1, outfp);
			    fflush(outfp);
			}
		}
	    else
		{
		    if(lyric[0] == ME_CHORUS_TEXT || lyric[0] == ME_INSERT_TEXT)
			fprintf(outfp, "\r");
		    fputs(lyric + 1, outfp);
		    fflush(outfp);
		}
	}
}


static void ctl_event(CtlEvent *e)
{
    switch(e->type)
	{
	case CTLE_NOW_LOADING:
	    ctl_file_name((char *)e->v1);
	    break;
	case CTLE_LOADING_DONE:
	    // MIDI file is loaded, about to play
	    current_file_info = get_midi_file_info(current_file, 1);
	    if (current_file_info != NULL) {
		printf("file info not NULL\n");
	    } else {
		printf("File info is NULL\n");
	    }

	    int n = 1;
	    char *evt_str;
	    while ((evt_str = event2string(n++)) != NULL) {
		printf("Event in tabel: %s\n", evt_str);
	    }

	    build_lyric_lines();

	    break;
	case CTLE_PLAY_START:

	    ctl_total_time(e->v1);
	    break;
	case CTLE_CURRENT_TIME:
	    ctl_current_time((int)e->v1);
	    break;
#ifndef CFG_FOR_SF
	case CTLE_LYRIC:
	    ctl_lyric((int)e->v1);
	    break;
#endif
	}
}



static void *play_timidity(void *args) {
    char *argv[] = {"54154.kar"};
    
    timidity_play_main(1, argv);
    printf("Play timidity finished\n");
}


/* Called when the windows are realized
 */
static void realize_cb (GtkWidget *widget, gpointer data) {
    /* start the video playing in its own thread */
    pthread_t back_id;
    pthread_create(&back_id, NULL, play_background, NULL);

    pthread_t timidity_id;
    pthread_create(&timidity_id, NULL, play_timidity, NULL);
}

static gboolean delete_event( GtkWidget *widget,
                              GdkEvent  *event,
                              gpointer   data )
{
    /* If you return FALSE in the "delete-event" signal handler,
     * GTK will emit the "destroy" signal. Returning TRUE means
     * you don't want the window to be destroyed.
     * This is useful for popping up 'are you sure you want to quit?'
     * type dialogs. */

    g_print ("delete event occurred\n");

    /* Change TRUE to FALSE and the main window will be destroyed with
     * a "delete-event". */
    return TRUE;
}

/* Another callback */
static void destroy( GtkWidget *widget,
                     gpointer   data )
{
    gtk_main_quit ();
}

int main(int argc, char** argv)
{
    // Is this necessary?
    XInitThreads();

    int i;

    /* TiMidity */
    init_timidity();

    /* FFMpeg stuff */

    AVFrame *pFrame = NULL;
    AVPacket packet;

    AVDictionary *optionsDict = NULL;

    av_register_all();

    if(avformat_open_input(&pFormatCtx, "short.mpg", NULL, NULL)!=0)
	return -1; // Couldn't open file
  
    // Retrieve stream information
    if(avformat_find_stream_info(pFormatCtx, NULL)<0)
	return -1; // Couldn't find stream information
  
    // Dump information about file onto standard error
    av_dump_format(pFormatCtx, 0, argv[1], 0);
  
    // Find the first video stream
    videoStream=-1;
    for(i=0; i<pFormatCtx->nb_streams; i++)
	if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO) {
	    videoStream=i;
	    break;
	}
    if(videoStream==-1)
	return -1; // Didn't find a video stream

    for(i=0; i<pFormatCtx->nb_streams; i++)
	if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_AUDIO) {
	    printf("Found an audio stream too\n");
	    break;
	}

  
    // Get a pointer to the codec context for the video stream
    pCodecCtx=pFormatCtx->streams[videoStream]->codec;
  
    // Find the decoder for the video stream
    pCodec=avcodec_find_decoder(pCodecCtx->codec_id);
    if(pCodec==NULL) {
	fprintf(stderr, "Unsupported codec!\n");
	return -1; // Codec not found
    }
  
    // Open codec
    if(avcodec_open2(pCodecCtx, pCodec, &optionsDict)<0)
	return -1; // Could not open codec

    sws_ctx =
	sws_getContext
	(
	 pCodecCtx->width,
	 pCodecCtx->height,
	 pCodecCtx->pix_fmt,
	 pCodecCtx->width,
	 pCodecCtx->height,
	 PIX_FMT_YUV420P,
	 SWS_BILINEAR,
	 NULL,
	 NULL,
	 NULL
	 );

    /* GTK stuff now */

    /* This is called in all GTK applications. Arguments are parsed
     * from the command line and are returned to the application. */
    gtk_init (&argc, &argv);
    
    /* create a new window */
    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    
    /* When the window is given the "delete-event" signal (this is given
     * by the window manager, usually by the "close" option, or on the
     * titlebar), we ask it to call the delete_event () function
     * as defined above. The data passed to the callback
     * function is NULL and is ignored in the callback function. */
    g_signal_connect (window, "delete-event",
		      G_CALLBACK (delete_event), NULL);
    
    /* Here we connect the "destroy" event to a signal handler.  
     * This event occurs when we call gtk_widget_destroy() on the window,
     * or if we return FALSE in the "delete-event" callback. */
    g_signal_connect (window, "destroy",
		      G_CALLBACK (destroy), NULL);

    g_signal_connect (window, "realize", G_CALLBACK (realize_cb), NULL);
    
    /* Sets the border width of the window. */
    gtk_container_set_border_width (GTK_CONTAINER (window), 10);

    image = gtk_image_new();
    gtk_widget_show (image);

    /* This packs the button into the window (a gtk container). */
    gtk_container_add (GTK_CONTAINER (window), image);
        
    /* and the window */
    gtk_widget_show (window);
    
    /* All GTK applications must have a gtk_main(). Control ends here
     * and waits for an event to occur (like a key press or
     * mouse event). */
    gtk_main ();
    
    return 0;
}

