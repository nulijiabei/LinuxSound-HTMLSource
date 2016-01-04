

LIBS = -ldl

all: applyplugin mono_amp stereo_amp zip

applyplugin: applyplugin.c load.c
	cc -o applyplugin applyplugin.c load.c $(LIBS)

mono_amp:
	(cd mono_amp; make)

stereo_amp:
	(cd stereo_amp; make)

zip:
	zip all.zip *.c mono_amp/*.[ch]  stereo_amp/*.[ch] Makefile *_amp/Makefile
