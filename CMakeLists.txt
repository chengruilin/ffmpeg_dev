cmake_minimum_required(VERSION 3.22.1)
project(ffmpeg_dev)

include_directories(${CMAKE_SOURCE_DIR}/include)
link_directories(${CMAKE_SOURCE_DIR}/lib)

set(CMAKE_CXX_STANDARD 14)

#aux_source_directory(./src SOURCE_FILES)
FILE(GLOB_RECURSE SOURCE_FILES ./src/*.cpp)
foreach(SOURCE_FILE ${SOURCE_FILES})
    message(${SOURCE_FILE})
endforeach(SOURCE_FILE)

FILE(GLOB_RECURSE HEADER_FILES ./src/*.h)
foreach(HEADER_FILE ${HEADER_FILES})
    message(${HEADER_FILE})
endforeach(HEADER_FILE)

add_executable(ffmpeg_dev ${SOURCE_FILES})
target_link_libraries(ffmpeg_dev avutil swresample avcodec avfilter swscale avformat avdevice)


