
LIBS = -lpulse-simple -lpulse -lpulsecommon -lsndfile
CFLAGS = -g

EXE: 	\
	pacat-simple \
	parec-simple \
	pa-mic-2-speaker-simple \
	pa-mic-2-speaker \
	pulsedevlist \
	parec \
	pacat \
	pacat2 \
	parec-latency \
	palist_clients \
	palist_devices \
	palist_devices_ongoing \
	pavolume \
	pavolume_client \
	pavolume_sink \
	pamonitor_clients \


all: $(EXE)

pacat-simple: pacat-simple.o
	gcc $(CFLAGS) $(LIBS) -o pacat-simple pacat-simple.o

parec-simple: parec-simple.o
	gcc $(CFLAGS) $(LIBS) -o parec-simple parec-simple.o

pa-mic-2-speaker-simple: pa-mic-2-speaker-simple.o
	gcc $(CFLAGS) $(LIBS) -o pa-mic-2-speaker-simple pa-mic-2-speaker-simple.o

pulsedevlist: pulsedevlist.o
	gcc $(CFLAGS) $(LIBS) -o pulsedevlist pulsedevlist.o

pacat: pacat.o
	gcc $(CFLAGS) $(LIBS) -o pacat pacat.o

pacat2: pacat2.o
	gcc $(CFLAGS) $(LIBS) -o pacat2 pacat2.o

parec: parec.o
	gcc $(CFLAGS) $(LIBS) -o parec parec.o

pa-mic-2-speaker: pa-mic-2-speaker.o
	gcc $(CFLAGS) $(LIBS) -o pa-mic-2-speaker pa-mic-2-speaker.o

parec-latency: parec-latency.o
	gcc $(CFLAGS) $(LIBS) -o parec-latency parec-latency.o

palist_clients: palist_clients.o
	gcc $(CFLAGS) $(LIBS) -o palist_clients palist_clients.o

palist_devices: palist_devices.o
	gcc $(CFLAGS) $(LIBS) -o palist_devices palist_devices.o

palist_devices_ongoing: palist_devices_ongoing.o
	gcc $(CFLAGS) $(LIBS) -o palist_devices_ongoing palist_devices_ongoing.o

pavolume: pavolume.o
	gcc $(CFLAGS) $(LIBS) -o pavolume pavolume.o

pavolume_client: pavolume_client.o
	gcc $(CFLAGS) $(LIBS) -o pavolume_client pavolume_client.o

pamonitor_clients: pamonitor_clients.o
	gcc $(CFLAGS) $(LIBS) -o pamonitor_clients pamonitor_clients.o

pavolume_sink: pavolume_sink.o
	gcc $(CFLAGS) $(LIBS) -o pavolume_sink pavolume_sink.o

clean:
	rm $(EXE) *.o
