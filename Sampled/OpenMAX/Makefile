
INCLUDES=-I ./libomxil-bellagio-0.9.3/include/
CFLAGS = -g

all: OpenMAX_IL_volume OpenMAX_IL_info

OpenMAX_IL_volume: OpenMAX_IL_volume.c
	cc $(INCLUDES) -o OpenMAX_IL_volume OpenMAX_IL_volume.c -L libomxil-bellagio-0.9.3/src/.libs -l omxil-bellagio

OpenMAX_IL_info: OpenMAX_IL_info.c
	cc $(FLAGS) $(INCLUDES) -o OpenMAX_IL_info OpenMAX_IL_info.c -L libomxil-bellagio-0.9.3/src/.libs -l omxil-bellagio

omxvolcontroltest: omxvolcontroltest.c
	cc $(INCLUDES) -o omxvolcontroltest omxvolcontroltest.c -L libomxil-bellagio-0.9.3/src/.libs -l omxil-bellagio

clean:
	rm OpenMAX_IL_volume omxvolcontroltest
