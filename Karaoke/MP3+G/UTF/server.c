#include <stdio.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <stdlib.h>
#include <signal.h>
#include <string.h>

#define SIZE 1024 
char buf[SIZE]; 
#define TIME_PORT 8189

int sockfd, client_sockfd; 

void intHandler(int dummy) {
    close(client_sockfd);
    close(sockfd);
    exit(1);
}

int main(int argc, char *argv[]) { 
    int sockfd, client_sockfd; 
    int nread, len; 
    struct sockaddr_in serv_addr, client_addr; 
    time_t t; 

    signal(SIGINT, intHandler);
    
    /* create endpoint */ 
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) { 
	perror(NULL); exit(2); 
    } 
    /* bind address */ 
    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); 
    serv_addr.sin_port = htons(TIME_PORT); 
    if (bind(sockfd, 
	     (struct sockaddr *) &serv_addr, 
	     sizeof(serv_addr)) < 0) { 
	perror(NULL); exit(3); 
    } 
    /* specify queue */ 
    listen(sockfd, 5); 
    for (;;) { 
	len = sizeof(client_addr); 
	client_sockfd = accept(sockfd, 
			       (struct sockaddr *) &client_addr, 
			       &len); 
	if (client_sockfd == -1) { 
	    perror(NULL); continue; 
	} 
	printf("Client connected\n");
	while ((nread = read(client_sockfd, buf, SIZE-1)) > 0) {
	    int n;
	    buf[nread] = '\0';
	    for (n = 0; n < strlen(buf); n++)
		fprintf(stdout, "%X ", buf[n]);
	    fputs("\n", stdout);

	    fputs(buf, stdout);
	    fflush(stdout);
	}
	close(client_sockfd); 
    }
}
