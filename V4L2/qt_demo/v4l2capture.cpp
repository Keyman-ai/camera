#include "v4l2capture.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <cstring>
#include <iostream>

V4L2Capture::V4L2Capture(const std::string &device){
    : device_(device), fd_(-1), isOpened_(false){}
}

V4L2Capture::~V4L2Capture(){
    close();
}

bool V4L2Capture::open(){
    fd_ = ::open(device_.c_str(), O_RDWR | O_NONBLOCK, 0);
    if(fd_ < 0){
        std::cerr << "Failed to open device " << device_ << std::endl;
        return false;
    }

    isOpened_ = true;
    return true;
}

void V4L2Capture::close(){
    if(isOpened_){
        ::close(fd_);
        isOpened_ = false;
    }
}

bool V4L2Capture::init(){
    //set format
    std::memset(&format_, 0, sizeof(format_));
    format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    format.fmt.pix.width = 640;
    format.fmt.pix.height = 480;
    format.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
    format.fmt.pix.field = V4L2_FIELD_INTERLACED;

    if(ioctl(fd_, VIDIOC_S_FMT, &format) < 0){
        std::cerr << "Failed to set format" << std::endl;
        return false;
    }

    //request buffers
    v4l2_requestbuffers reqBuffers;
    std::memset(&reqBuffers, 0, sizeof(reqBuffers));
    reqBuffers.count = 4;
    reqBuffers.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    reqBuffers.memory = V4L2_MEMORY_MMAP;

    if(ioctl(fd_, VIDIOC_REQBUFS, &reqBuffers) < 0){
        std::cerr << "Failed to request buffers" << std::endl;
        return false;
    }

    buffers_.resize(reqBuffers.count);

    for(int i = 0; i < reqBuffers.count; ++i){
        std::memset(&buffer_, 0, sizeof(buffers_));
        buffer_.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buffer_.memory = V4L2_MEMORY_MMAP;
        buffer_.index = i;

        if (ioctl(fd_, VIDIOC_QUERYBUF, &buffer_) == -1) {
            std::cerr << "Failed to query buffer" << std::endl;
            return false;
        }

        buffers_[i] = mmap(nullptr, buffer_.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, buffer_.m.offset);
        if (buffers_[i] == MAP_FAILED) {
            std::cerr << "Failed to mmap buffer" << std::endl;
            return false;
        }
    }

    return true;
}

bool V4L2Capture::start(){
    for(int i = 0; i < buffers_.size(); ++i){
        std::memset(&buffer_, 0, sizeof(buffer_));
        buffer_.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buffer_.memory = V4L2_MEMORY_MMAP;
        buffer_.index = i;

        if(ioctl(fd_, VIDIOC_QBUF, &buffer_) == -1){
            std::cerr << "Failed to queue buffer" << std::endl;
            return false;
        }
    }

    v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if(ioctl(fd_, VIDIOC_STREAMON, &type) == -1){
        std::cerr << "Failed to start stream" << std::endl;
        return false;
    }

    return true;
}

void V4L2Capture::stop(){
    v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ioctl(fd_, VIDIOC_STREAMOFF, &type);
}

bool V4L2Capture::captureFrame(std::vector<uint8_t> &frameData, int &width, int &height){
    std::memset(&buffer_, 0, sizeof(buffer_));
    buffer_.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buffer_.memory = V4L2_MEMORY_MMAP;

    if(ioctl(fd_, VIDIOC_DQBUF, &buffer_) == -1){
        std::cerr << "Failed to dequeue buffer" << std::endl;
        return false;
    }

    frameData.assign(static_cast<uint8_t*>(buffers_[buffer_.index]), static_cast<uint8_t*>(buffers_[buffer_.index]) + buffer_.length);

    if(ioctl(fd_, VIDIOC_QBUF, &buffer_) == -1){
        std::cerr << "Failed to queue buffer" << std::endl;
        return false;   
    }

    width = format_.fmt.pix.width;
    height = format_.fmt.pix.height;

    return true;
}
