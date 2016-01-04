CFLAGS += -O2 -Wall -Wstrict-prototypes
GTKLIB = `gtk-config --libs`
GTKCFLAGS = `gtk-config --cflags`
GTKLIB = `pkg-config --libs gtk+-2.0`
GTKCFLAGS = `pkg-config --cflags gtk+-2.0`
TESTS = player
LDFLAGS=-L rtutils
LDLIBS=-lrt_utils -lrt

FF_CPPFLAGS = -I $(FFDIR)

all:	$(TESTS)

fake-feedb.o player-body.o: CPPFLAGS += -I rtutils
out-gdk.o: CFLAGS += $(GTKCFLAGS)
in-libav.o: CPPFLAGS += $(FF_CPPFLAGS)

player: LDFLAGS += $(GTKLIB)
player: LDFLAGS += -L $(FFDIR)/libavutil -L $(FFDIR)/libavcodec -L $(FFDIR)/libavformat
player: LDLIBS +=  -lavformat -lavcodec -lavutil -lz -lbz2 -lm

player: player.o fake-feedb.o player-body.o out-gdk.o in-libav.o

clean:		
	rm -rf *.o
	rm -rf $(TESTS)
