

#include <gtk/gtk.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

GtkWidget *window, *text;
GtkTextBuffer *textbuf = NULL;

GtkTextIter iter, start_iter, end_iter;

GIOChannel *channel_in, 
           *channel_out;

int pipeAppli[2],pipeGtk[2];

int fpip_in, fpip_out;	/* in and out depends in which process we are */
int pid;

int str_ptr = 0;

static void

pipe_error(char *st)
{
    fprintf(stderr,"CONNECTION PROBLEM WITH Gtk+ PROCESS IN %s BECAUSE:%s\n",
	    st,
	    strerror(errno));
    exit(1);
}

void
gtk_pipe_string_read(char *str)
{
    gsize len;
    int slen = 1;
    GError *error = NULL;
    
    g_io_channel_read_chars(channel_in, str, slen,
			   &len, &error); 
    if (len!=slen) pipe_error("CHANNEL_STRING_READ on string part");
    str[slen]='\0';		/* Append a terminal 0 */
    fprintf(stderr, "Reading gtk str %s\n", str);
}

gboolean addtext(GIOChannel *channel, GIOCondition cond, 
		 gpointer client_data) {
    gchar *message_u8;
    char message[1000];    

    gtk_pipe_string_read(message);

    message_u8 = g_locale_to_utf8( message, -1, NULL, NULL, NULL );

    gtk_text_buffer_get_bounds(textbuf, &start_iter, &end_iter);
    // mod JN iter -> end_iter
    gtk_text_buffer_insert(textbuf, &end_iter,
			   message_u8, -1);

    return TRUE;
}


void start_gtk() {
    gtk_init (0, NULL);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(window), 250, 180);
    gtk_widget_set_name(window, "TiMidity");
    text = gtk_text_view_new();
    textbuf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text));

    gtk_widget_show(text);
    gtk_container_add(GTK_CONTAINER(window), text);

    gtk_widget_realize(window);

    gtk_widget_show_all(window);

    //g_timeout_add(100, addtext, NULL);
    g_io_add_watch(channel_in, G_IO_IN, addtext, NULL);
//addtext();

    gtk_main();

}


int main(int argc, char **argv) {
    int res;
    GError *error = NULL;

     res=pipe(pipeGtk);
    if (res!=0) pipe_error("PIPE_GTK CREATION");
    
    if ((pid=fork())==0) {   /*child*/
	close(pipeGtk[1]); 
	    
	fpip_in=pipeGtk[0];

	channel_in =  g_io_channel_unix_new(fpip_in);

	/* set to raw I/O, unbuffered */
	g_io_channel_set_encoding(channel_in, NULL, &error);
	g_io_channel_set_buffered(channel_in, FALSE);

	start_gtk();
    } else {

    
	close(pipeGtk[0]);
	
	fpip_out= pipeGtk[1];
	
	int ch = 'a';
	
	while (1) {
	    write(fpip_out, &ch, 1);
	    ch = 'a' + (ch -'a' + 1) % 26;
	    sleep(1);
	}
	
	write(fpip_out, "hello", strlen("hello"));
	sleep(1000);
    }
}
