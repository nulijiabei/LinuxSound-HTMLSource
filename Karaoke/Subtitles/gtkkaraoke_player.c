#include <fluidsynth.h>
#include <fluid_midi.h>
#include <string.h>

#include <gtk/gtk.h>
#include <gdk/gdkx.h>

/* GString stuff from https://developer.gnome.org/glib/2.31/glib-Strings.html
   Memory alloc from https://developer.gnome.org/glib/2.30/glib-Memory-Allocation.html
   Packing demo from https://developer.gnome.org/gtk-tutorial/2.90/x386.html
   Thread stuff from https://developer.gnome.org/gtk-faq/stable/x481.html
   GArrays from http://www.gtk.org/api/2.6/glib/glib-Arrays.html
   Pango attributes from http://www.ibm.com/developerworks/library/l-u-pango2/
   Timeouts at http://www.gtk.org/tutorial1.2/gtk_tut-17.html
 */

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

fluid_synth_t* synth;

GtkWidget *lyric_labels[2];

fluid_player_t* player;

int current_panel = -1;  // panel showing current lyric line
int current_line = 0;  // which line is the current lyric
gchar *current_lyric;   // currently playing lyric line
GString *front_of_lyric;  // part of lyric to be colouredGString
GString *end_of_lyric;    // part of lyrci to not be coloured

gchar *markup[] = {"<span foreground=\"red\">",
		   "</span><span foreground=\"black\">",
		   "</span>"};
gchar *markup_newline[] = {"<span foreground=\"black\">",
		   "</span>"};
GString *marked_up_label;

struct _reset_label_data {
    GtkLabel *label;
    gchar *text;
    PangoAttrList *attrs;
};

typedef struct _reset_label_data reset_label_data;

/**
 * redraw a label some time later
 */
gint reset_label_cb(gpointer data) {
    reset_label_data *rdata = ( reset_label_data *) data;

    printf("Seting text %s on widget %X\n", rdata->text, rdata->label);
    gtk_label_set_text(rdata->label, rdata->text);
    gtk_label_set_attributes(rdata->label, rdata->attrs);

    GtkAllocation* alloc = g_new(GtkAllocation, 1);
    gtk_widget_get_allocation((GtkWidget *) (rdata->label), alloc);
    //printf("Set label text to \"%s\"\n", gtk_label_get_text(rdata->label));
    printf("Lable has width %d\n", alloc->width);

    return FALSE;
}


/**
 * This MIDI event callback filters out the TEXT and LYRIC events
 * and passes the rest to the default event handler.
 * Here we just print the text of the event, more
 * complex handling can be done
 */
int event_callback(void *data, fluid_midi_event_t *event) {
    fluid_synth_t* synth = (fluid_synth_t*) data;
    int type = fluid_midi_event_get_type(event);
    int chan = fluid_midi_event_get_channel(event);
    if (synth == NULL) printf("Synth is null\n");

    //return 0;

    switch(type) {
    case MIDI_TEXT:
	printf("Callback: Playing text event %s (length %d)\n", event->paramptr, event->param1);

	if (((char *) event->paramptr)[0] == '\\') {
	    int next_panel = current_panel; // really (current_panel+2)%2
	    int next_line = current_line + 2;
	    char *next_lyric;

	    if (current_line + 2 >= lyric_lines.lines->len) {
		return FLUID_OK;
	    }
	    current_line += 1;
	    current_panel = (current_panel + 1) % 2;

	    // set up new line as current line
	    char *lyric =  event->paramptr;
	    current_lyric = g_array_index(lyric_lines.lines, GString *, current_line)->str;
	    front_of_lyric = g_string_new(lyric+1); // lose \
	    end_of_lyric = g_string_new(current_lyric);
	    printf("New line. Setting front to %s end to \"%s\"\n", lyric+1, current_lyric); 

	    // update label for next line after this one
	    char *str = g_array_index(lyric_lines.lines, GString *, next_line)->str;
	    printf("Setting text in label %d to \"%s\"\n", next_panel, str);

	    next_lyric = g_array_index(lyric_lines.lines, GString *, next_line)->str;
	    
	    marked_up_label = g_string_new(markup_newline[0]);
	    //g_string_erase(marked_up_label, strlen(attr_start_markup), -1);
	    g_string_append(marked_up_label, next_lyric);
	    g_string_append(marked_up_label, markup_newline[1]);
	    PangoAttrList *attrs;
	    gchar *text;
	    pango_parse_markup (marked_up_label->str, -1,0, &attrs, &text, NULL, NULL);

	    if (next_panel >= 0) {
		gdk_threads_enter();
		
		reset_label_data *label_data;
		label_data = g_new(reset_label_data, 1);
		label_data->label = (GtkLabel *) lyric_labels[next_panel];
		label_data->text = text;
		label_data->attrs = attrs;
		gtk_timeout_add(1000, reset_label_cb, label_data);
		/*
		//gtk_label_set_text((GtkLabel *) lyric_labels[which_panel], which_lyric);
		gtk_label_set_text((GtkLabel *) lyric_labels[next_panel], text);
		gtk_label_set_attributes(GTK_LABEL(lyric_labels[next_panel]), attrs);
		*/
		// Dies if you try to flush at this point!
		// gdk_flush();
		
		gdk_threads_leave();
	    }
	} else {
	    // change text colour as chars are played
	    char *lyric =  event->paramptr;
	    if ((front_of_lyric != NULL) && (lyric != NULL)) {
		g_string_append(front_of_lyric, lyric);
		char *s = front_of_lyric->str;
		printf("Displaying \"%s\"\n", current_lyric);
		printf("  Colouring \"%s\"\n", s);
		printf("  Not colouring \"%s\"\n", current_lyric + strlen(s));

		// todo: avoid memory leak
		marked_up_label = g_string_new(markup[0]);
		//g_string_erase(marked_up_label, strlen(attr_start_markup), -1);
		g_string_append(marked_up_label, s);
		g_string_append(marked_up_label, markup[1]);
		g_string_append(marked_up_label, current_lyric + strlen(s));
		g_string_append(marked_up_label, markup[2]);
		printf("Marked up label \"%s\"\n", marked_up_label->str);

		/* Example from http://www.ibm.com/developerworks/library/l-u-pango2/
		 */
		PangoAttrList *attrs;
		gchar *text;
		pango_parse_markup (marked_up_label->str, -1,0, &attrs, &text, NULL, NULL);
		
		gdk_threads_enter();
		gtk_label_set_text((GtkLabel *) lyric_labels[current_panel], 
				   text);
		gtk_label_set_attributes(GTK_LABEL(lyric_labels[current_panel]), attrs);
		// Dies if you try to flush at this point!
		//gdk_flush();
		
		gdk_threads_leave();
	    }
	}
	return  FLUID_OK;

    case MIDI_LYRIC:
	printf("Callback: Playing lyric event %d %s\n", event->param1, event->paramptr);
	return  FLUID_OK;
    
    case MIDI_EOT:
	printf("End of track\n");
	exit(0);
    }
    // return FLUID_OK;
    return fluid_synth_handle_midi_event( data, event);
}

void build_lyric_lines() {
    int n;
    lyric_t *plyric;
    GString *line = g_string_new("");
    GArray *lines =  g_array_sized_new(FALSE, FALSE, sizeof(GString *), 64);

    lyric_lines.title = NULL;

    for (n = 0; n < lyrics->len; n++) {
	plyric = g_array_index(lyrics, lyric_t *, n);
	gchar *lyric = plyric->lyric;
	int tick = plyric->tick;

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


/**
 * This is called whenever new data is loaded, such as a new file.
 * Here we extract the TEXT and LYRIC events and just print them
 * to stdout. They could e.g. be saved and displayed in a GUI
 * as the events are received by the event callback.
 */ 
int onload_callback(void *data, fluid_player_t *player) {
    long ticks = 0L;
    lyric_t *plyric;

    printf("Load callback, tracks %d \n", player->ntracks);
    int n;
    for (n = 0; n < player->ntracks; n++) {
	fluid_track_t *track = player->track[n];
	printf("Track %d\n", n);
	fluid_midi_event_t *event = fluid_track_first_event(track);
	while (event != NULL) {
	    switch (fluid_midi_event_get_type (event)) {
	    case MIDI_TEXT:
	    case MIDI_LYRIC:
		/* there's no fluid_midi_event_get_sysex()
		   or fluid_midi_event_get_time() so we 
		   have to look inside the opaque struct
		*/
		ticks += event->dtime;
		printf("Loaded event %s for time %d\n", 
		       event->paramptr,
		       ticks);
		plyric = g_new(lyric_t, 1);
		plyric->lyric = g_strdup(event->paramptr);
		plyric->tick = ticks;
		g_array_append_val(lyrics, plyric);
	    }
	    event = fluid_track_next_event(track);
	}
    }

    printf("Saved %d lyric events\n", lyrics->len);
    for (n = 0; n < lyrics->len; n++) {
	plyric = g_array_index(lyrics, lyric_t *, n);
	printf("Saved lyric %s at %d\n", plyric->lyric, plyric->tick);
    }

    build_lyric_lines();

    gdk_threads_enter();
    char *str = g_array_index(lyric_lines.lines, GString *, 1)->str;
    gtk_label_set_text((GtkLabel *) lyric_labels[0], str);
    str = g_array_index(lyric_lines.lines, GString *, 2)->str;
    gtk_label_set_text((GtkLabel *) lyric_labels[1], str);
    // gdk_flush ();
    
    /* release GTK thread lock */
    gdk_threads_leave();
    
    // marked_up_label = g_string_new(attr_start_markup); 

    return FLUID_OK;
}

/* Called when the windows are realized
 */
static void realize_cb (GtkWidget *widget, gpointer data) {
    /* play the midi files, if any */
    fluid_player_play(player);
}

/* This is a callback function. The data arguments are ignored
 * in this example. More on callbacks below. */
static void hello( GtkWidget *widget,
                   gpointer   data )
{
    g_print ("Hello World\n");
    int n, pitch_bend;
    pitch_bend = 2000;
    for (n = 0; n < 16; n++) {
	if(fluid_synth_pitch_bend (synth,
					   n,
				       pitch_bend) == FLUID_OK) {
	    printf("Pitch bend set ok\n");
	} else {
	    printf("Pitch bend not set ok\n");
	} 
	printf("Pitch bend on channel %d is %d\n", n,
	       fluid_synth_get_pitch_bend (synth,
					   n,
					   &pitch_bend));
    }

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
    fluid_settings_t* settings;

    fluid_audio_driver_t* adriver;
    settings = new_fluid_settings();
    fluid_settings_setstr(settings, "audio.driver", "alsa");
    //fluid_settings_setint(settings, "lash.enable", 0);
    fluid_settings_setint(settings, "synth.polyphony", 64);
    fluid_settings_setint(settings, "synth.reverb.active", FALSE);
    fluid_settings_setint(settings, "synth.sample-rate", 22050);
    synth = new_fluid_synth(settings);
    player = new_fluid_player(synth);

    lyrics = g_array_sized_new(FALSE, FALSE, sizeof(lyric_t *), 1024);

    /* Set the MIDI event callback to our own functions rather than the system default */
    fluid_player_set_playback_callback(player, event_callback, synth);

    /* Add an onload callback so we can get information from new data before it plays */
    fluid_player_set_onload_callback(player, onload_callback, NULL);

    adriver = new_fluid_audio_driver(settings, synth);
    /* process command line arguments */
    for (i = 1; i < argc; i++) {
        if (fluid_is_soundfont(argv[i])) {
	    printf("Loading soundfont %s\n", argv[i]);
	    fluid_synth_sfload(synth, argv[i], 1);
        } else {
            fluid_player_add(player, argv[i]);
        }
    }

    /*
    int n;
    int pitch_bend;
    for (n = 0; n < 16; n++) {
	printf("Pitch bend on channel %d is %d\n", n,
	       fluid_synth_get_pitch_bend (synth,
					   n,
					   &pitch_bend));
    }
    */


   /* GtkWidget is the storage type for widgets */
    GtkWidget *window;
    GtkWidget *button;
    GtkWidget *lyrics_box;

    
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

    lyrics_box = gtk_vbox_new(TRUE, 1);
    //lyrics_box = gtk_table_new(2, 1, TRUE);
    gtk_widget_show(lyrics_box);

    char *str = "     ";
    lyric_labels[0] = gtk_label_new(str);
    str =  "World";
    lyric_labels[1] = gtk_label_new(str);

    gtk_widget_show (lyric_labels[0]);
    gtk_widget_show (lyric_labels[1]);

    
    gtk_box_pack_start (GTK_BOX (lyrics_box), lyric_labels[0], TRUE, TRUE, 0);
    gtk_box_pack_start (GTK_BOX (lyrics_box), lyric_labels[1], TRUE, TRUE, 0);
    /*
    gtk_table_attach_defaults((GtkTable *) lyrics_box,  lyric_labels[0],
			      0, 1, 0, 1);
    gtk_table_attach_defaults((GtkTable *) lyrics_box,  lyric_labels[1],
			      0, 1, 1, 2);
    */

    /* This packs the button into the window (a gtk container). */
    gtk_container_add (GTK_CONTAINER (window), lyrics_box);
        
    /* and the window */
    gtk_widget_show (window);
    
    GdkWindow *gwin;
    
    gwin = gtk_widget_get_window(GTK_WIDGET(lyrics_box));
    printf("the X11 id is %u\n", GDK_DRAWABLE_XID(gwin));

    /* All GTK applications must have a gtk_main(). Control ends here
     * and waits for an event to occur (like a key press or
     * mouse event). */
    gtk_main ();
    
    return 0;

    /*
      GStreamer with GTK using a video window:
      http://gstreamer.freedesktop.org/data/doc/gstreamer/head/gst-plugins-base-libs/html/gst-plugins-base-libs-gstvideooverlay.html

      See for overlay window
      https://gitorious.org/blog-examples/blog-examples/blobs/master/fullscreen_video_with_gst_gtk/fullscreen.c
    */

    /* wait for playback termination */
    fluid_player_join(player);
    /* cleanup */
    delete_fluid_audio_driver(adriver);
    delete_fluid_player(player);
    delete_fluid_synth(synth);
    delete_fluid_settings(settings);
    return 0;
}

