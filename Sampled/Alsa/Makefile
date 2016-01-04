
LIBS = `pkg-config --libs alsa`
# LIBS = -lasound -lm

CFLAGS = -g

EXE: 	\
	alsa_capture \
	alsa_playback \
	alsa-mic-2-speaker \
	alsa_device_info \
	alsa_more_device_info \
	aplay-l \
	aplay-L \
	latency \
	latency.orig \
	playback-capture \
	device-info \
	volume \


all: $(EXE)


alsa_capture: alsa_capture.o
	gcc  -o alsa_capture alsa_capture.o $(LIBS)

alsa_playback: alsa_playback.o
	gcc  -o alsa_playback alsa_playback.o $(LIBS)

alsa-mic-2-speaker: alsa-mic-2-speaker.o
	gcc  -o alsa-mic-2-speaker alsa-mic-2-speaker.o $(LIBS)

alsa_device_info: alsa_device_info.o
	gcc  -o alsa_device_info alsa_device_info.o $(LIBS)

alsa_more_device_info: alsa_more_device_info.o
	gcc  -o alsa_more_device_info alsa_more_device_info.o $(LIBS)

aplay-l: aplay-l.o
	gcc  -o aplay-l aplay-l.o $(LIBS)

aplay-L: aplay-L.o
	gcc  -o aplay-L aplay-L.o $(LIBS)

latency: latency.o
	gcc  -o latency latency.o $(LIBS)

latency.orig: latency.orig.o
	gcc  -o latency.orig latency.orig.o $(LIBS)

playback-capture: playback-capture.o
	gcc  -o playback-capture playback-capture.o $(LIBS)

device-info: device-info.o
	gcc  -o device-info device-info.o $(LIBS)

volume: volume.o
	gcc  -o volume volume.o $(LIBS)

clean:
	rm $(EXE) *.o
