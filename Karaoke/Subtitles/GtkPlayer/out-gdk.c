#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>

#include "out.h"

#define MAXSLIDE 150

struct controls {
  GtkWidget *prbar;
  GtkWidget *d_area;
  GdkRectangle u_area;
  GdkGC *gc;
};

static GdkPixmap *screen;
static int end = 0;

void output_init(int *argc, char **argv[])
{
  gtk_init(argc, argv);
}

static void events_update(void)
{
#ifdef VIDEO
  while (gtk_events_pending()) {
    gtk_main_iteration_do(FALSE);
  }
#endif
  if (gtk_events_pending()) {
    gtk_main_iteration_do(FALSE);
  }
}

void pbar_update(void *c1, float l)
{
  struct controls *c = c1;

  l += MAXSLIDE / 2;
  if (l > MAXSLIDE) {
    l = MAXSLIDE;
  }
  gtk_progress_bar_update(GTK_PROGRESS_BAR(c->prbar), l / (float)MAXSLIDE);
}

int file_end(void)
{
  events_update();
  return end;
}

static void file_set_end(void)
{
  end = 1;
}

void frame_display(void *c1, int w, int h, unsigned char *b, int i)
{
  struct controls *c = c1;

  if (b == NULL) {
    file_set_end();
    return;
  }
  gdk_draw_rgb_image(screen, c->gc, 0, 0, w, h,
		  GDK_RGB_DITHER_MAX, b, w * 3);
  gtk_widget_draw(GTK_WIDGET(c->d_area), &c->u_area);
}

static gint delete_event(GtkWidget * widget, GdkEvent * event, gpointer data)
{
  end = 1;

  return (TRUE);
}

static gboolean expose_event(GtkWidget * widget, GdkEventExpose * event,
		gpointer data)
{
  if ((screen != NULL) && (!end)) {
    gdk_draw_pixmap(widget->window,
		    widget->style->fg_gc[GTK_WIDGET_STATE(widget)],
		    screen, event->area.x, event->area.y,
		    event->area.x, event->area.y, event->area.width,
		    event->area.height);
  }

  return TRUE;
}

static gint configure_event(GtkWidget * widget, GdkEventConfigure * event)
{
  if (!screen) {
    screen = gdk_pixmap_new(widget->window, widget->allocation.width,
		    widget->allocation.height, -1);
    gdk_draw_rectangle(screen, widget->style->white_gc, TRUE, 0, 0,
		    widget->allocation.width, widget->allocation.height);
  }
  
  return TRUE;
}

void *window_prepare(int w, int h)
{
  GdkGC *gc1;
  GtkWidget *vbox;
  GtkWidget *hbox;
  GtkWidget *label;
  GtkObject *adj2;
  GtkWidget *window;
  GdkColor black;
  GdkColor white;
  char buff[80];
  gint expid, confid;
  struct controls *c;

  c = malloc(sizeof(struct controls));
  if (c == NULL) {
    return NULL;
  }

  gdk_rgb_init();
  gtk_widget_set_default_colormap(gdk_rgb_get_cmap());
  gtk_widget_set_default_visual(gdk_rgb_get_visual());

  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_default_size(GTK_WINDOW(window), w, h + 50);

  vbox = gtk_vbox_new(FALSE, 0);
  hbox = gtk_hbox_new(FALSE, 0);
  gtk_container_add(GTK_CONTAINER(window), GTK_WIDGET(vbox));


  adj2 = gtk_adjustment_new(0.0, 0.0, 21.0, 0.1, 1.0, 1.0);
  c->prbar = gtk_progress_bar_new_with_adjustment(GTK_ADJUSTMENT(adj2));
  gtk_box_pack_start(GTK_BOX(vbox), c->prbar, TRUE, TRUE, 0);
  gtk_widget_show(c->prbar);
//drawing area
  c->d_area = gtk_drawing_area_new();
  gtk_drawing_area_size(GTK_DRAWING_AREA(c->d_area), w, h);
  expid = gtk_signal_connect(GTK_OBJECT(c->d_area), "expose_event",
			   GTK_SIGNAL_FUNC(expose_event), NULL);
  confid =
	gtk_signal_connect(GTK_OBJECT(c->d_area), "configure_event",
			   GTK_SIGNAL_FUNC(configure_event), NULL);
  gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(c->d_area), FALSE, FALSE, 5);
  gtk_widget_show(c->d_area);

  sprintf(buff, "QoSlib Demo");
  label = gtk_label_new(buff);
  gtk_box_pack_start(GTK_BOX(hbox), GTK_WIDGET(label), FALSE, FALSE, 5);
  gtk_widget_show(label);

  gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(hbox), FALSE, FALSE, 5);

  gtk_widget_show(hbox);
  gtk_widget_show(vbox);
  gtk_widget_show(window);
  gtk_signal_connect(GTK_OBJECT(window), "delete_event",
		       GTK_SIGNAL_FUNC(delete_event), NULL);

  c->u_area.x = 0;
  c->u_area.y = 0;
  c->u_area.width = c->d_area->allocation.width;
  c->u_area.height = c->d_area->allocation.height;
  c->gc = gdk_gc_new(c->d_area->window);
  gc1 = gdk_gc_new(c->d_area->window);

  gdk_color_black(gdk_colormap_get_system(), &black);
  gdk_color_white(gdk_colormap_get_system(), &white);
  gdk_gc_set_foreground(c->gc, &black);
  gdk_gc_set_background(c->gc, &white);
  gdk_gc_set_foreground(gc1, &white);
  gdk_gc_set_background(gc1, &black);

  return c;
}
