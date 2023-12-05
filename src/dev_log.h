//
// Created by chengrui on 23-12-5.
//

#ifndef FFMPEG_DEV_DEV_LOG_H
#define FFMPEG_DEV_DEV_LOG_H

#include <cstdio>
#include <cstring>

#define DEBUG

#define FILENAME_ (strrchr(__FILE__, '/') + 1)

#ifdef DEBUG
#define LOGD(fmt, ...) printf("[DEBUG][%s:%d] " fmt "\n", FILENAME_, __LINE__, ##__VA_ARGS__)
#define LOGE(fmt, ...) printf("[ERROR][%s:%d] " fmt "\n", FILENAME_, __LINE__, ##__VA_ARGS__)
#else
#define LOGD(fmt, ...)
#endif

#endif //FFMPEG_DEV_DEV_LOG_H
