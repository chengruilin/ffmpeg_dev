//
// Created by chengrui on 23-12-8.
//

#include "mmap_utils.h"
#include "dev_log.h"
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <sys/mman.h>
#include <sys/stat.h>

MMapUtils::MMapUtils() {

}

MMapUtils::~MMapUtils() {

}

int MMapUtils::wirte_to_mmap(const char *path, const char *content) {

    int fd = open(path, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (fd < 0) {
        LOGD("open file failed");
        return -1;
    }

    struct stat st{};
    if (fstat(fd, &st) == -1) {
        LOGD("fstat failed");
        return -1;
    }

    size_t dataSize = strlen(content);
    //off_t size = lseek(fd, 0, SEEK_END);
    lseek(fd, dataSize - 1, SEEK_END);
    write(fd, "", 1);
    char *buffer = (char *) mmap(nullptr, dataSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (buffer == MAP_FAILED) {
        LOGD("mmap failed");
        close(fd);
        return -1;
    }

    close(fd);

    memcpy(buffer, content, dataSize);
    //if (msync(buffer, size + dataSize, MS_SYNC) == -1) {
    //    LOGD("msync failed");
    //    munmap(buffer, size);
    //    close(fd);
    //    return -1;
    //}

    if (munmap(buffer, dataSize) == -1) {
        LOGD("munmap failed");
        close(fd);
        return -1;
    }

    close(fd);
    return 0;
}