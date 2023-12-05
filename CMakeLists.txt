cmake_minimum_required(VERSION 3.22.1)
project(ffmpeg_dev)

# add_library( avcodec SHARED IMPORTED)
# add_library( avdevice SHARED IMPORTED)
# add_library( avfilter SHARED IMPORTED)
# add_library( avformat SHARED IMPORTED)
# add_library( avutil SHARED IMPORTED)
# add_library( swresample SHARED IMPORTED)
# add_library( swscale SHARED IMPORTED)
# set_target_properties( avcodec PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/lib/libavcodec.so)
# set_target_properties( avdevice PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/lib/libavdevice.so)
# set_target_properties( avfilter PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/lib/libavfilter.so)
# set_target_properties( avformat PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/lib/libavformat.so)
# set_target_properties( avutil PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/lib/libavutil.so)
# set_target_properties( swresample PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/lib/libswresample.so)
# set_target_properties( swscale PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/lib/libswscale.so)

include_directories(${CMAKE_SOURCE_DIR}/include)
message("include ${CMAKE_SOURCE_DIR}/include")
link_directories(${CMAKE_SOURCE_DIR}/lib)
message("link directories ${CMAKE_SOURCE_DIR}/lib")

set(CMAKE_CXX_STANDARD 14)
add_executable(ffmpeg_dev main.cpp)
target_link_libraries(ffmpeg_dev avutil swresample avcodec avfilter swscale avformat avdevice)


