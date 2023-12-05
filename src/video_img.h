//
// Created by chengrui on 23-12-5.
//
#ifndef FFMPEG_DEV_VIDEO_IMG_H
#define FFMPEG_DEV_VIDEO_IMG_H

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
}
#endif

class VideoImgHelper {
public:
    explicit VideoImgHelper(const char* filePath);

    int init_context();

    int print_video_info();

    int frame_to_img_buf(AVFrame* frame, enum AVCodecID codecID, uint8_t* outbuf, size_t outSize);

    int frame_to_img_file(AVFrame* frame, enum AVCodecID codecID, const char* outPath);

    AVFrame* get_video_first_frame();

    AVFrame* get_video_frame_by_index(int video_stream_index);

    ~VideoImgHelper();

private:
    AVFormatContext *formatContext{};
    const char* videoFilePath;
};

#endif //FFMPEG_DEV_VIDEO_IMG_H