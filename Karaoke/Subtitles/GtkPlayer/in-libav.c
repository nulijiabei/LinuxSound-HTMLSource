#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "video-in.h"

static enum PixelFormat pix_fmt;

static struct SwsContext *rescaler_context(AVCodecContext *c)
{
  int w, h;

  w = c->width;
  h = c->height;
  return sws_getContext(w, h, c->pix_fmt, w, h, pix_fmt,
                        SWS_BICUBIC, NULL, NULL, NULL);
}

void *file_open(const char *fname)
{
  int res, i;
  AVFormatContext *avctx;
  AVCodec *codec;

  av_register_all();

  pix_fmt = PIX_FMT_RGB32;
  pix_fmt = PIX_FMT_RGB24;
  
  res = av_open_input_file(&avctx, fname, NULL, 0, NULL);
  if (res < 0) {
    fprintf(stderr, "Cannot open input file %s: %d\n", fname, res);

    return NULL;
  }

  res = av_find_stream_info(avctx);
  if (res < 0) {
    fprintf(stderr, "Cannot find stream information: %d\n", res);
    /* FIXME: Close the input file? */

    return NULL;
  }
  dump_format(avctx, 1, fname, 0);

  for (i = 0; i < avctx->nb_streams; i++) {
    codec = avcodec_find_decoder(avctx->streams[i]->codec->codec_id);
    if (!codec) {
      fprintf(stderr, "Cannot find codec %d (stream %d)\n",
                      avctx->streams[i]->codec->codec_id, i);
      /* FIXME: Close the input file? */

      return NULL;
    }

    res = avcodec_open(avctx->streams[i]->codec, codec);
    if (res < 0) {
      fprintf(stderr, "Cannot open codec %d (stream %d)\n",
                      avctx->streams[i]->codec->codec_id, i);
      /* FIXME: Close the input file? */

      return NULL;
    }
  }

  return avctx;
}

uint8_t *frame_get(void *h)
{
  AVFormatContext *ctx = h;
  int res = 1, decoded;
  AVPacket pkt;
  static struct SwsContext *swsctx;
  static AVFrame pic, rgb;

  av_init_packet(&pkt);

  while (res >= 0) {
    res = av_read_frame(ctx, &pkt);
    if (res >= 0) {
      if (ctx->streams[pkt.stream_index]->codec->codec_type == CODEC_TYPE_VIDEO) {
        avcodec_get_frame_defaults(&pic);
        res = avcodec_decode_video(ctx->streams[pkt.stream_index]->codec,
                                   &pic, &decoded, pkt.data, pkt.size);
        if (res < 0) {
          return NULL;
        }
        if (decoded) {
          if (swsctx == NULL) {
            swsctx = rescaler_context(ctx->streams[pkt.stream_index]->codec);
            avcodec_get_frame_defaults(&rgb);
            avpicture_alloc((AVPicture*)&rgb, pix_fmt,
                            ctx->streams[pkt.stream_index]->codec->width,
                            ctx->streams[pkt.stream_index]->codec->height);
          }
          if (swsctx) {
            sws_scale(swsctx, pic.data, pic.linesize, 0,
                      ctx->streams[pkt.stream_index]->codec->height,
                      rgb.data, rgb.linesize);
            return rgb.data[0];
          }
        }
      }
    }
  }

  return NULL;
}

void frame_size(void *handle, int *w, int *h)
{
  int i;
  AVFormatContext *ctx = handle;

  *w = *h = 0;
  for (i = 0; i < ctx->nb_streams; i++) {
    if (ctx->streams[i]->codec->codec_type == CODEC_TYPE_VIDEO) {
      *w = ctx->streams[i]->codec->width;
      *h = ctx->streams[i]->codec->height;
    }
  }
}

void file_close(void *h)
{
  av_close_input_file(h);
}
