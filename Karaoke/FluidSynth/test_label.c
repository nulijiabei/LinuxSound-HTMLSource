#include <fluidsynth.h>
#include <fluid_midi.h>
#include <string.h>

#include <gtk/gtk.h>

/* GString stuff from https://developer.gnome.org/glib/2.31/glib-Strings.html
   Memory alloc from https://developer.gnome.org/glib/2.30/glib-Memory-Allocation.html
   Packing demo from https://developer.gnome.org/gtk-tutorial/2.90/x386.html
   Thread stuff from https://developer.gnome.org/gtk-faq/stable/x481.html
   GArrays from http://www.gtk.org/api/2.6/glib/glib-Arrays.html
   Pango attributes from http://www.ibm.com/developerworks/library/l-u-pango2/
   Timeouts at http://www.gtk.org/tutorial1.2/gtk_tut-17.html
 */

int MAX = 58;

int line_no = 0;

gchar *lines[] =
{"Here comes\n the sun",
"doo doo doo doo",
"Here comes the sun",
"I said it's alright",
"Little",
"darling",
"It's been a long",
"cold lonely winter",
"Little",
"darling",
"It feels like years",
"since it's been here",
"Here comes",
"the sun",
"Here comes the sun",
"I said it's alright",
"Little",
"darling",
"The smile's returning",
"to their faces",
"Little",
"darling",
"It seems like years",
"since it's been here",
"Here comes",
"the sun",
"Here comes the sun",
"I said it's alright",
"Sun sun sun",
"here it comes",
"Sun sun sun",
"here it comes",
"Sun sun sun",
"here it comes",
"Sun sun sun",
"here it comes",
"Sun sun sun",
"here it comes",
"Little",
"darling",
"I feel that ice",
"is slowly melting",
"Little",
"darling",
"It seems like years",
"since it's been clear",
"Here comes",
"the sun",
"Here comes the sun",
"I said it's alright",
"Here comes",
"the sun",
"Here comes",
"the sun",
"It's",
"alright",
"It's",
 "alright"};


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

GtkWidget *labels[2];
int current_label = 0;

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
 * redraw a lable some time later
 */
gint reset_label_cb(gpointer data) {

    if (line_no < MAX) {
	gtk_label_set_text((GtkLabel *) labels[current_label], lines[line_no++]);
	current_label = (++current_label)%2;
	return TRUE;
    }
    //gtk_label_set_attributes(rdata->label, rdata->attrs);

    printf("Finished\n");
    return FALSE;
}


/* Called when the windows are realized
 */
static void realize_cb (GtkWidget *widget, gpointer data) {

    //gtk_timeout_add(1000, reset_label_cb, NULL);

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
    gtk_widget_show(lyrics_box);

    char *str = "Hello";
    labels[0] = gtk_label_new(str);
    labels[1] = gtk_label_new(str);

    gtk_widget_show (labels[0]);
    gtk_widget_show (labels[1]);

    gtk_box_pack_start (GTK_BOX (lyrics_box), labels[0], TRUE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (lyrics_box), labels[1], TRUE, FALSE, 0);

    

    /* This packs the button into the window (a gtk container). */
    gtk_container_add (GTK_CONTAINER (window), lyrics_box);
    

    
    /* and the window */
    gtk_widget_show (window);
    
    /* All GTK applications must have a gtk_main(). Control ends here
     * and waits for an event to occur (like a key press or
     * mouse event). */
    gtk_main ();
    
    return 0;

}

