

#include <gtk/gtk.h>

GtkWidget *window, *text;
GtkTextBuffer *textbuf = NULL;

GtkTextIter iter, start_iter, end_iter;

int str_ptr = 0;

gboolean addtext(void *arg) {
    char *str = "some text\nand some more\n";
    gchar *message_u8;
    char message[2];    

    message[0] = str[str_ptr];
    message[1] = '\0';
    str_ptr = (str_ptr + 1) % strlen(str);

    message_u8 = g_locale_to_utf8( message, -1, NULL, NULL, NULL );

    gtk_text_buffer_get_bounds(textbuf, &start_iter, &end_iter);
    // mod JN iter -> end_iter
    gtk_text_buffer_insert(textbuf, &end_iter,
			   message_u8, -1);

    return TRUE;
}

int main(int argc, char **argv) {

    gtk_init (&argc, &argv);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(window), 250, 180);
    gtk_widget_set_name(window, "TiMidity");
    text = gtk_text_view_new();
    textbuf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text));

    gtk_widget_show(text);
    gtk_container_add(GTK_CONTAINER(window), text);

    gtk_widget_realize(window);

    gtk_widget_show_all(window);

    g_timeout_add(100, addtext, NULL);
//addtext();

    gtk_main();

}
