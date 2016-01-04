
#include <gtk/gtk.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

int pipeAppli[2],pipeGIO[2]; /* Pipe for communication with GIO process   */
int fpip_in, fpip_out;	/* in and out depends in which process we are */
int pid;	               /* Pid for child process */

GIOChannel *channel_in, 
           *channel_out;


static void
pipe_error(char *st)
{
    fprintf(stderr,"CONNECTION PROBLEM WITH GIO+ PROCESS IN %s BECAUSE:%s\n",
	    st,
	    strerror(errno));
    exit(1);
}

gboolean
handle_input(GIOChannel *channel, GIOCondition cond, 
	     gpointer client_data)
{
    char str[256];
    GError *error = NULL;
    gsize len;

    if (cond == G_IO_HUP) exit(0);

    g_io_channel_read_chars(channel_in, str, 256,
			    &len, &error);
    printf("Str read from gio channel is %s\n", str);

    // send a response
    write_out();
}

void write_out() {
    char *str = "hello world reply\n";
    GError *error = NULL;
    gsize len;

    int message;

    g_io_channel_write_chars(channel_out, str, strlen(str)+1,
			    &len, &error);
    //g_io_channel_flush(channel_out, &error);
}

void Launch_GIO_Process(GIOChannel *channel) {
    GMainLoop *loop;

    loop = g_main_loop_new(NULL,FALSE);
    g_io_add_watch(channel,G_IO_IN | G_IO_HUP | G_IO_ERR,
		   (GIOFunc)handle_input, loop);
    g_main_loop_run(loop);
}

void app_handle() {
    char *str = "Hello world\n";;
    int nread;
    
    // include \0 in output
    nread = write(fpip_out, str, strlen(str)+1);
    
    sleep(1);

    char str_ret[256];
    nread = read(fpip_in, str_ret, 255);
    //str_ret[nread] = '\0';
    printf("Read back %s\n", str_ret);
}

void
pipe_open(void)
{
    int res;
    GError *error = NULL;
    
    res=pipe(pipeAppli);
    if (res!=0) pipe_error("PIPE_APPLI CREATION");
    
    res=pipe(pipeGIO);
    if (res!=0) pipe_error("PIPE_GIO CREATION");
    
    if ((pid=fork())==0) {   /*child*/
	close(pipeGIO[1]); 
	close(pipeAppli[0]);
	    
	fpip_in=pipeGIO[0];
	fpip_out= pipeAppli[1];

	channel_in =  g_io_channel_unix_new(fpip_in);
	channel_out =  g_io_channel_unix_new(fpip_out);
	
	g_io_channel_set_encoding(channel_in, NULL, &error);
	g_io_channel_set_encoding(channel_out, NULL, &error);

	g_io_channel_set_buffered(channel_in, FALSE);
	g_io_channel_set_buffered(channel_out, FALSE);

	Launch_GIO_Process(channel_in);

	/* Won't come back from here */
	fprintf(stderr,"WARNING: come back from GIO+\n");
	exit(0);
    }
    
    close(pipeGIO[0]);
    close(pipeAppli[1]);
    
    fpip_in= pipeAppli[0];
    fpip_out= pipeGIO[1];

    app_handle();
}

int main(int argc, char **argv) {
    pipe_open();
}
