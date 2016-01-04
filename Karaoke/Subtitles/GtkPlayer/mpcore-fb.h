#ifndef __MP_CORE_H__
#define __MP_CORE_H__

int file_end(void);
void file_set_end(void);
FILE *file_open(char *fname, ImageDesc *img);
void *window_prepare(ImageDesc *img);
void frame_display(void *c, ImageDesc *img, unsigned char *b, int i);

void events_update(void);
void pbar_update(void *c, float l);

#define output_init(a, b)
#endif	/* __MP_CORE_H__ */
