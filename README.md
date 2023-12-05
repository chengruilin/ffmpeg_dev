# FFMPEG Dev

## Build
Need add local ffmpeg lib path to LD_LIBRARY_PATH.

```bash
export LD_LIBRARY_PATH=${LIB_PATH}

mkdir build
cd build
cmake ..
make
./ffmpeg-dev
```