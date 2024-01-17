#include "video_img.h"
#include "dev_log.h"
#include "mmap_utils.h"
#include <unistd.h>


int video_frame_to_jpeg() {
    char* videoPath = "../videos/test.mp4";

    VideoImgHelper imgHelper = VideoImgHelper(videoPath);

    int ret = imgHelper.init_context();
    if (ret > 0) {
        imgHelper.print_video_info();
        AVFrame *frame = imgHelper.get_video_first_frame();
        imgHelper.frame_to_img_file(frame, AV_CODEC_ID_MJPEG, "../videos/test.jpeg");

        AVFrame *frame2 = imgHelper.get_video_frame_by_index(100);
        imgHelper.frame_to_img_file(frame2, AV_CODEC_ID_MJPEG, "../videos/test2.jpeg");
    }
    return 0;
}

int http_create_server() {

    return 0;
}

int http_create_client() {

    return 0;
}

int test_mmap() {
    MMapUtils mmapUtils = MMapUtils();
    mmapUtils.wirte_to_mmap("../test/test.txt", "hello world!");
    return 0;
}

int help() {
    printf("[f] video frame to jpeg\n");
    printf("[s] http create server\n");
    printf("[c] http create client\n");
    printf("[m] mmap test\n");
    printf("[h] help\n");
}

int main(int argc, char *argv[]) {
    int opt;
    while ((opt = getopt(argc, argv, "fscmh")) != -1) {
        switch (opt) {
            case 'f':
                video_frame_to_jpeg();
                break;
            case 's':

                break;
            case 'c':

                break;
            case 'm':
                test_mmap();
                break;
            case 'h':
                help();
                break;
            default:
                help();
                break;
        }
    }

    return 0;
}