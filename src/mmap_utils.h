//
// Created by chengrui on 23-12-8.
//

#ifndef FFMPEG_DEV_MMAP_UTILS_H
#define FFMPEG_DEV_MMAP_UTILS_H

#include <sys/mman.h>

class MMapUtils {

public:
    explicit MMapUtils();

    ~MMapUtils();

    int wirte_to_mmap(const char* path, const char* content);
};

#endif //FFMPEG_DEV_MMAP_UTILS_H
