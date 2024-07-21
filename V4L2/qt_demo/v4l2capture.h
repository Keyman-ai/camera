#ifndef V4L2CAPTURE_H
#define V4L2CAPTURE_H

#include <linux/videodev2.h>
#include <string>
#include <vector>

class V4L2Capture {
public:
    V4L2Capture(const std::string &device);
    ~V4L2Capture();

    bool open();
    void close();
    bool init();
    bool start();
    void stop();
    bool captureFrame(std::vector<uint8_t> &frameData, int &width, int &height);

private:
    std::string device_;
    int fd_;
    v4l2_format format_;
    v4l2_buffer buffer_;
    std::vector<void *> buffers_;
    bool isOpened_;

};

#endif // V4L2CAPTURE_H