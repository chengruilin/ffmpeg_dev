#include "video_img.h"

int main() {

    const char* videoPath = "/home/chengrui/Documents/code/ffmpeg-dev/videos/test.mp4";

    VideoImgHelper imgHelper = VideoImgHelper(videoPath);

    int ret = imgHelper.init_context();
    if (ret > 0) {
        imgHelper.print_video_info();
        AVFrame *frame = imgHelper.get_video_first_frame();
        imgHelper.frame_to_img_file(frame, AV_CODEC_ID_MJPEG, "/home/chengrui/Documents/code/ffmpeg-dev/videos/test.jpeg");

        AVFrame *frame2 = imgHelper.get_video_frame_by_index(100);
        imgHelper.frame_to_img_file(frame2, AV_CODEC_ID_MJPEG, "/home/chengrui/Documents/code/ffmpeg-dev/videos/test2.jpeg");
    }

    return 0;
}

