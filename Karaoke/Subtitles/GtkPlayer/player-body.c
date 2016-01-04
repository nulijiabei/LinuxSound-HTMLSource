#include <sys/time.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>
#include <signal.h>
#include <stdint.h>

#include "out.h"
#include "video-in.h"
#include "rt_utils.h"
#include "feedb.h"

static unsigned long long int timevect[10000];
static unsigned long int timevect1[10000];
static long int timevect2[10000];

static uint64_t gettime(void)
{
  uint64_t res;
  struct timeval now;

  gettimeofday(&now, NULL);
  res = now.tv_usec + now.tv_sec * 1000000;

  return res;
}

static void save_results(int myid)
{
    FILE *f;
    char filename[20];
    int i;

    printf("Saving\n");
    sprintf(filename, "res_%d_if.tim", myid);
    f = fopen(filename, "w");
    for (i = 1; i < 5000; i++) {
	if (timevect1[i] != 0) {
	    if (timevect[i] < timevect[i - 1]) {
		timevect[i] = timevect[i - 1];
//              fprintf(f, "%d\t-1\t%lu\n", i, (unsigned long int)timevect[i] /*- timevect[0]*/);
	    } else {
		if (timevect[i - 1] != timevect[i - 2]) {
		    fprintf(f, "%d\t%lu\t%lu\n", i,
			    (unsigned long int) (timevect[i] -
						 timevect[i - 1]),
			    (unsigned long int) timevect[i]
			    /*- timevect[0]*/
			);
		}
	    }
	}
    }
    sprintf(filename, "res_%d_ct.tim", myid);
    f = fopen(filename, "w");
    for (i = 1; i < 500; i++) {
	if (timevect1[i] != 0) {
	    fprintf(f, "%d\t%lu\t1\n", i,
		    (unsigned long int) (timevect1[i] - timevect[i]));
	}
    }
    fclose(f);
    sprintf(filename, "res_%d_l.tim", myid);
    f = fopen(filename, "w");
    for (i = 1; i < 500; i++) {
	fprintf(f, "%d\t%ld\n", i, timevect2[i]);
    }
    fclose(f);
   
    feedb_print_stats("res", myid);
}

#define T 40000		// FIXME
int mpbody(int myid, int argc, char *argv[])
{
    struct controls *c;
    int x, y, ox, oy, dir;
    void *file_ctx;
    unsigned char *rgbbuff;
    int i;
    int h, w;
    void *hnd, *periodic_hnd;

    output_init(&argc, &argv);
    if (argc != 2) {
	printf("Gimme the file name, please!!!\n");
	exit(-1);
    }

    file_ctx = file_open(argv[1]);
    
    hnd = feedb_start();
    periodic_hnd = start_periodic_timer(0, T);	/* FIXME: Period */

    x = 60;
    y = 60;
    ox = x;
    oy = y;
    dir = 10;
    i = 0;

    frame_size(file_ctx, &w, &h);
    c = window_prepare(w, h);

    printf("[MPEGDemo] Starting to play...\n");

    while (!file_end()) {
	timevect[i] = gettime();
	rgbbuff = frame_get(file_ctx);
        frame_display(c, w, h, rgbbuff, myid);
        if (i > 0) {
            long int l;

            l = /*qman_howlate(0)*/ timevect2[i - 1];
//          timevect2[i] = l;
	    l = l / 5000;	/* FIXME */
	    pbar_update(c, l);
	}

	timevect1[i] = gettime();
	timevect2[i++] = feedb_endcycle(hnd, periodic_hnd);
    }
    file_close(file_ctx);
    printf("Returning\n");

    save_results(myid);
    return 1;
}

void player_init(void)
{
  feedb_init();
}
