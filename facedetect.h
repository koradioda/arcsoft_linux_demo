#ifndef FACEDETECT_H
#define FACEDETECT_H

#include <QObject>
#include <QRunnable>
#include "inc/amcomdef.h"
#include "inc/arcsoft_face_sdk.h"
#include "inc/asvloffscreen.h"
#include "inc/merror.h"
#include <QImage>
#include <QMap>
#include <QVideoFrame>

#define NSCALE 16
#define FACENUM 5

class FaceDetect : public QObject,public QRunnable
{
    Q_OBJECT
public:
    explicit FaceDetect(QObject *parent = nullptr);

protected:
    void run();

private:
    int ColorSpaceConvert(MInt32 width, MInt32 height, MInt32 format, MUInt8 *imgData, ASVLOFFSCREEN &offscreen);
private slots:
    void recvImageData(const QImage &image,const QSize &size,const QMap<QString,ASF_FaceFeature> &fDB,const MHandle &mhandle);
    void recvVideoData(const QVideoFrame &frame,const QSize &size,const QMap<QString,ASF_FaceFeature> &fDB);
signals:
    void sendLogStr(const QString&);
    void sendCompareResult(const QString&);
    void sendDetectedData(const QImage&,const QRect&,const QString&);

private:
    MHandle mHandle=NULL;               // SDK engine
    QImage mImage;                               // need recvfrom main thread
    QSize mSize;                                // previewLabel size ,for showing image,need recvfrom main thread
    QString predict;
    MFloat score;
    QMap<QString,ASF_FaceFeature> featureDB;   // face feature base,for compare face, need recvfrom main thread

};

#endif // FACEDETECT_H
