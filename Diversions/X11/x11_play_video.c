
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#if 0
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#endif 

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>

Display *dpy;
Window window;
GC gc;

pthread_t tid = -1;

#if 0
GtkWidget *image;
GtkWidget *window;

#if GTK_MAJOR_VERSION == 2
GdkPixmap *pixmap;
#endif
#endif // Gtk

/* FFMpeg vbls */
AVFormatContext *pFormatCtx = NULL;
AVCodecContext *pCodecCtx = NULL;
int videoStream;
struct SwsContext *sws_ctx = NULL;
AVCodec *pCodec = NULL;


/*
static void pixmap_destroy_notify(guchar *pixels,
				  gpointer data) {
    printf("Destroy pixmap - not sure how\n");
}
*/

XImage *image = NULL;
static XWindowAttributes attribs;
int depth;
static XVisualInfo vinfo;

/*
 * Scan the available visuals on this Display/Screen.  Try to find
 * the 'best' available TrueColor visual that has a decent color
 * depth (at least 15bit).  If there are multiple visuals with depth
 * >= 15bit, we prefer visuals with a smaller color depth.
 */
static int vo_find_depth_from_visuals(Display * dpy, int screen,
                                      Visual ** visual_return)
{
    XVisualInfo visual_tmpl;
    XVisualInfo *visuals;
    int nvisuals, i;
    int bestvisual = -1;
    int bestvisual_depth = -1;

    visual_tmpl.screen = screen;
    visual_tmpl.class = TrueColor;
    visuals = XGetVisualInfo(dpy,
                             VisualScreenMask | VisualClassMask,
                             &visual_tmpl, &nvisuals);
    if (visuals != NULL)
    {
        for (i = 0; i < nvisuals; i++)
        {
            /*
             * Save the visual index and its depth, if this is the first
             * truecolor visul, or a visual that is 'preferred' over the
             * previous 'best' visual.
             */
            if (bestvisual_depth == -1
                || (visuals[i].depth >= 15
                    && (visuals[i].depth < bestvisual_depth
                        || bestvisual_depth < 15)))
            {
                bestvisual = i;
                bestvisual_depth = visuals[i].depth;
            }
       }

        if (bestvisual != -1 && visual_return != NULL)
            *visual_return = visuals[bestvisual].visual;

        XFree(visuals);
    }
    return bestvisual_depth;
}

static void freeImage() {
    XDestroyImage(image);
    image = NULL;
}

static void fillImage(uint8_t *data) {
    int m,n;
    int height = 480;
    int width = 720;

    /*
    for (m = 0; m < width; m++) {
	for (n = 0; n < height; n++) {
	    int base = m*width + n*3;
	    long pixel = (data[n] << 16) + (data[n+1] << 8) + data[n+2];
	    XPutPixel(image, m, n, pixel);
	}
    }
    */
    for (m = 0; m < width; m++) {
	for (n = 0; n < height*3; n++) {
	    image->data[m*width + n] = data[m*width + n];
	}
    }
}

static void getImage(uint8_t *data) {
    if (image != NULL) {
	freeImage();
    }
    int width = 720;
    int height = 480;
    image =  XCreateImage(dpy, vinfo.visual, depth, ZPixmap,
                             0, NULL, 720, 480, 8, 0);
    image->data = malloc(image->bytes_per_line * height);
    memset(image->data, 0, image->bytes_per_line * height);
    printf("Image bits per pixel %d\n", image->bits_per_pixel);

    fillImage(data);
    //Pixmap p = XCreatePixmap(dpy, XDefaultRootWindow(dpy), width, height, depth);
    XPutImage(dpy, window, DefaultGC(dpy, 0), image, 0, 0, 0, 0, width, height);
    XFlush(dpy);
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

    /* initialize packet, set data to NULL, let the demuxer fill it */
    /* http://ffmpeg.org/doxygen/trunk/doc_2examples_2demuxing_8c-example.html#a80 */
    av_init_packet(&packet);
    packet.data = NULL;
    packet.size = 0;

    int bytesDecoded;
    //GdkPixbuf *pixbuf;
    AVFrame *picture_RGB;
    char *buffer;

    pFrame=avcodec_alloc_frame();

    i=0;
    picture_RGB = avcodec_alloc_frame();
    buffer = malloc (avpicture_get_size(PIX_FMT_RGB24, 720, 576));
    avpicture_fill((AVPicture *)picture_RGB, buffer, PIX_FMT_RGB24, 720, 576);

    int width = pCodecCtx->width;
    int height = pCodecCtx->height;
    
    sws_ctx = sws_getContext(width, height, pCodecCtx->pix_fmt, width, height, 
			     PIX_FMT_RGB24, SWS_BICUBIC, NULL, NULL, NULL);

    while(av_read_frame(pFormatCtx, &packet)>=0) {
	if(packet.stream_index==videoStream) {
	    usleep(33670);  // 29.7 frames per second
	    // Decode video frame
	    avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished,
				  &packet);


	    if (frameFinished) {
		printf("Frame %d\n", i++);
		
		sws_scale(sws_ctx,  (uint8_t const * const *) pFrame->data, pFrame->linesize, 0, height, picture_RGB->data, picture_RGB->linesize);
		
		printf("old width %d new width %d\n",  pCodecCtx->width, picture_RGB->width);

		getImage(picture_RGB->data[0]);
#if 0
		pixbuf = gdk_pixbuf_new_from_data(picture_RGB->data[0], GDK_COLORSPACE_RGB,
						  0, 8, width, height, 
						  picture_RGB->linesize[0], pixmap_destroy_notify,
						  NULL);
				


		/* start GTK thread lock for drawing */
		gdk_threads_enter();    

#if  GTK_MAJOR_VERSION == 2
		/* this leaks memory somehow */
		pixmap = gdk_pixmap_new(window->window, 720, 480, -1);
	
		gdk_draw_pixbuf((GdkDrawable *) pixmap, NULL,
				pixbuf, 
				0, 0, 0, 0, 720, 480,
				GDK_RGB_DITHER_NORMAL, 0, 0);

		gtk_image_set_from_pixmap((GtkImage*) image, pixmap, NULL);

		//gtk_widget_queue_draw(image);
#elif GTK_MAJOR_VERSION == 3
		gtk_image_set_from_pixbuf((GtkImage*) image, pixbuf);
		
		//gtk_widget_queue_draw(image);
		
#endif	      
		//g_object_unref(pixbuf);
		//sws_freeContext(sws_ctx);
		/* release GTK thread lock */
		gdk_threads_leave();
#endif // 0
	    }
	}
	av_free_packet(&packet);
    }

    printf("Video over!\n");
    exit(0);
}

#if 0
/* Called when the windows are realized
 */
static void realize_cb (GtkWidget *widget, gpointer data) {
    /* start the video playing in its own thread */
    pthread_t tid;
    pthread_create(&tid, NULL, play_background, NULL);
}
#endif

#if 0
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
#endif

int main(int argc, char** argv)
{
    // Is this necessary?
    XInitThreads();

    int i;


    /* FFMpeg stuff */

    AVFrame *pFrame = NULL;
    AVPacket packet;

    AVDictionary *optionsDict = NULL;

    av_register_all();

    if(avformat_open_input(&pFormatCtx, "short.mpg", NULL, NULL)!=0) {
	fprintf(stderr, "no file\n");
	return -1; // Couldn't open file
    }
  
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

    // X now
    dpy = XOpenDisplay(NULL);

    int blackColor = BlackPixel(dpy, DefaultScreen(dpy));
    int whiteColor = WhitePixel(dpy, DefaultScreen(dpy));
    window = XCreateSimpleWindow(dpy, DefaultRootWindow(dpy), 0, 0, 
				     720, 480, 0, blackColor, blackColor);
    XSelectInput(dpy, window, StructureNotifyMask);
    XMapWindow(dpy, window);
    gc = XCreateGC(dpy, window, 0, NULL);

    XGetWindowAttributes(dpy, window, &attribs);
    depth = attribs.depth;
    if (depth != 15 && depth != 16 && depth != 24 && depth != 32)
    {
	fprintf(stderr, "Worong depth\n");
	exit(1);
    }

    XMatchVisualInfo(dpy, 0, depth, DirectColor, &vinfo);

    for(;;) {
	XEvent e;
	XNextEvent(dpy, &e);
	printf("Got event %d\n", e.type);
	if (e.type == MapNotify) {
	    if (tid == -1) {
		pthread_create(&tid, NULL, play_background, NULL);
	    }
	}
    }

#if 0
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
#endif // gtk stuff
    
    return 0;
}

