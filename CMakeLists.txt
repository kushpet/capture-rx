cmake_minimum_required (VERSION 3.9)
project (capture_rx CXX)

set(CMAKE_CXX_STANDARD 11)

find_library(LIBIIO_LIBRARIES iio)
find_path(LIBIIO_INCLUDEDIR iio.h)
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${LIBIIO_INCLUDEDIR})

add_executable(capture_rx ${CMAKE_SOURCE_DIR}/main.cpp)

target_link_libraries(capture_rx LINK_PRIVATE ${LIBIIO_LIBRARIES})

