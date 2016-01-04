
V = 3

LIBS = -l glib-2.0 $(shell pkg-config --libs gtk+-$(V).0  libavformat libavcodec libavutil libswscale) -lpthread -lX11 
# INCLUDES = -I fluidsynth-1.1.6-new/include/ -I fluidsynth-1.1.6-new/src/midi/ -I fluidsynth-1.1.6-new/src/utils/ -I/usr/include/glib-2.0 -I/usr/lib64/glib-2.0/include -I/usr/include/gtk-2.0/ -I/usr/include/cairo -I/usr/include/pango-1.0/ -I/usr/lib64/gtk-2.0/include -I/usr/include/gdk-pixbuf-2.0/ -I/usr/include/atk-1.0

INCLUDES =  $(shell pkg-config --cflags gtk+-$(V).0 libavformat libavcodec libswscale libavutil ) 

CFLAGS = -g $(INCLUDES)

EXE:    \
	text \
	textwatch \


all: $(EXE)

text: text.o
	gcc text.o $(LIBS) -o text


textwatch: textwatch.o
	gcc textwatch.o $(LIBS) -o textwatch

file_player: file_player.o
	gcc $(LIBS) -o file_player file_player.o

display_image: display_image.o
	gcc  display_image.o $(LIBS) -o display_image

karaoke_player: karaoke_player.o
	gcc $(LIBS) -o karaoke_player karaoke_player.o

gtkkaraoke_player: gtkkaraoke_player.o
	gcc gtkkaraoke_player.o $(LIBS) -o gtkkaraoke_player

gtkkaraoke_player2: gtkkaraoke_player2.o
	gcc gtkkaraoke_player2.o $(LIBS) -o gtkkaraoke_player2

play_video: play_video.o
	gcc play_video.o $(LIBS) -o play_video

gtk_play_video: gtk_play_video.o
	gcc gtk_play_video.o $(LIBS) -o gtk_play_video

gtk_play_video_overlay: gtk_play_video_overlay.o
	gcc gtk_play_video_overlay.o $(LIBS) -o gtk_play_video_overlay

gtk_play_video_overlay_alpha: gtk_play_video_overlay_alpha.o
	gcc gtk_play_video_overlay_alpha.o $(LIBS) -o gtk_play_video_overlay_alpha

gtk_play_video_cairo: gtk_play_video_cairo.o
	gcc gtk_play_video_cairo.o $(LIBS) -o gtk_play_video_cairo

gtk_play_video_pango: gtk_play_video_pango.o
	gcc gtk_play_video_pango.o $(LIBS) -o gtk_play_video_pango

tutorial02: tutorial02.o
	gcc tutorial02.o -lavutil -lavformat -lavcodec -lswscale -lz -lm -lSDL -o tutorial02

tutorial05: tutorial05.o
	gcc tutorial05.o -lavutil -lavformat -lavcodec -lswscale -lz -lm -lSDL -o tutorial05

test_label: test_label.o
	gcc $(LIBS) -o test_label test_label.o

tutorial_play: tutorial_play.o
	gcc  tutorial_play.o $(LIBS) -o tutorial_play


clean:
	rm $(EXE) *.o
