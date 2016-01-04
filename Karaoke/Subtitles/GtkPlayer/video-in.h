
void *file_open(const char *fname);
uint8_t *frame_get(void *h);
void frame_size(void *handle, int *w, int *h);
void file_close(void *h);

