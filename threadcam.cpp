#include "threadcam.h"
#include <QMessageBox>

threadCam::threadCam(QObject *parent) : QObject(parent)
{
    initCamera();
    if(!camera->isAvailable())
    {
        QMessageBox::warning(nullptr,"Warning","camera not available!");
        return ;
    }
}

threadCam::~threadCam()
{
    emit sendCapture(QImage(),QRect(),QString());
    qDebug()<<"thread:"<<QThread::currentThread()<<" end by Cam";
}

bool threadCam::isAvailable()
{
    return camera->isAvailable();
}

void threadCam::cameraWork()        // moveToThread  then auto start eventLoop by the mechanism
{
    qDebug()<<"Camera:"<<QThread::currentThread();
    connect(videoSurface,SIGNAL(frameAvailable(QVideoFrame)),this,SLOT(capture(QVideoFrame)));  // 接受videoSurface发送的视频帧，并通过capture槽函数转发出去
    startCamera();                  // constantly operate camera
    if(!openCamera)
    {
        camera->stop();
    }
}

void threadCam::recvDataFromGUI(const QSize &size, const QMap<QString, ASF_FaceFeature> &fDB)
{
    featureDB = fDB;
    mSize = size;
}

int threadCam::convert_yuv_to_rgb_pixel(int y, int u, int v)
{
    unsigned int pixel32 = 0;
   unsigned char *pixel = (unsigned char *)&pixel32;
   int r, g, b;
   r = y + (1.370705 * (v-128));
   g = y - (0.698001 * (v-128)) - (0.337633 * (u-128));
   b = y + (1.732446 * (u-128));
   if(r > 255) r = 255;
   if(g > 255) g = 255;
   if(b > 255) b = 255;
   if(r < 0) r = 0;
   if(g < 0) g = 0;
   if(b < 0) b = 0;
   pixel[0] = r * 220 / 256;
   pixel[1] = g * 220 / 256;
   pixel[2] = b * 220 / 256;
   return pixel32;
}

int threadCam::convert_yuv_to_rgb_buffer(unsigned char *yuv, unsigned char *rgb, unsigned int width, unsigned int height)
{
    unsigned int in, out = 0;
    unsigned int pixel_16;
    unsigned char pixel_24[3];
    unsigned int pixel32;
    int y0, u, y1, v;
    for(in = 0; in < width * height * 2; in += 4) {
        pixel_16 = yuv[in + 3] << 24 | yuv[in + 2] << 16 | yuv[in + 1] <<  8 | yuv[in + 0];
        y0 = (pixel_16 & 0x000000ff);
        u  = (pixel_16 & 0x0000ff00) >>  8;
        y1 = (pixel_16 & 0x00ff0000) >> 16;
        v  = (pixel_16 & 0xff000000) >> 24;
        pixel32 = convert_yuv_to_rgb_pixel(y0, u, v);
        pixel_24[0] = (pixel32 & 0x000000ff);
        pixel_24[1] = (pixel32 & 0x0000ff00) >> 8;
        pixel_24[2] = (pixel32 & 0x00ff0000) >> 16;
        rgb[out++] = pixel_24[0];
        rgb[out++] = pixel_24[1];
        rgb[out++] = pixel_24[2];
        pixel32 = convert_yuv_to_rgb_pixel(y1, u, v);
        pixel_24[0] = (pixel32 & 0x000000ff);
        pixel_24[1] = (pixel32 & 0x0000ff00) >> 8;
        pixel_24[2] = (pixel32 & 0x00ff0000) >> 16;
        rgb[out++] = pixel_24[0];
        rgb[out++] = pixel_24[1];
        rgb[out++] = pixel_24[2];
    }
    return 0;
}

void threadCam::initCamera()
{   // init camera
    camera = new QCamera(this);                     // init camera
    videoSurface = new VideoFrameCapture;           // set a surfface to cath frame
    QCameraViewfinderSettings vfSetting;
    vfSetting.setResolution(800,600);               // set the resoluton of image
    camera->setViewfinderSettings(vfSetting);
    camera->setViewfinder(videoSurface);
}

void threadCam::startCamera()
{
    camera->start();
}

void threadCam::stopCamera()
{
    openCamera = false;
}

void threadCam::capture(const QVideoFrame &buffer)
{
//    const QImage image = frame.image();                 // change frame format
//    const QImage image;
    // emit sendCapture(image,QRect(),QString());
    QImage image;
    QVideoFrame frame(buffer);  // make a copy we can call map (non-const) on
    frame.map(QAbstractVideoBuffer::ReadOnly);
    QImage::Format imageFormat = QVideoFrame::imageFormatFromPixelFormat(
                frame.pixelFormat());
    // BUT the frame.pixelFormat() is QVideoFrame::Format_Jpeg, and this is
    // mapped to QImage::Format_Invalid by
    // QVideoFrame::imageFormatFromPixelFormat
    if (imageFormat != QImage::Format_Invalid) {
        image = QImage(frame.bits(),
                     frame.width(),
                     frame.height(),
                     // frame.bytesPerLine(),
                     imageFormat);
    } else {
        // e.g. JPEG
        char *rgb24 = new char[frame.width() * frame.height() * 3]();
        convert_yuv_to_rgb_buffer(frame.bits(), (unsigned char*)rgb24, frame.width(), frame.height());
        image = QImage((unsigned char*)rgb24, frame.width(), frame.height(), frame.width() * 3, QImage::Format_RGB888);
    }
    frame.unmap();

    emit sendDataToVideoFt(image,mSize,featureDB);      // 数据转发
}


