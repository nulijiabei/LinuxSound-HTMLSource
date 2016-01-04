
V = 3
T = ../../../MIDI/Timidity/TiMidity++-2.14.0
T_INC = -I $(T)/timidity -I $(T)/utils -I $(T)/libarc

INCLUDES =  $(T_INC) $(shell pkg-config --cflags gtk+-$(V).0  libavformat libavcodec libavutil libswscale)
CFLAGS = -g $(INCLUDES)
LIBS = $(shell pkg-config --libs gtk+-$(V).0  libavformat libavcodec libavutil libswscale)

all: if_video.so testio

testio: test.c
	gcc $(CFLAGS) test.c -o testio $(LIBS)

if_video.so: gtk_c.o gtk_i.o gtk_p.o
	gcc -shared -o if_video.so  gtk_c.o gtk_i.o gtk_p.o $(LIBS)

gtk_c.o : gtk_c.c
	gcc  -fPIC $(CFLAGS) -c -o gtk_c.o gtk_c.c $(LIBS)

gtk_i.o : gtk_i.c
	gcc  -fPIC $(CFLAGS) -c -o gtk_i.o gtk_i.c $(LIBS)

gtk_p.o : gtk_p.c
	gcc  -fPIC $(CFLAGS) -c -o gtk_p.o gtk_p.c $(LIBS)

clean:
	rm *.o *.so
