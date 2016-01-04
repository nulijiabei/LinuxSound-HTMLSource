
LIBS = -I/usr/include/ffmpeg/ -lavformat -lavcodec -lavutil -lao
#LIBS = -Iffmpeg-1.1.3 -Lffmpeg-1.1.3/libavformat:ffmpeg-1.1.3/libavcodec -lavformat -lavcodec -lavutil -lao

CFLAGS = -g

EXE: 	\
	play


all: $(EXE)


play: play.c
	gcc $(CFLAGS) $(LIBS) -o play play.c

clean:
	rm play

