/*
    捕获来自 /dev/video0 的一帧YUYV格式图片，并保存为 pic.yuv 文件
    编译和运行: 确保你有安装了V4L2库，可以用以下命令编译并运行这个程序：
    gcc -o capture example_capture.c -lv4l2
    ./capture
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <sys/mman.h>

#define WIDTH 640
#define HEIGHT 480
int main()
{
    inf fd = open("/dev/video0", O_RDWR);
    if (fd == -1) {
        perror("Opening video device");
        return 1;
    }

    struct v4l2_capability_cap;
    if(ioctl(fd, VIDIOC_QUERYCAP, &cap) == -1){
        perror("Querying Capabilities");
        return 1;
    }

    struct v4l2_format fmt;
    memset(&fmt, 0, sizeof(fmt));
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = WIDTH;
    fmt.fmt.pix.height = HEIGHT;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
    fmt.fmt.pix.field = V4L2_FIELD_NONE;
    if(ioctl(fd, VIDIOC_S_FMT, &fmt) == -1){
        perror("Setting Pixel Format");
        return 1;
    }

    struct buffer* buffers = calloc(req.count, sizeof(*buffers));
    struct v4l2_buffer buf;
    memset(&buf, 0, sizeof(buf));
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = 0;
    if(ioctl(fd, VIDIOC_QUERYBUF, &buf) == -1){
        perror("Querying Buffer");
        return 1;
    }  

    buffers[0].length = buf.length;
    buffers[0].start = mmap(NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buf.m.offset);
    if (buffers[0].start == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    memset(&buf, 0, sizeof(buf));
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = 0;
    if (ioctl(fd, VIDIOC_QBUF, &buf) == -1) {
        perror("Query Buffer");
        return 1;
    }

    if (ioctl(fd, VIDIOC_STREAMON, &buf.type) == -1) {
        perror("Start Capture");
        return 1;
    }

    memset(&buf, 0, sizeof(buf));
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    if (ioctl(fd, VIDIOC_DQBUF, &buf) == -1) {
        perror("Retrieve Frame");
        return 1;
    }

    FILE *file = fopen("pic.yuv", "wb");
    if (!file) {
        perror("Saving image");
        return 1;
    }
    fwrite(buffers[0].start, buf.bytesused, 1, file);
    fclose(file);

     if (ioctl(fd, VIDIOC_STREAMOFF, &buf.type) == -1) {
        perror("Stop Capture");
        return 1;
    }

    munmap(buffers[0].start, buffers[0].length);
    free(buffers);
    close(fd);

    return 0;

}
