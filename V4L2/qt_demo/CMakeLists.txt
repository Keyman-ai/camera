cmake_minimum_required(VERSION 3.10)
project(V4L2QtCapture)

set(CMAKE_CXX_STANDARD 11)

find_package(Qt5Widgets REQUIRED)

add_executable(V4L2QtCapture main.cpp v4l2capture.cpp)

target_link_libraries(V4L2QtCapture Qt5::Widgets)
