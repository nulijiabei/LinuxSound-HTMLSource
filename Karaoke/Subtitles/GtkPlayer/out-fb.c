#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <linux/fb.h>

#include "out.h"

//#define FULLCOLOR

struct controls {
  int fb_size;
  int fb_line_len;
  int bpp;
  struct fb_cmap *old_cmap;
  int fbdev_fd;
  unsigned char *b;
};


static int end = 0;
static char *fbdevname = "/dev/fb0";
static struct fb_cmap old_cmap;

#ifdef FULLCOLOR
int color_setup(struct controls *p, struct fb_var_screeninfo *fb_vinfo)
{
  int res;

  fb_vinfo->bits_per_pixel = 32;
fprintf(stderr, "BPP: %d\n", fb_vinfo->bits_per_pixel);
fb_vinfo->red.msb_right = fb_vinfo->green.msb_right = fb_vinfo->blue.msb_right = fb_vinfo->transp.msb_right = 0;
fb_vinfo->transp.offset = fb_vinfo->transp.length = 0;
fb_vinfo->blue.offset = 0;
fb_vinfo->transp.offset = 24; fb_vinfo->transp.length = 8;

  res = ioctl(p->fbdev_fd, FBIOPUT_VSCREENINFO, fb_vinfo);
  if (res < 0) {
    perror("Error opening fbdev");
    close(p->fbdev_fd);
    free(p);
    
    return -1;
  }
fprintf(stderr, "BPP1: %d\n", fb_vinfo->bits_per_pixel);

  return 0;
}
#else

static struct fb_cmap *cmap_bw(int size)
{
  int i;
  unsigned short int *red, *green, *blue;
  struct fb_cmap *cmap;
        
  red = malloc(size * sizeof(red[0]));
  if(!red) {
    fprintf(stderr, "Can't allocate red palette with %d entries.\n", size);

    return NULL;
  }
  for(i = 0; i < size; i++) {
    red[i] = (65535 / (size - 1)) * i;
  }
  
  green = malloc(size * sizeof(green[0]));
  if(!green) {
    fprintf(stderr, "Can't allocate green palette with %d entries.\n", size);
    free(red);
    
    return NULL;
  }
  for(i = 0; i< size; i++) {
    green[i] = (65535 / (size - 1)) * i;
  }
  
  blue = malloc(size * sizeof(blue[0]));
  if(!blue) {
    fprintf(stderr, "Can't allocate blue palette with %d entries.\n", size);
    free(red);
    free(green);
    
    return NULL;
  }
  for(i = 0; i< size; i++) {
    blue[i] = (65535 / (size - 1)) * i;
  }
  
  cmap = malloc(sizeof(struct fb_cmap));
  if(!cmap) {
    fprintf(stderr, "Can't allocate color map\n");
    free(red);
    free(green);
    free(blue);

    return NULL;
  }
  cmap->start = 0;
  cmap->transp = 0;
  cmap->len = size;
  cmap->red = red;
  cmap->blue = blue;
  cmap->green = green;
  cmap->transp = NULL;
  
  return cmap;
}

static struct fb_cmap *cmap_color(/*int rsize, int bsize, int gsize*/ int size)
{
  int i;
  unsigned short int *red, *green, *blue;
  struct fb_cmap *cmap;

  printf("Colormap Size: %d\n", size);
  red = malloc(size * sizeof(red[0]));
  if(!red) {
    fprintf(stderr, "Can't allocate red palette with %d entries.\n", size);

    return NULL;
  }
  
  green = malloc(size * sizeof(green[0]));
  if(!green) {
    fprintf(stderr, "Can't allocate green palette with %d entries.\n", size);
    free(red);
    
    return NULL;
  }
  
  blue = malloc(size * sizeof(blue[0]));
  if(!blue) {
    fprintf(stderr, "Can't allocate blue palette with %d entries.\n", size);
    free(red);
    free(green);
    
    return NULL;
  }
  for(i = 0; i < size; i++) {
    red[i] = (65535/(size - 1)) * i;
    blue[i] = (65535/(size - 1)) * i;
    green[i] = (65535/(size - 1)) * i;
//    printf("%d ---> R: %d  G: %d  B: %d\n", i, red[i], green[i], blue[i]);
  }
  
  cmap = malloc(sizeof(struct fb_cmap));
  if(!cmap) {
    fprintf(stderr, "Can't allocate color map\n");
    free(red);
    free(green);
    free(blue);

    return NULL;
  }
  cmap->start = 0;
  cmap->transp = 0;
  cmap->len = size;
  cmap->red = red;
  cmap->blue = blue;
  cmap->green = green;
  cmap->transp = NULL;
  
  return cmap;
}

int color_setup(struct controls *p, struct fb_var_screeninfo *fb_vinfo)
{
  struct fb_cmap *cmap;
  int res;




fb_vinfo->bits_per_pixel = 24;
fprintf(stderr, "BPP: %d\n", fb_vinfo->bits_per_pixel);
fb_vinfo->red.msb_right = fb_vinfo->green.msb_right = fb_vinfo->blue.msb_right = fb_vinfo->transp.msb_right = 0;
fb_vinfo->transp.offset = fb_vinfo->transp.length = 0;
fb_vinfo->blue.offset = 0;
fb_vinfo->transp.offset = 24; fb_vinfo->transp.length = 8;

res = ioctl(p->fbdev_fd, FBIOPUT_VSCREENINFO, fb_vinfo);
if (res < 0) {
perror("Error opening fbdev");
close(p->fbdev_fd);
free(p);

return -1;
}
fprintf(stderr, "BPP1: %d\n", fb_vinfo->bits_per_pixel);




  
#if 0
  cmap = cmap_bw(256);
#else
  cmap = cmap_color(256);
#endif

  old_cmap.len = 256;
  old_cmap.start = 0;
  old_cmap.red = malloc(256 * 2);
  old_cmap.blue = malloc(256 * 2);
  old_cmap.green = malloc(256 * 2);
  old_cmap.transp = malloc(256 * 2);
  res = ioctl(p->fbdev_fd, FBIOGETCMAP, &old_cmap);
  if (res) {
    fprintf(stderr, "can't get cmap: %s\n", strerror(errno));
    close(p->fbdev_fd);
    free(p);

    return -1;
  }
  res = ioctl(p->fbdev_fd, FBIOPUTCMAP, cmap);
  if (res) {
    fprintf(stderr, "can't put cmap: %s\n", strerror(errno));
    close(p->fbdev_fd);
    free(p);

    return -1;
  }
  free(cmap->red);
  free(cmap->green);
  free(cmap->blue);
  free(cmap);

  return 0;
}

#endif

void events_update(void)
{
}
void pbar_update(void *c, float l)
{
}

int file_end(void)
{
  return end;
}

void file_set_end(void)
{
  end = 1;
}

void frame_display(void *c1, int w, int h, unsigned char *b, int id)
{
  int i;
  struct controls *c = c1;
#if 0 /*ndef FULLCOLOR*/
  if (c->bpp * 8 != img->Depth) {
    fprintf(stderr, "(%d != %d) Screen BPP != Img BPP!!! :(\n",
		    c->bpp, img->Depth);

    exit(-1);
  }
#endif
  
  for (i = 0; i < h; i++) {
//fprintf(stderr, "memcpy(%d %d %p %p --- %d * %d = %d)\n", id, c->fb_line_len, c->b, b, w, c->bpp, w * c->bpp);
    memcpy(c->fb_line_len  / 2 * id + c->b + i * c->fb_line_len, b + i * w * c->bpp, w * c->bpp);
  }
}

void *window_prepare(int w, int h)
{
  int res;
  struct fb_var_screeninfo fb_vinfo;
  struct fb_fix_screeninfo fb_finfo;
  struct controls *p;

  p = malloc(sizeof(struct controls));
  if (p == NULL) {
    return NULL;
  }
  
  p->fbdev_fd = open(fbdevname, O_RDWR);
  if (p->fbdev_fd == -1) {
    perror("Cannot open FB device");
    free(p);
    
    return NULL;
  }
  
  res = ioctl(p->fbdev_fd, FBIOGET_VSCREENINFO, &fb_vinfo);
  if (res < 0) {
    perror("Error opening fbdev");
    close(p->fbdev_fd);
    free(p);
    
    return NULL;
  }
  
#if 0
  p->fbtty_fd = open("dev/tty", O_RDWR);
  if (fbtty_fd < 0) {
    perror("Error opening /dev/tty");
    
    return NULL;
  }
#endif
  fprintf(stderr, "Bits per Pixel: %d\n", fb_vinfo.bits_per_pixel);
  fprintf(stderr, "Red.Len: %d\n", fb_vinfo.red.length);
  fprintf(stderr, "Blue.Len: %d\n", fb_vinfo.blue.length);
  fprintf(stderr, "Green.Len: %d\n", fb_vinfo.green.length);
  
  res = ioctl(p->fbdev_fd, FBIOGET_FSCREENINFO, &fb_finfo);
  if (res < 0) {
    perror("Can't get FSCREENINFO");
    close(p->fbdev_fd);
    free(p);

    return NULL;
  }

  res = color_setup(p, &fb_vinfo);
  if (res < 0) {
    return NULL;
  }
 
  p->fb_line_len = fb_finfo.line_length;
  p->fb_size = fb_finfo.smem_len;
  p->bpp = fb_vinfo.bits_per_pixel / 8;				/* CHECKME! */
  p->b = NULL;
  p->b = (unsigned char *)mmap(0, p->fb_size, PROT_READ | PROT_WRITE,
		  MAP_SHARED, p->fbdev_fd, 0);
  if (p->b == (unsigned char *) -1) {
    perror("MMAP");
    close(p->fbdev_fd);
    free(p);
    
    return NULL;
  }

#if 0
  center = frame_buffer + (out_width - in_width) * fb_pixel_size /
	  2 + ( (out_height - in_height) / 2 ) * fb_line_len +
	  x_offset * fb_pixel_size + y_offset * fb_line_len;
#endif
  printf("frame_buffer @ %p\n", p->b);
#if 0
  printf("center @ %p\n", center);
  printf("pixel per line: %d\n", fb_line_len / fb_pixel_size);
  memset(frame_buffer, '\0', fb_line_len * fb_yres);
#endif

  return p;
}

void output_init(int *argc, char **argv[])
{
}
