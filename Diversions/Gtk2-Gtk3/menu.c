
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
  { "Auto next",     "document-open", "_Auto", "<control>A",
    "Auto next", G_CALLBACK (open_file_cb) },
  { "Clear All",     "document-open", "_Clear", "<control>C",
    "Clear all", G_CALLBACK (playlist_cb) },
};
static guint n_entries = G_N_ELEMENTS (entries);

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

