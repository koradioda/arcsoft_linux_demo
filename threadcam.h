#ifndef THREADCAM_H
#define THREADCAM_H

#include <QObject>
#include <QThread>
#include <videoframecapture.h>
#include <QCamera>
#include <QCameraViewfinderSettings>
#include "inc/amcomdef.h"
#include "inc/arcsoft_face_sdk.h"
#include "inc/asvloffscreen.h"
#include "inc/merror.h"

/*##    deal some operations to control camera  ##*/

class threadCam : public QObject
{
    Q_OBJECT
public:
    explicit threadCam(QObject *parent = nullptr);
    ~threadCam();
    bool isAvailable();

private slots:
    void initCamera();
    void startCamera();
    void stopCamera();
    void capture(const QVideoFrame &frame);
    void cameraWork();
    void recvDataFromGUI(const QSize& size,const QMap<QString,ASF_FaceFeature> &fDB);
    int convert_yuv_to_rgb_pixel(int y, int u, int v);
    int convert_yuv_to_rgb_buffer(unsigned char *yuv, unsigned char *rgb, unsigned int width, unsigned int height);

signals:
    void sendCapture(const QImage &image,const QRect &rect,const QString &str);        // send captured to others
    void sendDataToVideoFt(const QImage &image,const QSize &size,const QMap<QString,ASF_FaceFeature> &fDB);
    void started();

private:
    QCamera *camera;
    VideoFrameCapture *videoSurface;
    bool openCamera = true;

    // data need emit
    QMap<QString,ASF_FaceFeature> featureDB;
    QSize mSize;

};

#endif // THREADCAM_H
