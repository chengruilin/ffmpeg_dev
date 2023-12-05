#include <iostream>
#include <string>
#include <cstdint>
#include <cstdio>

#ifdef __cplusplus
extern "C" {
#endif
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#ifdef __cplusplus
};
#endif

static int frame2ImgFile(AVFrame* frame, enum AVCodecID codecID, uint8_t* outbuf, size_t outSize) {
    int ret = 0;

    AVPacket pkt;
    AVCodecContext* ctx = nullptr;
    AVFrame * rgbFrame = nullptr;
    uint8_t* buffer = nullptr;

    struct SwsContext* swsCtx = nullptr;
    av_init_packet(&pkt);

    const AVCodec* codec = avcodec_find_encoder(codecID);
    if (!codec) {
        std::cerr << "codec not found" << std::endl;
        goto end;
    }

    if (!codec->pix_fmts) {
        std::cerr << "codec pix fmts not found" << std::endl;
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
        std::cerr << "codec open failed" << std::endl;
        goto end;
    }

    if (frame->format != ctx->pix_fmt) {
        rgbFrame = av_frame_alloc();
        if (rgbFrame == nullptr) {
            std::cerr << "rgb frame alloc failed" << std::endl;
            goto end;
        }
        swsCtx = sws_getContext(frame->width, frame->height, (AVPixelFormat)frame->format,
                                frame->width, frame->height, ctx->pix_fmt,
                                SWS_FAST_BILINEAR, nullptr, nullptr, nullptr);
        if (!swsCtx) {
            std::cerr << "sws get context failed" << std::endl;
            goto end;
        }
        int bufferSize = av_image_get_buffer_size(ctx->pix_fmt, frame->width, frame->height, 1) * 2;
        buffer = (unsigned char*)av_malloc(bufferSize);
        if (buffer == nullptr) {
            std::cerr << "buffer alloc failed" << std::endl;
            goto end;
        }
        av_image_fill_arrays(rgbFrame->data, rgbFrame->linesize, buffer, ctx->pix_fmt, frame->width, frame->height, 1);
        if ((ret = sws_scale(swsCtx, frame->data, frame->linesize, 0, frame->height, rgbFrame->data, rgbFrame->linesize)) < 0) {
            std::cerr << "sws scale failed" << std::endl;
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
        std::cerr << "codec send frame failed" << std::endl;
        goto end;
    }
    ret = avcodec_receive_packet(ctx, &pkt);
    if (ret < 0) {
        std::cerr << "codec receive packet failed" << std::endl;
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
    return ret;
}


int main() {
    std::cout << "Hello, World!" << std::endl;

    std::cout << "avformat network init" << std::endl;
    avformat_network_init();

    AVFormatContext *formatContext = nullptr;
    if (avformat_open_input(&formatContext, "/home/chengrui/Documents/code/ffmpeg-dev/videos/test.mp4", nullptr, nullptr) != 0) {
        std::cout << "avformat open input failed" << std::endl;
        return -1;
    }

    if (avformat_find_stream_info(formatContext, NULL) < 0) {
        std::cout << "avformat find stream info failed" << std::endl;
        avformat_close_input(&formatContext);
        return -1;
    }

    int video_stream_index = -1;
    for (int i = 0; i < formatContext->nb_streams; ++i) {
        AVStream *stream = formatContext->streams[i];
        if (stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            std::cout << "video stream index: " << i << std::endl;
            std::cout << "video codec id: " << stream->codecpar->codec_id << std::endl;
            std::cout << "video width: " << stream->codecpar->width << std::endl;
            std::cout << "video height: " << stream->codecpar->height << std::endl;
            std::cout << "video frame rate: " << stream->avg_frame_rate.num << "/" << stream->avg_frame_rate.den << std::endl;
            std::cout << "video bit rate: " << stream->codecpar->bit_rate << std::endl;
            std::cout << "video codec name: " << avcodec_get_name(stream->codecpar->codec_id) << std::endl;
            video_stream_index = i;
        } else if (stream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            std::cout << "audio stream index: " << i << std::endl;
            std::cout << "audio codec id: " << stream->codecpar->codec_id << std::endl;
            std::cout << "audio sample rate: " << stream->codecpar->sample_rate << std::endl;
            std::cout << "audio bit rate: " << stream->codecpar->bit_rate << std::endl;
            std::cout << "audio codec name: " << avcodec_get_name(stream->codecpar->codec_id) << std::endl;
        }
    }

    if (video_stream_index == -1) {
        std::cerr << "video stream not found" << std::endl;
        avformat_close_input(&formatContext);
        return -1;
    }

    AVCodecParameters *codecParameters = formatContext->streams[video_stream_index]->codecpar;
    const AVCodec* codec = avcodec_find_decoder(codecParameters->codec_id);
    if (!codec) {
        std::cerr << "codec not found" << std::endl;
        avformat_close_input(&formatContext);
        return -1;
    }

    AVCodecContext* codecContext = avcodec_alloc_context3(codec);
    if (!codecContext) {
        std::cerr << "codec context alloc failed" << std::endl;
        avformat_close_input(&formatContext);
        return -1;
    }

    if (avcodec_parameters_to_context(codecContext, codecParameters) < 0) {
        std::cerr << "codec parameters to context failed" << std::endl;
        avcodec_free_context(&codecContext);
        avformat_close_input(&formatContext);
        return -1;
    }

    if (avcodec_open2(codecContext, codec, nullptr) < 0) {
        std::cerr << "codec open failed" << std::endl;
        avcodec_free_context(&codecContext);
        avformat_close_input(&formatContext);
        return -1;
    }

    AVPacket packet;
    av_init_packet(&packet);

    AVFrame *frame = av_frame_alloc();
    int frameNumber = 0;
    while (av_read_frame(formatContext, &packet) >= 0) {
        if (packet.stream_index == video_stream_index) {
            int ret = avcodec_send_packet(codecContext, &packet);
            if (ret < 0 && ret != AVERROR(EAGAIN)) {
                break;
            }
            while (ret >= 0) {
                ret = avcodec_receive_frame(codecContext, frame);
                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                    break;
                } else if (ret < 0) {
                    std::cerr << "decode error" << std::endl;
                    break;
                }
                // handler frame
                std::cout << "frame to image start" << frame->width << std::endl;

                int bufSize = av_image_get_buffer_size(AV_PIX_FMT_BGRA, frame->width, frame->height, 64);
                uint8_t* outbuf = (uint8_t*)av_malloc(bufSize);
                int picSIze = frame2ImgFile(frame, AV_CODEC_ID_MJPEG, outbuf, bufSize);

                std::string fileName = "./text" + std::to_string(frameNumber) + ".jpg";
                FILE* f = fopen(fileName.c_str(), "wb+");
                if (f) {
                    fwrite(outbuf, 1, picSIze, f);
                    fclose(f);
                }
                av_free(outbuf);

                ++frameNumber;
                if (frameNumber > 1) {
                    break;
                }
            }
            if (frameNumber > 1) {
                break;
            }
        }
    }
    av_frame_free(&frame);
    //avcodec_free_context(&formatContext);

    avformat_close_input(&formatContext);
    std::cout << "avformat network deinit" << std::endl;
    avformat_network_deinit();

    return 0;
}

