/*
  TiMidity++ -- MIDI to WAVE converter and player
  Copyright (C) 1999-2002 Masanao Izumo <mo@goice.co.jp>
  Copyright (C) 1995 Tuukka Toivonen <tt@cgs.fi>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

  gtk_i.c - Glenn Trigg 29 Oct 1998

*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H*/

#include <string.h>
#ifdef HAVE_GLOB_H
#include <glob.h>
#endif
#if HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
#include <gtk/gtk.h>
#include <stdlib.h>

#include "timidity.h"
#include "common.h"
#include "output.h"
#include "gtk_h.h"

#include "pixmaps/playpaus.xpm"
#include "pixmaps/prevtrk.xpm"
#include "pixmaps/nexttrk.xpm"
#include "pixmaps/rew.xpm"
#include "pixmaps/ff.xpm"
#include "pixmaps/restart.xpm"
#include "pixmaps/quit.xpm"
#include "pixmaps/quiet.xpm"
#include "pixmaps/loud.xpm"
#include "pixmaps/open.xpm"
#include "pixmaps/keyup.xpm"
#include "pixmaps/keydown.xpm"
#include "pixmaps/slow.xpm"
#include "pixmaps/fast.xpm"
#include "pixmaps/timidity.xpm"

static GtkWidget *create_menubar(void);
static GtkWidget *create_button_with_pixmap(GtkWidget *, const char **, gint, gchar *);
static GtkWidget *create_pixmap_label(GtkWidget *, const char **);
static gint delete_event(GtkWidget *, GdkEvent *, gpointer);
void destroy (GtkWidget *, gpointer);

#if  GTK_MAJOR_VERSION == 2
static GtkTooltips *create_yellow_tooltips(void);
static void handle_input(gpointer, gint, GdkInputCondition);
#else
static gboolean handle_input(GIOChannel *, GIOCondition, gpointer);
#endif

void generic_cb(GtkWidget *, gpointer);
void generic_scale_cb(GtkAdjustment *, gpointer);
void open_file_cb(GtkWidget *, gpointer);
void playlist_cb(GtkWidget *, guint);
void playlist_op(GtkWidget *, guint);

#if GTK_MAJOR_VERSION == 2
void file_list_cb(GtkWidget *, gint, gint, GdkEventButton *, gpointer);
#else
static void file_list_cb (GtkTreeSelection *, gpointer);
static void toggle_auto_cb (GtkWidget *, gpointer);
#endif

void clear_all_cb(GtkWidget *, gpointer);
void filer_cb(GtkWidget *, gpointer);
void tt_toggle_cb(GtkWidget *, gpointer);
void locate_update_cb(GtkWidget *, GdkEventButton *, gpointer);
void my_adjustment_set_value(GtkAdjustment *, gint);
#if GTK_MAJOR_VERSION == 2
void set_icon_pixmap(GtkWidget *, gchar **);
#else
GtkListStore *list_store;
#endif

static GtkWidget *window, *clist, *text, *vol_scale, *locator;
static GtkWidget *filesel = NULL, *plfilesel = NULL;
static GtkWidget *tot_lbl, *cnt_lbl, *ttshow;

#if  GTK_MAJOR_VERSION == 2
static GtkTooltips *ttip;
static GtkWidget *auto_next;
#endif

#if GTK_MAJOR_VERSION == 3
#define GTK_SIGNAL_FUNC(x) (x)
#define GTK_OBJECT(x) G_CALLBACK(x)
#define gtk_signal_connect(a,b,c,d) g_signal_connect((a), (b), G_CALLBACK(c), (d))
static int auto_next = TRUE;
#endif

static int file_number_to_play; /* Number of the file we're playing in the list */
static int max_sec, is_quitting = 0, locating = 0, local_adjust = 0;

#if  GTK_MAJOR_VERSION == 2
static GtkItemFactoryEntry ife[] = {
    {"/File/Open", "<control>O", open_file_cb, 0, NULL},
    {"/File/sep", NULL, NULL, 0, "<Separator>"},
    {"/File/Load Playlist", "<control>L", playlist_cb,
     'l', NULL},
    {"/File/Save Playlist", "<control>S", playlist_cb,
     's', NULL},
    {"/File/sep", NULL, NULL, 0, "<Separator>"},
    {"/File/Quit", "<control>Q", generic_cb, GTK_QUIT, NULL},
    {"/Options/Auto next", "<control>A", NULL, 0, "<CheckItem>"},
    {"/Options/Show tooltips", "<control>T", tt_toggle_cb, 0, "<CheckItem>"},
    {"/Options/Clear All", "<control>C", clear_all_cb, 0, NULL}
};
#else // GTK 3.0
/* See http://developer.ubuntu.com/resources/technologies/application-indicators/
   and http://www.cs.hunter.cuny.edu/~sweiss/course_materials/csci493.70/lecture_notes/GTK_menus.pdf
 */
static GtkActionEntry entries[] = {
    { "FileMenu", NULL, "_File" },
    { "Open",     "document-open", "_Open", "<control>O",
      "Open a file", G_CALLBACK (open_file_cb) },
    { "Open Playlist",     "document-open", "_Playlist", "<control>P",
      "Open a playlist", G_CALLBACK (playlist_cb) },
    { "Save Playlist",     "document-save", "_Save", "<control>S",
      "Save a playlist", G_CALLBACK (playlist_cb) },
    { "Quit",     "application-exit", "_Quit", "<control>Q",
      "Exit the application", G_CALLBACK (gtk_main_quit) },

    { "OptionsMenu", NULL, "_Options" },

    { "Clear All",     "document-open", "_Clear", "<control>C",
      "Clear all", G_CALLBACK (playlist_cb)},
};
static guint n_entries = G_N_ELEMENTS (entries);

static const GtkToggleActionEntry toggle_entries[] = {
     { "Auto next",     "document-open", "_Auto", "<control>A",
       "Auto play next song", G_CALLBACK (toggle_auto_cb), TRUE }
};
static guint n_toggle_entries = G_N_ELEMENTS (toggle_entries);

static const gchar *ui_info =
    "<ui>"
    "  <menubar name='MenuBar'>"
    "    <menu action='FileMenu'>"
    "      <menuitem action='Open'/>"
    "      <menuitem action='Open Playlist'/>"
    "      <menuitem action='Save Playlist'/>"
    "      <separator/>"
    "      <menuitem action='Quit'/>"
    "    </menu>"
    "    <menu action='OptionsMenu'>"
    "      <menuitem action='Auto next'/>"
    "      <menuitem action='Clear All'/>"
    "      <separator/>"
    "      <menuitem action='Quit'/>"
    "    </menu>"
    "  </menubar>"
    "</ui>";
#endif

static GtkTextBuffer *textbuf = NULL;
static GtkTextIter start_iter, end_iter;
static GtkTreeIter select_iter;
static GtkTextMark *mark;

/*----------------------------------------------------------------------*/

void
generic_cb(GtkWidget *widget, gpointer data)
{
    gtk_pipe_int_write(GPOINTER_TO_INT(data));
    if(GPOINTER_TO_INT(data) == GTK_PAUSE) {
	gtk_label_set_text(GTK_LABEL(cnt_lbl), "Pause");
    }
}

void
tt_toggle_cb(GtkWidget *widget, gpointer data)
{
#if  GTK_MAJOR_VERSION == 2
    if( GTK_CHECK_MENU_ITEM(ttshow)->active ) {
	gtk_tooltips_enable(ttip);
    }
    else {
	gtk_tooltips_disable(ttip);
    }
#elif  GTK_MAJOR_VERSION == 3
    if(gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(ttshow)) ) {
	//gtk_tooltips_enable(ttip);
    }
    else {
	//gtk_tooltips_disable(ttip);
    }
#endif
}


void
open_file_cb(GtkWidget *widget, gpointer data)
{
#if  GTK_MAJOR_VERSION == 2
    if( ! filesel ) {
	filesel = gtk_file_selection_new("Open File");
	gtk_file_selection_hide_fileop_buttons(GTK_FILE_SELECTION(filesel));

	gtk_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(filesel)->ok_button),
			   "clicked",
			   G_CALLBACK (filer_cb), GINT_TO_POINTER(1));

	gtk_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(filesel)->cancel_button),
			   "clicked",
			   G_CALLBACK (filer_cb), GINT_TO_POINTER(0));
    }
    gtk_widget_show(GTK_WIDGET(filesel));    
#elif GTK_MAJOR_VERSION == 3
    // From http://www.gnu.org/software/guile-gnome/docs/gtk/html/GtkFileChooserDialog.html
    if( ! filesel ) {
	filesel = gtk_file_chooser_dialog_new("Open file",
					      GTK_WINDOW(window),
					      GTK_FILE_CHOOSER_ACTION_OPEN,
					      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					      GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
					      NULL);
	if (gtk_dialog_run (GTK_DIALOG (filesel)) == GTK_RESPONSE_ACCEPT) {
	    char *filename;
	    gchar *filenames[2];
	    
	    filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (filesel));
	    filenames[0] = filename;
	    printf("Got file %s\n", filename);
	    filenames[1] = NULL;

	    //GtkTreeIter   iter;
	    gtk_list_store_append (list_store, &select_iter);  /* Acquire an iterator */
	    gtk_list_store_set (list_store, &select_iter, 0, filename, -1);
	}
	gtk_widget_destroy (filesel);
	filesel = NULL;
    }
#endif

}

void
filer_cb(GtkWidget *widget, gpointer data)
{
    gchar *filenames[2];
#ifdef GLOB_BRACE
    int i;
    const gchar *patt;
    glob_t pglob;

    if(GPOINTER_TO_INT(data) == 1) {
	patt = gtk_file_selection_get_filename(GTK_FILE_SELECTION(filesel));
	if(glob(patt, GLOB_BRACE|GLOB_NOMAGIC|GLOB_TILDE, NULL, &pglob))
	    return;
	for( i = 0; i < pglob.gl_pathc; i++) {
	    filenames[0] = pglob.gl_pathv[i];
	    filenames[1] = NULL;
#if GTK_MAJOR_VERSION == 2
	    gtk_clist_append(GTK_CLIST(clist), filenames);
#else
	    GtkTreeIter iter;
	    gtk_list_store_append((GtkListStore *) list_store, *iter);
	    gtk_list_store_set ((GtkListStore *) list_store, 
				iter, 0, filenames[0], -1);
#endif
	}
	globfree(&pglob);
    }
#else
    if((int)data == 1) {
	filenames[0] = gtk_file_selection_get_filename(GTK_FILE_SELECTION(filesel));
	filenames[1] = NULL;
#if GTK_MAJOR_VERSION == 2
	gtk_clist_append(GTK_CLIST(clist), filenames);
#else
	GtkTreeIter   iter;
	gtk_list_store_append (list_store, &iter);  /* Acquire an iterator */
	gtk_list_store_set (list_store, &iter, 0, filenames[0], -1);
#endif
    }
#endif
    gtk_widget_hide(filesel);
#if GTK_MAJOR_VERSION == 2
    gtk_clist_columns_autosize(GTK_CLIST(clist));
#else
    // No replacement?
#endif
}

void
generic_scale_cb(GtkAdjustment *adj, gpointer data)
{
    if(local_adjust)
	return;

    gtk_pipe_int_write(GPOINTER_TO_INT(data));

    /* This is a bit of a hack as the volume scale (a GtkVScale) seems
       to have it's minimum at the top which is counter-intuitive. */
    if(GPOINTER_TO_INT(data) == GTK_CHANGE_VOLUME) {
	gtk_pipe_int_write(MAX_AMPLIFICATION - 
			   gtk_adjustment_get_value(adj));
    }
    else {
	gtk_pipe_int_write((int) gtk_adjustment_get_value(adj)*100);
    }
}

#if GTK_MAJOR_VERSION == 2
void
file_list_cb(GtkWidget *widget, gint row, gint column,
	     GdkEventButton *event, gpointer data)
{
    gint	retval;
    gchar	*fname;

    if(event && (event->button == 3)) {
	if(event->type == GDK_2BUTTON_PRESS) {
	    gtk_clist_remove(GTK_CLIST(clist), row);
	}
	else {
	    return;
	}
    }
    retval = gtk_clist_get_text(GTK_CLIST(widget), row, 0, &fname);
    if(retval) {
	gtk_pipe_int_write(GTK_PLAY_FILE);
	gtk_pipe_string_write(fname);
	file_number_to_play=row;
    }
}
#else // GTK 3
static void
file_list_cb (GtkTreeSelection *selection, gpointer data)
{
    GtkTreeIter iter;
    GtkTreeModel *model;
    gchar *fname;
    
    if (gtk_tree_selection_get_selected (selection, &model, &iter)) {
	gtk_tree_model_get (model, &iter, 0, &fname, -1);
	
	g_print ("You selected a book by %s\n", fname);
	
	gtk_pipe_int_write(GTK_PLAY_FILE);
	gtk_pipe_string_write(fname);
#if GTK_MAJOR_VERSION == 2
	file_number_to_play=1;
#endif
    }
}
#endif

void
playlist_cb(GtkWidget *widget, guint data)
{
    const gchar	*pldir;
    gchar	*plpatt;

    if( ! plfilesel ) {
	plfilesel = gtk_file_selection_new("");
	gtk_file_selection_hide_fileop_buttons(GTK_FILE_SELECTION(plfilesel));

	pldir = g_getenv("TIMIDITY_PLAYLIST_DIR");
	if(pldir != NULL) {
	    plpatt = g_strconcat(pldir, "/*.tpl", NULL);
	    gtk_file_selection_set_filename(GTK_FILE_SELECTION(plfilesel),
					    plpatt);
	    g_free(plpatt);
	}

#if GTK_MAJOR_VERSION == 2
	gtk_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(plfilesel)->ok_button),
			   "clicked",
			   GTK_SIGNAL_FUNC (playlist_op), (gpointer)1);
	gtk_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(plfilesel)->cancel_button),
			   "clicked",
			   GTK_SIGNAL_FUNC (playlist_op), NULL);
#elif GTK_MAJOR_VERSION == 3
	// FIXME needs  gtk_file_chooser_dialog_new
#endif
    }

    gtk_window_set_title(GTK_WINDOW(plfilesel), ((char)data == 'l')?
			 "Load Playlist":
			 "Save Playlist");
    gtk_object_set_user_data(GTK_OBJECT(plfilesel), GINT_TO_POINTER(data));
    gtk_file_selection_complete(GTK_FILE_SELECTION(plfilesel), "*.tpl");

    gtk_widget_show(plfilesel);
} /* playlist_cb */

void
playlist_op(GtkWidget *widget, guint data)
{
    int		i;
    const gchar	*filename[2];
    gchar	action, *rowdata, fname[BUFSIZ], *tmp;
    FILE	*plfp;

    gtk_widget_hide(plfilesel);

    if(!data)
	return;

    action = GPOINTER_TO_INT(gtk_object_get_user_data(GTK_OBJECT(plfilesel)));
    filename[0] = gtk_file_selection_get_filename(GTK_FILE_SELECTION(plfilesel));

    if(action == 'l') {
	if((plfp = fopen(filename[0], "r")) == NULL) {
	    g_error("Can't open %s for reading.", filename[0]);
	    return;
	}
	while(fgets(fname, BUFSIZ, plfp) != NULL) {
            gchar *filename[2];
	    if(fname[strlen(fname) - 1] == '\n')
		fname[strlen(fname) - 1] = '\0';
	    filename[0] = fname;
	    filename[1] = NULL;
#if GTK_MAJOR_VERSION == 2
	    gtk_clist_append(GTK_CLIST(clist), filename);
#else
	    // FIXME
#endif
	}
	fclose(plfp);
#if GTK_MAJOR_VERSION == 2
	gtk_clist_columns_autosize(GTK_CLIST(clist));
#else
	// FIXME
#endif
    }
    else if(action == 's') {
	if((plfp = fopen(filename[0], "w")) == NULL) {
	    g_error("Can't open %s for writing.", filename[0]);
	    return;
	}
#if GTK_MAJOR_VERSION == 2
	for(i = 0; i < GTK_CLIST(clist)->rows; i++) {
	    gtk_clist_get_text(GTK_CLIST(clist), i, 0, &rowdata);
	    /* Make sure we have an absolute path. */
	    if(*rowdata != '/') {
		tmp = g_get_current_dir();
		rowdata = g_strconcat(tmp, "/", rowdata, NULL);
		fprintf(plfp, "%s\n", rowdata);
		g_free(rowdata);
		g_free(tmp);
	    }
	    else {
		fprintf(plfp, "%s\n", rowdata);
	    }
	}
#elif GTK_MAJOR_VERSION == 3
	// FIXME needs  gtk_file_chooser_dialog_new
#endif
	fclose(plfp);
    }
    else {
	g_error("Invalid playlist action!.");
    }
} /* playlist_op */

void
clear_all_cb(GtkWidget *widget, gpointer data)
{
#if GTK_MAJOR_VERSION == 2
    gtk_clist_clear(GTK_CLIST(clist));
#else
    gtk_list_store_clear(list_store);
    //select_iter = NULL;
#endif
} /* clear_all_cb */

gint
delete_event(GtkWidget *widget, GdkEvent *event, gpointer data)
{
    return (FALSE);
}

void
destroy (GtkWidget *widget, gpointer data)
{
    is_quitting = 1;
    gtk_pipe_int_write(GTK_QUIT);
}

void
locate_update_cb (GtkWidget *widget, GdkEventButton *ev, gpointer data)
{
    if( (ev->button == 1) || (ev->button == 2)) {
	if( ev->type == GDK_BUTTON_RELEASE ) {
	    locating = 0;
	}
	else {
	    locating = 1;
	}
    }
}

void
my_adjustment_set_value(GtkAdjustment *adj, gint value)
{
    local_adjust = 1;
    gtk_adjustment_set_value(adj, (gfloat)value);
    local_adjust = 0;
}

#if GTK_MAJOR_VERSION == 2
void
Launch_Gtk_Process(int pipe_number)
#else
    void
    Launch_Gtk_Process(GIOChannel *channel)
#endif
{
    int	argc = 0;
    gchar **argv = NULL;
    GtkWidget *button1, *button2, *button3, *button4, *button5,
	*button6, *button7, *button8, *button9, *button10,
	*button11, *button12, *swin;
    GtkWidget *table, *align, *handlebox;
    GtkWidget *vbox, *hbox, *vbox2, *scrolled_win;
#if GTK_MAJOR_VERSION == 2
    GtkObject *adj;
#else
    GtkAdjustment *adj;
#endif
    GtkWidget *mbar;

    /* enable locale */
#if GTK_MAJOR_VERSION == 2
    gtk_set_locale ();
#else
    /* no need for setlocale() */
#endif

    gtk_init (&argc, &argv);

    // FIXME ttip = create_yellow_tooltips();
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_widget_set_name(window, "TiMidity");
    gtk_window_set_title(GTK_WINDOW(window), "TiMidity - MIDI Player");
    gtk_window_set_wmclass(GTK_WINDOW(window), "timidity", "TiMidity");

    gtk_signal_connect(GTK_OBJECT(window), "delete_event",
		       GTK_SIGNAL_FUNC (delete_event), NULL);

    gtk_signal_connect(GTK_OBJECT(window), "destroy",
		       GTK_SIGNAL_FUNC (destroy), NULL);

#if GTK_MAJOR_VERSION == 2
    vbox = gtk_vbox_new(FALSE, 0);
#else
    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
#endif
    gtk_widget_set_size_request(vbox, 600, 600);

    gtk_container_add(GTK_CONTAINER(window), vbox);

    mbar = create_menubar();
    gtk_box_pack_start(GTK_BOX(vbox), mbar, FALSE, FALSE, 0);

    scrolled_win = gtk_scrolled_window_new(NULL, NULL);
    gtk_box_pack_start(GTK_BOX(vbox), scrolled_win, TRUE, TRUE ,0);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_win),GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);
    gtk_widget_show(scrolled_win);

    text = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text), GTK_WRAP_WORD);
    gtk_widget_show(text);
    gtk_container_add(GTK_CONTAINER(scrolled_win), text);

#if GTK_MAJOR_VERSION == 2
    hbox = gtk_hbox_new(FALSE, 4);
#else
    hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
#endif
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 4);
    gtk_widget_show(hbox);

    adj = gtk_adjustment_new(0., 0., 100., 1., 20., 0.);

#if GTK_MAJOR_VERSION == 2
    locator = gtk_hscale_new(GTK_ADJUSTMENT(adj));
#else
    locator = gtk_scale_new(GTK_ORIENTATION_HORIZONTAL, GTK_ADJUSTMENT(adj));
#endif
    gtk_scale_set_draw_value(GTK_SCALE(locator), TRUE);
    gtk_signal_connect(GTK_OBJECT(adj), "value_changed",
		       GTK_SIGNAL_FUNC(generic_scale_cb),
		       (gpointer)GTK_CHANGE_LOCATOR);
    gtk_signal_connect(GTK_OBJECT(locator), "button_press_event",
		       GTK_SIGNAL_FUNC(locate_update_cb),
		       NULL);
    gtk_signal_connect(GTK_OBJECT(locator), "button_release_event",
		       GTK_SIGNAL_FUNC(locate_update_cb),
		       NULL);
#if GTK_MAJOR_VERSION == 2 
    gtk_range_set_update_policy(GTK_RANGE(locator),
				GTK_UPDATE_DISCONTINUOUS);
#else
    // FIXME
#endif
    gtk_scale_set_digits(GTK_SCALE(locator), 0);
    gtk_widget_show(locator);
    gtk_box_pack_start(GTK_BOX(hbox), locator, TRUE, TRUE, 4);

    align = gtk_alignment_new(0., 1., 1., 0.);
    gtk_widget_show(align);
    cnt_lbl = gtk_label_new("00:00");
    gtk_widget_show(cnt_lbl);
    gtk_container_add(GTK_CONTAINER(align), cnt_lbl);
    gtk_box_pack_start(GTK_BOX(hbox), align, FALSE, TRUE, 0);

    align = gtk_alignment_new(0., 1., 1., 0.);
    gtk_widget_show(align);
    tot_lbl = gtk_label_new("/00:00");
    gtk_widget_show(tot_lbl);
    gtk_container_add(GTK_CONTAINER(align), tot_lbl);
    gtk_box_pack_start(GTK_BOX(hbox), align, FALSE, TRUE, 0);

#if GTK_MAJOR_VERSION == 2
    hbox = gtk_hbox_new(FALSE, 4);
#else
    hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
#endif
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 4);

#if GTK_MAJOR_VERSION == 2
    swin = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(swin),
				   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    clist = gtk_clist_new(1);
    gtk_container_add(GTK_CONTAINER(swin), clist);
    gtk_widget_show(swin);
    gtk_widget_show(clist);
    gtk_widget_set_usize(clist, 200, 10);
    gtk_clist_set_reorderable(GTK_CLIST(clist), TRUE);
    gtk_clist_set_button_actions(GTK_CLIST(clist), 0, GTK_BUTTON_SELECTS);
    gtk_clist_set_button_actions(GTK_CLIST(clist), 1, GTK_BUTTON_DRAGS);
    gtk_clist_set_button_actions(GTK_CLIST(clist), 2, GTK_BUTTON_SELECTS);
    gtk_clist_set_selection_mode(GTK_CLIST(clist), GTK_SELECTION_SINGLE);
    gtk_clist_set_column_auto_resize(GTK_CLIST(clist), 1, TRUE);
    gtk_signal_connect(GTK_OBJECT(clist), "select_row",
		       GTK_SIGNAL_FUNC(file_list_cb), NULL);

    gtk_box_pack_start(GTK_BOX(hbox), swin, TRUE, TRUE, 0);
#elif GTK_MAJOR_VERSION == 3
    // See https://developer.gnome.org/gtk3/3.0/TreeWidget.html
    swin = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(swin),
				   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    GtkTreeViewColumn *column;
    GtkCellRenderer *renderer;

    list_store = gtk_list_store_new (1, G_TYPE_STRING);


    clist = gtk_tree_view_new_with_model (GTK_TREE_MODEL (list_store));

    renderer = gtk_cell_renderer_text_new ();
    g_object_set (G_OBJECT (renderer),
		  "foreground", "red",
		  NULL);

    /* Create a column, associating the "text" attribute of the
     * cell_renderer to the first column of the model */
    column = gtk_tree_view_column_new_with_attributes ("Files", renderer,
						       "text", 0,
						       NULL);

    /* Add the column to the view. */
    gtk_tree_view_append_column (GTK_TREE_VIEW (clist), column);

    gtk_container_add(GTK_CONTAINER(swin), clist);
    gtk_widget_show(swin);
    gtk_widget_show(clist);
    gtk_widget_set_size_request(clist, 600, 600);
    gtk_box_pack_start(GTK_BOX(hbox), swin, TRUE, TRUE, 0);

    /* Add selection handler */
    GtkTreeSelection *select;
   
    select = gtk_tree_view_get_selection (GTK_TREE_VIEW (clist));
    gtk_tree_selection_set_mode (select, GTK_SELECTION_SINGLE);
    gtk_signal_connect(GTK_OBJECT(select), "changed",
		       G_CALLBACK(file_list_cb), NULL);

#endif

#if GTK_MAJOR_VERSION == 2
    vbox2 = gtk_vbox_new(FALSE, 0);
#else
    vbox2 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
#endif
    gtk_box_pack_start(GTK_BOX(hbox), vbox2, FALSE, FALSE, 0);
    gtk_widget_show(vbox2);

    /* This is so the pixmap creation works properly. */
    gtk_widget_realize(window);
#if GKT_MAJOR_VERSION == 2
    set_icon_pixmap(window, timidity_xpm);
#endif

    gtk_box_pack_start(GTK_BOX(vbox2),
		       create_pixmap_label(window, loud_xpm),
		       FALSE, FALSE, 0);

    adj = gtk_adjustment_new(30., 0., (gfloat)MAX_AMPLIFICATION, 1., 20., 0.);
#if GTK_MAJOR_VERSION == 2
    vol_scale = gtk_vscale_new(GTK_ADJUSTMENT(adj));
#else
    vol_scale = gtk_scale_new(GTK_ORIENTATION_VERTICAL, GTK_ADJUSTMENT(adj));
#endif
    gtk_scale_set_draw_value(GTK_SCALE(vol_scale), FALSE);
    gtk_signal_connect (GTK_OBJECT(adj), "value_changed",
			GTK_SIGNAL_FUNC(generic_scale_cb),
			(gpointer)GTK_CHANGE_VOLUME);
#if  GTK_MAJOR_VERSION == 2
    gtk_range_set_update_policy(GTK_RANGE(vol_scale),
				GTK_UPDATE_DELAYED);
#endif

    gtk_widget_show(vol_scale);

#if  GTK_MAJOR_VERSION == 2
    gtk_tooltips_set_tip(ttip, vol_scale, "Volume control", NULL);
#else
    // FIXME
#endif

    gtk_box_pack_start(GTK_BOX(vbox2), vol_scale, TRUE, TRUE, 0);

    gtk_box_pack_start(GTK_BOX(vbox2),
		       create_pixmap_label(window, quiet_xpm),
		       FALSE, FALSE, 0);

    handlebox = gtk_handle_box_new();
    gtk_box_pack_start(GTK_BOX(hbox), handlebox, FALSE, FALSE, 0);

#if GTK_MAJOR_VERSION == 2
    table = gtk_table_new(7, 2, TRUE);
#else
    table = gtk_grid_new();
#endif

    gtk_container_add(GTK_CONTAINER(handlebox), table);

    button1 = create_button_with_pixmap(window, playpaus_xpm, GTK_PAUSE,
				       "Play/Pause");

    button2 = create_button_with_pixmap(window, prevtrk_xpm, GTK_PREV,
				       "Previous file");

    button3 = create_button_with_pixmap(window, nexttrk_xpm, GTK_NEXT,
				       "Next file");
 
    button4 = create_button_with_pixmap(window, rew_xpm, GTK_RWD,
				       "Rewind");

    button5 = create_button_with_pixmap(window, ff_xpm, GTK_FWD,
				       "Fast forward");

    button6 = create_button_with_pixmap(window, keydown_xpm, GTK_KEYDOWN,
				       "Lower pitch");
 
    button7 = create_button_with_pixmap(window, keyup_xpm, GTK_KEYUP,
				       "Raise pitch");

    button8 = create_button_with_pixmap(window, slow_xpm, GTK_SLOWER,
				       "Decrease tempo");

    button9 = create_button_with_pixmap(window, fast_xpm, GTK_FASTER,
				       "Increase tempo");

    button10 = create_button_with_pixmap(window, restart_xpm, GTK_RESTART,
				       "Restart");

    button11 = create_button_with_pixmap(window, open_xpm, 0,
				       "Open");
#if GTK_MAJOR_VERSION == 2
    gtk_signal_disconnect_by_func(GTK_OBJECT(button11), G_CALLBACK(generic_cb), 0);
#else
    g_signal_handlers_disconnect_by_func(GTK_OBJECT(button11), G_CALLBACK(generic_cb), 0);
#endif
    gtk_signal_connect(GTK_OBJECT(button11), "clicked",
		       GTK_SIGNAL_FUNC(open_file_cb), 0);

    button12 = create_button_with_pixmap(window, quit_xpm, GTK_QUIT,
				       "Quit");

#if  GTK_MAJOR_VERSION == 2
    gtk_table_attach_defaults(GTK_TABLE(table), button1,
			      0, 2, 0, 1);
    gtk_table_attach_defaults(GTK_TABLE(table), button2,
			      0, 1, 1, 2);
   gtk_table_attach_defaults(GTK_TABLE(table), button3,
			      1, 2, 1, 2);
    gtk_table_attach_defaults(GTK_TABLE(table), button4,
			      0, 1, 2, 3);
    gtk_table_attach_defaults(GTK_TABLE(table), button5,
			      1, 2, 2, 3);
   gtk_table_attach_defaults(GTK_TABLE(table), button6,
			      0, 1, 3, 4);
    gtk_table_attach_defaults(GTK_TABLE(table), button7,
			      1, 2, 3, 4);
    gtk_table_attach_defaults(GTK_TABLE(table), button8,
			      0, 1, 4, 5);
    gtk_table_attach_defaults(GTK_TABLE(table), button9,
			      1, 2, 4, 5);
    gtk_table_attach_defaults(GTK_TABLE(table), button10,
			      0, 1, 5, 6);
    gtk_table_attach_defaults(GTK_TABLE(table), button11,
			      1, 2, 5, 6);
    gtk_table_attach_defaults(GTK_TABLE(table), button12,
			      0, 2, 6, 7);
#else
    gtk_grid_attach(GTK_GRID(table), button1,
		    0, 0, 2, 1);
    gtk_grid_attach(GTK_GRID(table), button2,
		    0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(table), button3,
		    1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(table), button4,
		    0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(table), button5,
		    1, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(table), button6,
		    0, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(table), button7,
		    1, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(table), button8,
		    0, 4, 1, 1);
    gtk_grid_attach(GTK_GRID(table), button9,
		    1, 4, 1, 1);
    gtk_grid_attach(GTK_GRID(table), button10,
		    0, 5, 1, 1);
    gtk_grid_attach(GTK_GRID(table), button11,
		    1, 5, 1, 1);
    gtk_grid_attach(GTK_GRID(table), button12,
		    0, 6, 2, 1);
#endif

    gtk_widget_show(hbox);
    gtk_widget_show(vbox);
    gtk_widget_show(table);
    gtk_widget_show(handlebox);
    gtk_widget_show(window);

#if  GTK_MAJOR_VERSION == 2
    gdk_input_add(pipe_number, GDK_INPUT_READ, handle_input, NULL);
#else
    g_io_add_watch(channel, G_IO_IN, handle_input, NULL);
#endif
    gtk_main();
}


GtkWidget *
create_button_with_pixmap(GtkWidget *window, const char **bits, gint data, gchar *thelp)
{
#if  GTK_MAJOR_VERSION == 2
    GtkWidget	*pw, *button;
    GdkPixmap	*pixmap;
    GdkBitmap	*mask;
    GtkStyle	*style;

    style = gtk_widget_get_style(window);
    pixmap = gdk_pixmap_create_from_xpm_d(window->window,
					  &mask,
					  &style->bg[GTK_STATE_NORMAL],
					  (gchar **)bits);
    pw = gtk_pixmap_new(pixmap, mask);
    gtk_widget_show(pw);

    button = gtk_button_new();
    gtk_container_add(GTK_CONTAINER(button), pw);
    gtk_signal_connect(GTK_OBJECT(button), "clicked",
		       GTK_SIGNAL_FUNC(generic_cb),
		       GINT_TO_POINTER(data));
    gtk_widget_show(button);
    gtk_tooltips_set_tip(ttip, button, thelp, NULL);

    return button;
#else
    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_xpm_data(bits);
    GtkWidget *img = gtk_image_new_from_pixbuf(pixbuf);
    GtkWidget *button = gtk_button_new();
    gtk_button_set_image(GTK_BUTTON(button), img);
    gtk_signal_connect(GTK_OBJECT(button), "clicked",
		       GTK_SIGNAL_FUNC(generic_cb),
		       GINT_TO_POINTER(data));
    gtk_widget_show(button);

    return button;
#endif
}



static GtkWidget *
create_pixmap_label(GtkWidget *window, const char **bits)
{
#if  GTK_MAJOR_VERSION == 2
    GtkWidget	*pw;
    GdkPixmap	*pixmap;
    GdkBitmap	*mask;
    GtkStyle	*style;

    style = gtk_widget_get_style(window);
    pixmap = gdk_pixmap_create_from_xpm_d(window->window,
					  &mask,
					  &style->bg[GTK_STATE_NORMAL],
					  (gchar **)bits);
    pw = gtk_pixmap_new(pixmap, mask);
    gtk_widget_show(pw);

    return pw;
#else
    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_xpm_data(bits);
    GtkWidget *img = gtk_image_new_from_pixbuf(pixbuf);

    return img;
#endif
}


#if  GTK_MAJOR_VERSION == 2
void
set_icon_pixmap(GtkWidget *window, gchar **bits)
{
    GdkPixmap	*pixmap;
    GdkBitmap	*mask;
    GtkStyle	*style;

    style = gtk_widget_get_style(window);
    pixmap = gdk_pixmap_create_from_xpm_d(window->window,
					  &mask,
					  &style->bg[GTK_STATE_NORMAL],
					  bits);
    gdk_window_set_icon(window->window, NULL, pixmap, mask);
    gdk_window_set_icon_name(window->window, "TiMidity");
}
#else
// FIXME
#endif

static GtkWidget *
create_menubar(void)
{
#if  GTK_MAJOR_VERSION == 2
    GtkItemFactory	*ifactory;
    GtkAccelGroup	*ag;

    ag = gtk_accel_group_new();

    ifactory = gtk_item_factory_new(GTK_TYPE_MENU_BAR, "<Main>", ag);
    gtk_item_factory_create_items(ifactory,
				  sizeof(ife) / sizeof(GtkItemFactoryEntry),
				  ife, NULL);
    gtk_widget_show(ifactory->widget);

    auto_next = gtk_item_factory_get_widget(ifactory, "/Options/Auto next");
    gtk_check_menu_item_set_state(GTK_CHECK_MENU_ITEM(auto_next), TRUE);
    ttshow = gtk_item_factory_get_widget(ifactory, "/Options/Show tooltips");
    gtk_check_menu_item_set_state(GTK_CHECK_MENU_ITEM(ttshow), TRUE);

    return ifactory->widget;
#else
    GtkUIManager *uim;
    GError *error = NULL;
    GtkActionGroup *action_group;

    action_group = gtk_action_group_new ("AppActions");
    gtk_action_group_add_actions (action_group,
				  entries, n_entries,
				  window);
     gtk_action_group_add_toggle_actions (action_group,
					  toggle_entries, 
					  n_toggle_entries,
					  window);
    uim = gtk_ui_manager_new ();
    g_object_set_data_full (G_OBJECT (window),
			    "ui-manager", uim,
			    g_object_unref);
    
    gtk_ui_manager_insert_action_group (uim, action_group, 0);
    gtk_window_add_accel_group (GTK_WINDOW (window),
				gtk_ui_manager_get_accel_group (uim));
    
    uim = gtk_ui_manager_new ();
    gtk_ui_manager_insert_action_group (uim, action_group, 0);

    if (!gtk_ui_manager_add_ui_from_string (uim, ui_info, -1, &error))
	{
	    g_message ("Failed to build menus: %sn", error->message);
	    g_error_free (error);
	    error = NULL;
	}
    return gtk_ui_manager_get_widget (uim, "/ui/MenuBar");
#endif
}

/* Following function curtesy of the gtk mailing list. */

#if GTK_MAJOR_VERSION == 2
static GtkTooltips *
create_yellow_tooltips()
{
    GtkTooltips	*tip;

    /* First create a default Tooltip */
    tip = gtk_tooltips_new();

    return tip;
}
#endif

#if GTK_MAJOR_VERSION == 3
static void toggle_auto_cb ( GtkWidget *widget, gpointer data ) {
    if (auto_next) 
	auto_next = FALSE;
    else
	auto_next = TRUE;
}
#endif

/* Receive DATA sent by the application on the pipe     */

#if GTK_MAJOR_VERSION == 2
static void
handle_input(gpointer client_data, gint source, GdkInputCondition ic)
#else
    gboolean
    handle_input(GIOChannel *channel, GIOCondition cond, 
		 gpointer client_data)
    
#endif
{
    int message;

    gtk_pipe_int_read(&message);
    //fprintf(stderr, "Handle input with switch %d\n", message);

    switch (message) {
    case REFRESH_MESSAGE:
	g_warning("REFRESH MESSAGE IS OBSOLETE !!!");
	break;

    case TOTALTIME_MESSAGE:
	{
	    int tt;
	    int minutes,seconds;
	    char local_string[20];
#if GTK_MAJOR_VERSION == 2
	    GtkObject *adj;
#else
	    GtkAdjustment *adj;
#endif

	    gtk_pipe_int_read(&tt);

	    seconds=max_sec=tt/play_mode->rate;
	    minutes=seconds/60;
	    seconds-=minutes*60;
	    sprintf(local_string,"/ %i:%02i",minutes,seconds);
	    gtk_label_set_text(GTK_LABEL(tot_lbl), local_string);

	    /* Readjust the time scale */
	    adj = gtk_adjustment_new(0., 0., (gfloat)max_sec,
				     1., 10., 0.);
	    gtk_signal_connect(GTK_OBJECT(adj), "value_changed",
			       GTK_SIGNAL_FUNC(generic_scale_cb),
			       (gpointer)GTK_CHANGE_LOCATOR);
	    gtk_range_set_adjustment(GTK_RANGE(locator),
				     GTK_ADJUSTMENT(adj));
	}
	break;

    case MASTERVOL_MESSAGE:
	{
	    int volume;
	    GtkAdjustment *adj;

	    gtk_pipe_int_read(&volume);
	    adj = gtk_range_get_adjustment(GTK_RANGE(vol_scale));
	    my_adjustment_set_value(adj, MAX_AMPLIFICATION - volume);
	}
	break;

    case FILENAME_MESSAGE:
	{
	    char filename[255], title[255];
	    char *pc;

	    gtk_pipe_string_read(filename);

	    /* Extract basename of the file */
	    pc = strrchr(filename, '/');
	    if (pc == NULL)
		pc = filename;
	    else
		pc++;

	    sprintf(title, "Timidity %s - %s", timidity_version, pc);
	    gtk_window_set_title(GTK_WINDOW(window), title);

	    /* Clear the text area. */
	    textbuf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text));
	    gtk_text_buffer_get_start_iter(textbuf, &start_iter);
	    gtk_text_buffer_get_end_iter(textbuf, &end_iter);
	    //iter = start_iter;
	}
	break;

    case FILE_LIST_MESSAGE:
	{
	    gchar filename[255], *fnames[2];
	    gint i, number_of_files;

	    /* reset the playing list : play from the start */
	    file_number_to_play = -1;

	    gtk_pipe_int_read(&number_of_files);
	    for (i = 0; i < number_of_files; i++)
		{
		    gtk_pipe_string_read(filename);
#if GTK_MAJOR_VERSION == 2
		    fnames[0] = filename;
		    fnames[1] = NULL;
		    gtk_clist_append(GTK_CLIST(clist), fnames);
#else
		    GtkTreeIter   iter;
		    gtk_list_store_append (list_store, &iter);  /* Acquire an iterator */
		    gtk_list_store_set (list_store, &iter, 0, filename, -1);
#endif
		}
#if GTK_MAJOR_VERSION == 2
	    gtk_clist_columns_autosize(GTK_CLIST(clist));
#else
	    //FIXME
#endif
	}
	break;

    case NEXT_FILE_MESSAGE:
    case PREV_FILE_MESSAGE:
    case TUNE_END_MESSAGE:
	{
	    int nbfile;

	    /* When a file ends, launch next if auto_next toggle */
#if GTK_MAJOR_VERSION == 2
	    if ( (message==TUNE_END_MESSAGE) &&
		 !GTK_CHECK_MENU_ITEM(auto_next)->active )
		return;
#else
	    if ( (message==TUNE_END_MESSAGE) &&
		 auto_next)
		return TRUE;
#endif

	    /* Total number of file to play in the list */
#if GTK_MAJOR_VERSION == 2
	    nbfile = GTK_CLIST(clist)->rows;

	    if (message == PREV_FILE_MESSAGE)
		file_number_to_play--;
	    else
		file_number_to_play++;

	    /* Do nothing if requested file is before first one */
	    if (file_number_to_play < 0) {
		file_number_to_play = 0;
		return;
	    }
	    if (file_number_to_play >= nbfile) {
		file_number_to_play = nbfile - 1;
		return;
	    }
#else
	    GtkTreeIter iter;
	    if (message == PREV_FILE_MESSAGE) {
		if(gtk_list_store_is_valid(list_store, &select_iter) && 
		   (! gtk_tree_model_iter_previous((GtkTreeModel *)list_store, &select_iter)))
		    return TRUE;
	    }
	    else {
		if(gtk_list_store_is_valid(list_store, &select_iter) && 
		    (! gtk_tree_model_iter_next((GtkTreeModel *)list_store, &select_iter)))
		    return TRUE;
	    }
#endif

#if GTK_MAJOR_VERSION == 2
	    if(gtk_clist_row_is_visible(GTK_CLIST(clist),
					file_number_to_play) !=
	       GTK_VISIBILITY_FULL) {
		if(gtk_clist_row_is_visible(GTK_CLIST(clist),
					    file_number_to_play) !=
		   2) { // no longer in gtk.h FIXME
		    gtk_clist_moveto(GTK_CLIST(clist), file_number_to_play,
				     -1, 1.0, 0.0);
		}
		gtk_clist_select_row(GTK_CLIST(clist), file_number_to_play, 0);
	    }
#else
	    
#endif
	}
 
	break;

    case CURTIME_MESSAGE:
	{
	    int		seconds, minutes;
	    int		nbvoice;
	    char	local_string[20];

	    gtk_pipe_int_read(&seconds);
	    gtk_pipe_int_read(&nbvoice);

	    if( is_quitting ) {

#if GTK_MAJOR_VERSION == 2
		return;
#else
		return TRUE;
#endif 
	    }
	    minutes=seconds/60;

	    sprintf(local_string,"%2d:%02d", minutes, (int)(seconds % 60));

	    gtk_label_set_text(GTK_LABEL(cnt_lbl), local_string);

	    /* Readjust the time scale if not dragging the scale */
	    if( !locating && (seconds <= max_sec)) {
		GtkAdjustment *adj;

		adj = gtk_range_get_adjustment(GTK_RANGE(locator));
		my_adjustment_set_value(adj, (gfloat)seconds);
	    }
	}
	break;

    case NOTE_MESSAGE:
	{
	    int channel;
	    int note;

	    gtk_pipe_int_read(&channel);
	    gtk_pipe_int_read(&note);
	    g_warning("NOTE chn%i %i", channel, note);
	}
	break;

    case PROGRAM_MESSAGE:
	{
	    int channel;
	    int pgm;

	    gtk_pipe_int_read(&channel);
	    gtk_pipe_int_read(&pgm);
	    g_warning("NOTE chn%i %i", channel, pgm);
	}
	break;

    case VOLUME_MESSAGE:
	{
	    int channel;
	    int volume;

	    gtk_pipe_int_read(&channel);
	    gtk_pipe_int_read(&volume);
	    g_warning("VOLUME= chn%i %i", channel, volume);
	}
	break;


    case EXPRESSION_MESSAGE:
	{
	    int channel;
	    int express;

	    gtk_pipe_int_read(&channel);
	    gtk_pipe_int_read(&express);
	    g_warning("EXPRESSION= chn%i %i", channel, express);
	}
	break;

    case PANNING_MESSAGE:
	{
	    int channel;
	    int pan;

	    gtk_pipe_int_read(&channel);
	    gtk_pipe_int_read(&pan);
	    g_warning("PANNING= chn%i %i", channel, pan);
	}
	break;

    case SUSTAIN_MESSAGE:
	{
	    int channel;
	    int sust;

	    gtk_pipe_int_read(&channel);
	    gtk_pipe_int_read(&sust);
	    g_warning("SUSTAIN= chn%i %i", channel, sust);
	}
	break;

    case PITCH_MESSAGE:
	{
	    int channel;
	    int bend;

	    gtk_pipe_int_read(&channel);
	    gtk_pipe_int_read(&bend);
	    g_warning("PITCH BEND= chn%i %i", channel, bend);
	}
	break;

    case RESET_MESSAGE:
	g_warning("RESET_MESSAGE");
	break;

    case CLOSE_MESSAGE:
#if GTK_MAJOR_VERSION == 2
	gtk_exit(0);
#else
	exit(0);
#endif
	break;

    case CMSG_MESSAGE:
	{
	    int type;
	    char message[1000];
	    gchar *message_u8;

	    gtk_pipe_int_read(&type);
	    gtk_pipe_string_read(message);
	    fprintf(stderr, "Read msg from pipe %s\n", message);

	    message_u8 = g_locale_to_utf8( message, -1, NULL, NULL, NULL );
	    gtk_text_buffer_get_bounds(textbuf, &start_iter, &end_iter);
	    gtk_text_buffer_insert(textbuf, &end_iter,
				   message_u8, -1);
	    gtk_text_buffer_insert(textbuf, &end_iter,
				   "\n", 1);
	    gtk_text_buffer_get_bounds(textbuf, &start_iter, &end_iter);

	    mark = gtk_text_buffer_create_mark(textbuf, NULL, &end_iter, 1);
	    gtk_text_view_scroll_to_mark(GTK_TEXT_VIEW(text), mark, 0.0, 0, 0.0, 1.0);
	    gtk_text_buffer_delete_mark(textbuf, mark);
	    g_free( message_u8 );
	}
	break;
    case LYRIC_MESSAGE:
	{
	    char message[1000];
	    gchar *message_u8;

	    gtk_pipe_string_read(message);
	    fprintf(stderr, "Read lyric msg '%s'\n", message);

	    message_u8 = g_locale_to_utf8( message, -1, NULL, NULL, NULL );
	    gtk_text_buffer_get_bounds(textbuf, &start_iter, &end_iter);
	    // mod JN iter -> end_iter
	    gtk_text_buffer_insert(textbuf, &end_iter,
				   message_u8, -1);
	    
	    gtk_text_buffer_get_bounds(textbuf, &start_iter, &end_iter);

	    mark = gtk_text_buffer_create_mark(textbuf, NULL, &end_iter, 1);
	    gtk_text_view_scroll_to_mark(GTK_TEXT_VIEW(text), mark, 0.0, 0, 0.0, 1.0);
	    gtk_text_buffer_delete_mark(textbuf, mark);
	    	
	    //gtk_widget_queue_draw(GTK_WIDGET(text));
	}
	break;
    default:
	g_warning("UNKNOWN Gtk+ MESSAGE %i", message);

    }
#if GTK_MAJOR_VERSION == 3
    return(TRUE);
#endif
}

