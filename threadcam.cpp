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

void threadCam::capture(const QVideoFrame &frame)
{
    const QImage image = frame.image();                 // change frame format
    // emit sendCapture(image,QRect(),QString());
    emit sendDataToVideoFt(image,mSize,featureDB);      // 数据转发
}
