#include <QApplication>
#include <QLabel>
#include <QImage>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>
#include "v4l2capture.h"

class CameraWidget : public QWidget {
    Q_OBJECT
public:
    CameraWidget(QWidget *parent = nullptr) : QWidget(parent), capture("/dev/video0") {
        if (!capture.open() || !capture.init() || !capture.start()) {
            throw std::runtime_error("Failed to initialize camera");
        }

        imageLabel = new QLabel(this);
        QVBoxLayout *layout = new QVBoxLayout(this);
        layout->addWidget(imageLabel);

        timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, &CameraWidget::updateFrame);
        timer->start(30); // Update frame every 30 ms
    }

    ~CameraWidget() {
        capture.stop();
        capture.close();
    }

private slots:
    void updateFrame() {
        std::vector<uint8_t> frameData;
        int width, height;
        if (capture.captureFrame(frameData, width, height)) {
            QImage img(frameData.data(), width, height, QImage::Format_YUYV);
            imageLabel->setPixmap(QPixmap::fromImage(img));
        }
    }

private:
    V4L2Capture capture;
    QLabel *imageLabel;
    QTimer *timer;
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    CameraWidget cameraWidget;
    cameraWidget.show();

    return app.exec();
}

#include "main.moc"
