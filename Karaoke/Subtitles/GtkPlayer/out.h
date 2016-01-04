#ifndef __MP_CORE_H__
#define __MP_CORE_H__

int file_end(void);
void *window_prepare(int w, int h);
void frame_display(void *c, int w, int h, unsigned char *b, int i);
void pbar_update(void *c, float l);
void output_init(int *argc, char **argv[]);

#endif	/* __MP_CORE_H__ */
