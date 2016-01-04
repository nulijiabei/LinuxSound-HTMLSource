

/* See http://cairographics.org/threaded_animation_with_cairo/
 * Threaded animations with Cairo
 */

#include <string.h>

#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <gdk/gdk.h>

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>

#define MIDI_FILE "54154.mid"
#define VIDEO_FILE "short.mpg"

GtkWidget *image;

GtkWidget *canvas;
GdkPixmap *dbuf_pixmap;

/* FFMpeg vbls */
AVFormatContext *pFormatCtx = NULL;
AVCodecContext *pCodecCtx = NULL;
int videoStream;
struct SwsContext *sws_ctx = NULL;
AVCodec *pCodec = NULL;

int already_drawing = 0;
cairo_t *real_cr;

static void pixmap_destroy_notify(guchar *pixels,
				  gpointer data) {
    printf("Ddestroy pixmap\n");
}

static void *play_midi(void *args) {
    char *argv[1];
    argv[0] = MIDI_FILE;
    int argc = 1;

    timidity_play_main(argc, argv);

    printf("Audio finished\n");
    exit(0);
}


static void *play_background(void *args) {
    /* based on code from
       http://www.cs.dartmouth.edu/~xy/cs23/gtk.html
       http://cdry.wordpress.com/2009/09/09/using-custom-io-callbacks-with-ffmpeg/
    */

    cairo_t *cr = (cairo_t *) args;
    //cr = gdk_cairo_create((GdkDrawable *) image);

    if (cr == real_cr) printf("cr is okay\n"); else printf("cr is different\n");
    cairo_surface_t *surface;

    /*
    //GdkRGBA color;
  int width = 720;
  int height = 480;

  width = gtk_widget_get_allocated_width (image);
  height = gtk_widget_get_allocated_height (image);
  cairo_arc (cr,
             width / 2.0, height / 2.0,
             MIN (width, height) / 2.0,
             0, 2 * G_PI);
cairo_set_line_width (cr, 0.1);
cairo_set_source_rgb (cr, 0, 0, 0);
cairo_rectangle (cr, 0.25, 0.25, 0.5, 0.5);
cairo_stroke (cr);
    */
  /*
  gtk_style_context_get_color (gtk_widget_get_style_context (image),
                               0,
                               &color);
			       gdk_cairo_set_source_rgba (cr, &color);
  */
    /*
  cairo_fill (cr);
  return;
    */

    int i;
    AVPacket packet;
    int frameFinished;
    AVFrame *pFrame = NULL;

    int oldSize;
    char *oldData;
    int bytesDecoded;
    GdkPixbuf *pixbuf;
    AVFrame *picture_RGB;
    char *buffer;

    GdkPixmap *pixmap;
    GdkBitmap *mask;

     cairo_surface_t *imagePicture = cairo_image_surface_create_from_png("/home/httpd/html/gartner_hype_cycle20112.png");

    pFrame=avcodec_alloc_frame();

    i=0;
    picture_RGB = avcodec_alloc_frame();
    buffer = malloc (avpicture_get_size(PIX_FMT_RGB24, 720, 576));
    avpicture_fill((AVPicture *)picture_RGB, buffer, PIX_FMT_RGB24, 720, 576);

    while(av_read_frame(pFormatCtx, &packet)>=0) {
	if(packet.stream_index==videoStream) {
	    //printf("Frame %d\n", i++);
	    usleep(33670);  // 29.7 frames per second
	    // Decode video frame
	    avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished,
				  &packet);

	    if (frameFinished) {
		int height = pCodecCtx->height;
		int width = pCodecCtx->width;
		int stride  = cairo_format_stride_for_width (CAIRO_FORMAT_ARGB32, width);
		//printf("Height: %d width %d stride %d\n", height, width, stride);

		sws_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height, PIX_FMT_RGB24, SWS_BICUBIC, NULL, NULL, NULL);
		
		sws_scale(sws_ctx,  (uint8_t const * const *) pFrame->data, pFrame->linesize, 0, pCodecCtx->height, picture_RGB->data, picture_RGB->linesize);

		/*
		pixbuf = gdk_pixbuf_new_from_data(picture_RGB->data[0], GDK_COLORSPACE_RGB, 0, 8, 720, 480, picture_RGB->linesize[0], pixmap_destroy_notify, NULL);
		*/
		
		/* start GTK thread lock for drawing */
		gdk_threads_enter();    

		/* GONE
		gdk_draw_pixbuf((GdkDrawable *) dbuf_pixmap, NULL,
				pixbuf, 
				0, 0, 0, 0, 720, 480,
				GDK_RGB_DITHER_NORMAL, 0, 0);
		gtk_image_set_from_pixmap((GtkImage*) image, dbuf_pixmap, NULL);
		*/
		//gtk_widget_queue_draw(image);
		unsigned char *data    = (unsigned char *) g_malloc0 (height * stride);
		surface = cairo_image_surface_create_for_data (picture_RGB->data[0],
		//surface = cairo_image_surface_create_for_data (data,
							       CAIRO_FORMAT_RGB24,
							       width,
							       height,
							       //picture_RGB->linesize[0]);
							       stride);
		//cairo_set_source_surface(cr, surface, 10,  10);
		cairo_set_source_surface(cr, imagePicture, 10, 10);
		cairo_paint(cr);

		//gtk_widget_queue_draw_area(image, 0, 0, width, height);

		//gdk_pixbuf_unref(pixbuf);

		/* release GTK thread lock */
		gdk_threads_leave();
	    }
	}
	av_free_packet(&packet);
    }
    printf("Video over!\n");
    exit(0);
}

/* Called when the windows are realized
 */
static gboolean on_draw_cb (GtkWidget *widget, cairo_t *cr, gpointer data) {
    // are we already drawing - safe check
    if (g_atomic_int_get(&already_drawing)) {
	return;
    } else {
	already_drawing++;
    }

    // works ok in the callback
    /*
     cairo_surface_t *imagePicture = cairo_image_surface_create_from_png("/home/httpd/html/gartner_hype_cycle20112.png");

     cairo_set_source_surface(cr, imagePicture, 10, 10);
     cairo_paint(cr);
     return;
    */

     real_cr = cr;

    /* play the midi files, if any */
    //fluid_player_play(player);

    // GONE dbuf_pixmap = gdk_pixmap_new(widget->window, 720, 480, -1);
    GError *error = NULL;

    /* start the video playing in its own thread */
    pthread_t tid_video;
    pthread_create(&tid_video, NULL, play_background, cr);

    /* start the MIDI file playing in its own thread */
    pthread_t tid_midi;
    //pthread_create(&tid_midi, NULL, play_midi, NULL);

    //gtk_widget_queue_draw_area(widget, 0, 0, 720, 480);

    return FALSE;
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

    int i;

    /* make X happy */
    XInitThreads();
    gdk_threads_init();

    /* Timidity stuff */
    int err;

    timidity_start_initialize();
    if ((err = timidity_pre_load_configuration()) == 0) {
	err = timidity_post_load_configuration();
    }
    if (err) {
        printf("couldn't load configuration file\n");
        exit(1);
    }

    timidity_init_player();

    /* FFMpeg stuff */

    AVFrame *pFrame = NULL;
    AVPacket packet;

    AVDictionary *optionsDict = NULL;


    av_register_all();

    if(avformat_open_input(&pFormatCtx, VIDEO_FILE, NULL, NULL)!=0)
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

   /* GtkWidget is the storage type for widgets */
    GtkWidget *window;
    GtkWidget *box;
    
    gdk_threads_enter();
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


    //g_signal_connect (window, "realize", G_CALLBACK (realize_cb), NULL);
    
    /* Sets the border width of the window. */
    gtk_container_set_border_width (GTK_CONTAINER (window), 10);

    box = gtk_vbox_new(TRUE, 1);
    //box = gtk_table_new(2, 1, TRUE);
    gtk_widget_show(box);

    /*
    char *str = "     ";
    lyric_labels[0] = gtk_label_new(str);
    str =  "World";
    lyric_labels[1] = gtk_label_new(str);
    */

    //image = gtk_image_new();
    image = gtk_drawing_area_new();
    g_signal_connect (image, "draw", G_CALLBACK (on_draw_cb), NULL);

    gtk_widget_show (image);
    
    // gtk_box_pack_start (GTK_BOX (box), lyric_labels[0], TRUE, TRUE, 0);

    gtk_box_pack_start (GTK_BOX (box), image, TRUE, TRUE, 0);
   
    /* This packs the button into the window (a gtk container). */
    gtk_container_add (GTK_CONTAINER (window), box);
        
    
    //GdkWindow *gwin;
    
    //gwin = gtk_widget_get_window(GTK_WIDGET(box));

    gtk_window_set_default_size(GTK_WINDOW(window), 720, 480);

    /* show all windows */
    gtk_widget_show (window);

    //gtk_widget_set_app_paintable(window, TRUE);
    //gtk_widget_set_double_buffered(window, FALSE);

    /* All GTK applications must have a gtk_main(). Control ends here
     * and waits for an event to occur (like a key press or
     * mouse event). */
    gtk_main ();

    gdk_threads_leave();    
    return 0;
}

