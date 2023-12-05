//
// Created by chengrui on 23-12-5.
//
#include "video_img.h"
#include "dev_log.h"

VideoImgHelper::VideoImgHelper(const char *path) {
    videoFilePath = path;
    formatContext = nullptr;
    LOGD("VideoImgHelper constructor");
}

VideoImgHelper::~VideoImgHelper() {
    LOGD("VideoImgHelper destructor");
    if (formatContext != nullptr) {
        avformat_close_input(&formatContext);
        LOGD("avformat close formatContext");
    }
    avformat_network_deinit();
    LOGD("avformat network deinit");
}

int VideoImgHelper::init_context() {
    FILE* file = fopen(videoFilePath, "rb");
    if (!file) {
        LOGE("video file not found");
        return -1;
    }

    avformat_network_init();

    if (avformat_open_input(&formatContext, videoFilePath, nullptr, nullptr) != 0) {
        LOGD("avformat open input failed");
        return -1;
    }
    LOGD("avformat network init.");

    if (avformat_find_stream_info(formatContext, nullptr) < 0) {
        LOGD("avformat find stream info failed");
        return -1;
    }

    return 1;
}

int VideoImgHelper::print_video_info() {
    for (int i = 0; i < formatContext->nb_streams; ++i) {
        AVStream *stream = formatContext->streams[i];
        if (stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            LOGD("video stream index: %d", i);
            LOGD("video codec id: %d", stream->codecpar->codec_id);
            LOGD("video width: %d", stream->codecpar->width);
            LOGD("video height: %d", stream->codecpar->height);
            LOGD("video frame rate: %d/%d", stream->avg_frame_rate.num, stream->avg_frame_rate.den);
            LOGD("video bit rate: %ld", stream->codecpar->bit_rate);
            LOGD("video codec name:%s ", avcodec_get_name(stream->codecpar->codec_id));
        } else if (stream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            LOGD("audio stream index: %d", i);
            LOGD("audio codec id: %d", stream->codecpar->codec_id);
            LOGD("audio sample rate: %d", stream->codecpar->sample_rate);
            LOGD("audio bit rate: %ld", stream->codecpar->bit_rate);
            LOGD("audio codec name: %s", avcodec_get_name(stream->codecpar->codec_id));
        }
    }
    return 1;
}

int VideoImgHelper::frame_to_img_buf(AVFrame* frame, enum AVCodecID codecID, uint8_t* outbuf, size_t outSize) {
    int ret = 0;

    AVPacket pkt;
    AVCodecContext* ctx = nullptr;
    AVFrame * rgbFrame = nullptr;
    uint8_t* buffer = nullptr;

    struct SwsContext* swsCtx = nullptr;
    av_init_packet(&pkt);

    const AVCodec* codec = avcodec_find_encoder(codecID);
    if (!codec) {
        LOGD("codec not found");
        goto end;
    }

    if (!codec->pix_fmts) {
        LOGD("codec pix fmts not found");
        goto end;
    }

    ctx = avcodec_alloc_context3(codec);
    ctx->bit_rate = 3000000;
    ctx->width = frame->width;
    ctx->height = frame->height;
    ctx->time_base.num = 1;
    ctx->time_base.den = 25;
    ctx->max_b_frames = 0;
    ctx->thread_count = 1;
    ctx->pix_fmt = *codec->pix_fmts;
    ret = avcodec_open2(ctx, codec, nullptr);

    if (ret < 0) {
        LOGD("codec open failed");
        goto end;
    }

    if (frame->format != ctx->pix_fmt) {
        rgbFrame = av_frame_alloc();
        if (rgbFrame == nullptr) {
            LOGD("rgb frame alloc failed");
            goto end;
        }
        swsCtx = sws_getContext(frame->width, frame->height, (AVPixelFormat)frame->format,
                                frame->width, frame->height, ctx->pix_fmt,
                                SWS_FAST_BILINEAR, nullptr, nullptr, nullptr);
        if (!swsCtx) {
            LOGD("sws get context failed");
            goto end;
        }
        int bufferSize = av_image_get_buffer_size(ctx->pix_fmt, frame->width, frame->height, 1) * 2;
        buffer = (unsigned char*)av_malloc(bufferSize);
        if (buffer == nullptr) {
            LOGD("buffer alloc failed");
            goto end;
        }
        av_image_fill_arrays(rgbFrame->data, rgbFrame->linesize, buffer, ctx->pix_fmt, frame->width, frame->height, 1);
        if ((ret = sws_scale(swsCtx, frame->data, frame->linesize, 0, frame->height, rgbFrame->data, rgbFrame->linesize)) < 0) {
            LOGD("sws scale failed");
            goto end;
        }
        rgbFrame->format = ctx->pix_fmt;
        rgbFrame->width = frame->width;
        rgbFrame->height = frame->height;
        ret = avcodec_send_frame(ctx, rgbFrame);
    }
    else {
        ret = avcodec_send_frame(ctx, frame);
    }
    if (ret < 0) {
        LOGD("codec send frame failed");
        goto end;
    }
    ret = avcodec_receive_packet(ctx, &pkt);
    if (ret < 0) {
        LOGD("codec receive packet failed");
        goto end;
    }
    if (pkt.size > 0 && pkt.size <= outSize) {
        memcpy(outbuf, pkt.data, pkt.size);
    }
    ret = pkt.size;
    end:
    if (swsCtx) {
        sws_freeContext(swsCtx);
    }
    if (rgbFrame) {
        av_frame_unref(rgbFrame);
        av_frame_free(&rgbFrame);
    }
    if (buffer) {
        av_free(buffer);
    }
    av_packet_unref(&pkt);
    if (ctx) {
        avcodec_close(ctx);
        avcodec_free_context(&ctx);
    }
    if (frame) {
        av_frame_unref(frame);
        av_frame_free(&frame);
    }
    return ret;
}

int VideoImgHelper::frame_to_img_file(AVFrame* frame, enum AVCodecID codecID, const char* outPath) {
    int bufSize = av_image_get_buffer_size(AV_PIX_FMT_BGRA, frame->width, frame->height, 64);
    auto* buf = (uint8_t*)av_malloc(bufSize);
    int picSize = frame_to_img_buf(frame, codecID, buf, bufSize);
    FILE* f = fopen(outPath, "wb+");
    if (f) {
        fwrite(buf, 1, picSize, f);
        fclose(f);
    }
    av_free(buf);
    return 1;
}

AVFrame* VideoImgHelper::get_video_first_frame() {
    return get_video_frame_by_index(0);
}

AVFrame* VideoImgHelper::get_video_frame_by_index(int video_stream_index) {
    int firstVideoStreamIndex = -1;
    for (int i = 0; i < formatContext->nb_streams; ++i) {
        AVStream* stream = formatContext->streams[i];
        if (stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            firstVideoStreamIndex = i;
            break;
        }
    }

    if (firstVideoStreamIndex == -1) {
        LOGD("video stream not found");
        return nullptr;
    }

    AVCodecParameters *codecParameters = formatContext->streams[firstVideoStreamIndex]->codecpar;
    const AVCodec* codec = avcodec_find_decoder(codecParameters->codec_id);
    if (!codec) {
        LOGD("codec not found");
        return nullptr;
    }

    AVCodecContext* codecContext = avcodec_alloc_context3(codec);
    if (!codecContext) {
        LOGD("codec context alloc failed");
        return nullptr;
    }

    if (avcodec_parameters_to_context(codecContext, codecParameters) < 0) {
        LOGD("codec parameters to context failed");
        avcodec_free_context(&codecContext);
        return nullptr;
    }

    if (avcodec_open2(codecContext, codec, nullptr) < 0) {
        LOGD("codec open failed");
        avcodec_free_context(&codecContext);
        return nullptr;
    }

    AVPacket packet;
    av_init_packet(&packet);

    AVFrame* frame = av_frame_alloc();
    int framwNo = 0;
    while (av_read_frame(formatContext, &packet) >= 0) {
        if (packet.stream_index == firstVideoStreamIndex) {
            int ret = avcodec_send_packet(codecContext, &packet);
            if (ret < 0 || ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                LOGD("avcodec send packet failed, ret=%d", ret);
                av_packet_unref(&packet);
                continue;
            }

            ret = avcodec_receive_frame(codecContext, frame);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                av_packet_unref(&packet);
                continue;
            } else if (ret < 0) {
                LOGD("avcodec receive frame failed, ret=%d", ret);
                av_packet_unref(&packet);
                break;
            }

            if (++framwNo == video_stream_index + 1) {
                LOGD("get %d frame success", video_stream_index);
                break;
            }
            av_packet_unref(&packet);
        }
        av_packet_unref(&packet);
    }
    avcodec_close(codecContext);
    return frame;
}



