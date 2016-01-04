#include<sys/types.h>
#include<sys/wait.h>
#include <unistd.h>
#include<stdio.h>

#define N 2

int mpbody(int myid, int argc, char *argv[]);
void player_init(void);

int main(int argc, char *argv[])
{
    int i;
    int dummy;

    player_init();
    for (i = 0; i < N; i++) {
        int res;

        res = fork();
	if (res < 0) {
            perror("Fork");

	    return -1;
	}
	if (res == 0) {
            mpbody(i, argc, argv);

	    return 0;
	}
    }
    printf("Waiting...\n");
    for (i = 0; i < N; i++) {
        wait(&dummy);
    }
    sleep(2);

    return 0;
}
