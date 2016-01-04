/* 
    TiMidity++ -- MIDI to WAVE converter and player
    Copyright (C) 1999-2002 Masanao Izumo <mo@goice.co.jp>
    Copyright (C) 1995 Tuukka Toivonen <tt@cgs.fi>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    motif_pipe.c: written by Vincent Pagel (pagel@loria.fr) 10/4/95
   
    pipe communication between motif interface and sound generator

    Copied from the motif_p source. - Glenn Trigg, 29 Oct 1998

    */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/ioctl.h>
#ifdef SOLARIS
#include <sys/filio.h>
#endif
#ifndef NO_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif
#include <sys/time.h>
#include <sys/types.h>

#include <gtk/gtk.h>

#if GTK_MAJOR_VERSION == 3
#include <glib.h>
#endif

#include "timidity.h"
#include "controls.h"
#include "gtk_h.h"

int pipeAppli[2],pipeGtk[2]; /* Pipe for communication with Gtk+ process   */
int fpip_in, fpip_out;	/* in and out depends in which process we are */
int pid;	               /* Pid for child process */

#if GTK_MAJOR_VERSION != 2
GIOChannel *channel_in, 
           *channel_out; /* in and out depends in which process we are */

static void
pipe_error(char *);

void status_error(GIOStatus status) {
    switch (status) {
    case G_IO_STATUS_ERROR:
	fprintf(stderr, "GIO status: error\n");
    case G_IO_STATUS_EOF:
	fprintf(stderr, "GIO status: eof\n");
    case G_IO_STATUS_AGAIN:
	fprintf(stderr, "GIO status: again\n");
    }
}

void channel_error(char *str, GIOStatus status, GError *error) {
    if (error != NULL)
	fprintf(stderr, "GIO channel error: %s\n", error->message);
    status_error(status);
    pipe_error(str);
}
#endif

/* DATA VALIDITY CHECK */
#define INT_CODE 214
#define STRING_CODE 216

#define DEBUGPIPE

/***********************************************************************/
/* PIPE COMUNICATION                                                   */
/***********************************************************************/

/* See http://docs.huihoo.com/symbian/nokia-symbian3-developers-library-v0.8/GUID-817C43E8-9169-4750-818B-B431D138D71A.html
 * for use of GIOChannels
 */

static void
pipe_error(char *st)
{
    fprintf(stderr,"CONNECTION PROBLEM WITH Gtk+ PROCESS IN %s BECAUSE:%s\n",
	    st,
	    strerror(errno));
    exit(1);
}


/*****************************************
 *              INT                      *
 *****************************************/


void
gtk_pipe_int_write(int c)
{
#if GTK_MAJOR_VERSION == 2
    int len;
    int code=INT_CODE;

#ifdef DEBUGPIPE
    //fprintf(stderr, "Writing debug int %d\n", code);
    len = write(fpip_out,&code,sizeof(code)); 
    if (len!=sizeof(code))
	pipe_error("PIPE_INT_WRITE");
#endif
    //fprintf(stderr, "Writing int %d\n", c);

    len = write(fpip_out,&c,sizeof(c)); 
    if (len!=sizeof(int))
	pipe_error("PIPE_INT_WRITE");
#else // GTK 3
    gsize len;
    int code=INT_CODE;
    GError *error = NULL;
    GIOStatus status;

#ifdef DEBUGPIPE
    //fprintf(stderr, "Writing gtk debug int %d\n", code);
    status = g_io_channel_write_chars(channel_out, (gchar *)&code, sizeof(code),
				      &len, &error); 
    if ((len!=sizeof(code)) || (status != G_IO_STATUS_NORMAL))
	channel_error("CHANNEL_INT_WRITE", status, error);
    g_io_channel_flush(channel_out, &error);
#endif
    //fprintf(stderr, "Writing gtk int %d\n", c);

    g_io_channel_write_chars(channel_out, (gchar *)&c, sizeof(c),
			     &len, &error); 
    if (len!=sizeof(int))
	pipe_error("CHANNEL_INT_WRITE");
    g_io_channel_flush(channel_out, &error);
#endif
}

void
gtk_pipe_int_read(int *c)
{
#if GTK_MAJOR_VERSION == 2
    int len;

#ifdef DEBUGPIPE
    int code;

    len = read(fpip_in,&code,sizeof(code)); 
    //fprintf(stderr, "Read debug int %d\n", code);

    if (len!=sizeof(code))
	pipe_error("PIPE_INT_READ");

    if (code!=INT_CODE)	
	fprintf(stderr,"BUG ALERT ON INT PIPE %i\n",code);
#endif

    len = read(fpip_in,c, sizeof(int)); 
    //fprintf(stderr, "Read int %d\n", *c);
    if (len!=sizeof(int)) pipe_error("PIPE_INT_READ");
#else // GTK 3
    gsize len;
    GError *error = NULL;
    GIOStatus status;

#ifdef DEBUGPIPE
    int code;

    status = g_io_channel_read_chars(channel_in, (gchar *)&code, sizeof(code),
			     &len, &error);
    if ((len!=sizeof(code)) || (status != G_IO_STATUS_NORMAL))
	channel_error("CHANNEL_INT_READ", status, error); 
    //fprintf(stderr, "Read gtk debug int %d\n", code);

    if (code!=INT_CODE)	
	fprintf(stderr,"BUG ALERT ON INT CHANNEL %i\n",code);
#endif

    status = g_io_channel_read_chars(channel_in, (gchar *) c, sizeof(int),
			    &len, &error);    
    if ((len!=sizeof(int)) || (status != G_IO_STATUS_NORMAL))
	channel_error("CHANNEL_INT_READ", status, error); 
 
    //fprintf(stderr, "Read gtk int %d\n", *c);
    // if (len!=sizeof(int)) pipe_error("CHANNEL_INT_READ");
#endif
}

void
pipe_int_write(int c)
{
    int len;
    int code=INT_CODE;

#ifdef DEBUGPIPE
    //fprintf(stderr, "Writing pipe debug int %d\n", code);
    len = write(fpip_out,&code,sizeof(code)); 
    if (len!=sizeof(code))
	pipe_error("PIPE_INT_WRITE");
#endif
    //fprintf(stderr, "Writing pipe int %d\n", c);

    len = write(fpip_out,&c,sizeof(c)); 
    if (len!=sizeof(int))
	pipe_error("PIPE_INT_WRITE");
}


void
pipe_int_read(int *c)
{
    int len;

#ifdef DEBUGPIPE
    int code;

    len = read(fpip_in,&code,sizeof(code)); 
    //fprintf(stderr, "Read pipe debug int from pipe %d\n", code);

    if (len!=sizeof(code))
	switch (len) {
	case 0:
	    pipe_error("PIPE_INT_READ_EOF");
	case -1:
	    pipe_error("PIPE_INT_READ_EAGAIN");
	default:
	    pipe_error("PIPE_INT_READ");
	}

    if (code!=INT_CODE)	
	fprintf(stderr,"BUG ALERT ON INT PIPE %i\n",code);
#endif

    len = read(fpip_in,c, sizeof(int)); 
    //fprintf(stderr, "Read pipe int from pipe %d\n", *c);
    if (len!=sizeof(int)) pipe_error("PIPE_INT_READ");
}

/*****************************************
 *              STRINGS                  *
 *****************************************/


void
gtk_pipe_string_write(char *str)
{
#if GTK_MAJOR_VERSION == 2
   int len, slen;

#ifdef DEBUGPIPE
   int code=STRING_CODE;
   //fprintf(stderr, "Writing debug int %d\n", code);

   len = write(fpip_out,&code,sizeof(code)); 
   if (len!=sizeof(code))
       pipe_error("PIPE_STRING_WRITE");
#endif
  
   slen=strlen(str);
   //fprintf(stderr, "Writing int %d\n", slen);
   len = write(fpip_out,&slen,sizeof(slen)); 
   if (len!=sizeof(slen)) 
       pipe_error("PIPE_STRING_WRITE");

   //fprintf(stderr, "Writing str %s\n", str);
   len = write(fpip_out,str,slen); 
   if (len!=slen) pipe_error("PIPE_STRING_WRITE on string part");
#else // GTK 3
   gsize len;
   int slen;
   GError *error = NULL;

#ifdef DEBUGPIPE
   int code=STRING_CODE;

   //fprintf(stderr, "Writing gtk debug int %d\n", code);

   g_io_channel_write_chars(channel_out,(gchar *)&code, sizeof(code),
			  &len, &error); 
   if (len!=sizeof(code))	
       pipe_error("CHANNEL_STRING_WRITE");
    g_io_channel_flush(channel_out, &error);
#endif
  
   slen=strlen(str);
   //fprintf(stderr, "Writing gtk strlen int %d of size %ld\n", slen, sizeof(slen));

   g_io_channel_write_chars(channel_out, (gchar *)&slen, sizeof(slen),
			   &len, &error); 
   if (len != sizeof(slen)) 
       pipe_error("CHANNEL_STRING_WRITE");
   g_io_channel_flush(channel_out, &error);

   //fprintf(stderr, "Writing gtk str %s\n", str);

   g_io_channel_write_chars(channel_out, str, slen,
			   &len, &error); 
   if (len!=slen) pipe_error("CHANNEL_STRING_WRITE on string part");
   g_io_channel_flush(channel_out, &error);
#endif
}

void
gtk_pipe_string_read(char *str)
{
#if GTK_MAJOR_VERSION == 2
    int len, slen;

#ifdef DEBUGPIPE
    int code;

    len = read(fpip_in,&code,sizeof(code));
    //fprintf(stderr, "Reading debug int %d\n", code);

    if (len!=sizeof(code)) pipe_error("PIPE_STRING_READ");
    if (code!=STRING_CODE) fprintf(stderr,"BUG ALERT ON STRING PIPE %i\n",code);
#endif

    len = read(fpip_in,&slen,sizeof(slen));
    //fprintf(stderr, "Reading int %d\n", slen);

    if (len!=sizeof(slen)) pipe_error("PIPE_STRING_READ");
    //fprintf(stderr, "Reading int %d\n", len);
    
    len = read(fpip_in,str,slen);
    if (len!=slen) pipe_error("PIPE_STRING_READ on string part");
    str[slen]='\0';		/* Append a terminal 0 */
    //fprintf(stderr, "Reading str %s\n", str);

#else // GTK 3
    gsize len;
    int slen;
    GError *error = NULL;

#ifdef DEBUGPIPE
    int code;

    g_io_channel_read_chars(channel_in, (gchar *)&code, sizeof(code),
			    &len, &error); 
    //fprintf(stderr, "Reading gtk debug string int %d size %ld\n", code, sizeof(code));

    if (len!=sizeof(code)) pipe_error("CHANNEL_STRING_READ");
    if (code!=STRING_CODE) fprintf(stderr,"BUG ALERT ON STRING CHANNEL %i\n",code);
#endif

    g_io_channel_read_chars(channel_in, (gchar *)&slen, sizeof(slen),
			   &len, &error); 
    //fprintf(stderr, "Reading gtk strlen int %d\n", (int)slen);
    if (len!=sizeof(slen)) pipe_error("CHANNEL_STRING_READ");
    
    g_io_channel_read_chars(channel_in, str, slen,
			   &len, &error); 
    if (len!=slen) pipe_error("CHANNEL_STRING_READ on string part");
    str[slen]='\0';		/* Append a terminal 0 */
    //fprintf(stderr, "Reading gtk str %s\n", str);
#endif
}

void
pipe_string_write(char *str)
{
   int len, slen;

#ifdef DEBUGPIPE
   int code=STRING_CODE;
   //fprintf(stderr, "Writing pipe debug int %d\n", code);

   len = write(fpip_out,&code,sizeof(code)); 
   if (len!=sizeof(code))
       pipe_error("PIPE_STRING_WRITE");
#endif
  
   slen=strlen(str);
   //fprintf(stderr, "Writing pipe strlen %d\n", slen);
   len = write(fpip_out,&slen,sizeof(slen)); 
   if (len!=sizeof(slen)) 
       pipe_error("PIPE_STRING_WRITE");

   //fprintf(stderr, "Writing pipe str %s\n", str);
   len = write(fpip_out,str,slen); 
   if (len!=slen) pipe_error("PIPE_STRING_WRITE on string part");
}



pipe_string_read(char *str)
{
    int len, slen;

#ifdef DEBUGPIPE
    int code;

    len = read(fpip_in,&code,sizeof(code));
    //fprintf(stderr, "Reading pipe debug int %d\n", code);

    if (len!=sizeof(code)) pipe_error("PIPE_STRING_READ");
    if (code!=STRING_CODE) fprintf(stderr,"BUG ALERT ON STRING PIPE %i\n",code);
#endif

    len = read(fpip_in,&slen,sizeof(slen));
    //fprintf(stderr, "Reading pipe string length %d\n", slen);

    if (len!=sizeof(slen)) pipe_error("PIPE_STRING_READ");
    //fprintf(stderr, "Read num bytes %d\n", len);

    len = read(fpip_in,str,slen);
    if (len!=slen) pipe_error("PIPE_STRING_READ on string part");
    str[slen]='\0';		/* Append a terminal 0 */
    //fprintf(stderr, "Reading pipe str from pipe '%s'\n", str);
}

int
pipe_read_ready(void)
{
    fd_set fds;
    int cnt;
    struct timeval timeout;


    FD_ZERO(&fds);
    FD_SET(fpip_in, &fds);
    timeout.tv_sec = timeout.tv_usec = 0;
    if((cnt = select(fpip_in + 1, &fds, NULL, NULL, &timeout)) < 0)
    {
	perror("select");
	return -1;
    }

    return cnt > 0 && FD_ISSET(fpip_in, &fds) != 0;
}

void
gtk_pipe_open(void)
{
    int res;
    GError *error = NULL;
    
    res=pipe(pipeAppli);
    if (res!=0) pipe_error("PIPE_APPLI CREATION");
    
    res=pipe(pipeGtk);
    if (res!=0) pipe_error("PIPE_GTK CREATION");
    
    if ((pid=fork())==0) {   /*child*/
	close(pipeGtk[1]); 
	close(pipeAppli[0]);
	    
	fpip_in=pipeGtk[0];
	fpip_out= pipeAppli[1];

#if GTK_MAJOR_VERSION == 2
	Launch_Gtk_Process(fpip_in);
#else
	channel_in =  g_io_channel_unix_new(fpip_in);
	channel_out =  g_io_channel_unix_new(fpip_out);

	/* set to raw I/O, unbuffered */
	g_io_channel_set_encoding(channel_in, NULL, &error);
	g_io_channel_set_encoding(channel_out, NULL, &error);
	g_io_channel_set_buffered(channel_in, FALSE);
	g_io_channel_set_buffered(channel_out, FALSE);

	Launch_Gtk_Process(channel_in);
#endif
	    

	/* Won't come back from here */
	fprintf(stderr,"WARNING: come back from Gtk+\n");
	exit(0);
    }
    
    close(pipeGtk[0]);
    close(pipeAppli[1]);
    
    fpip_in= pipeAppli[0];
    fpip_out= pipeGtk[1];

#if GTK_MAJOR_VERSION == 3
    /*
    channel_in =  g_io_channel_unix_new(fpip_in);
    */
    //channel_out =  g_io_channel_unix_new(fpip_out);
    //g_io_channel_set_encoding(channel_in, NULL, &error);
    //g_io_channel_set_encoding(channel_out, NULL, &error);
#endif
}
