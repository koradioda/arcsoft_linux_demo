#ifndef LIVENESSRECOGNIZE_H
#define LIVENESSRECOGNIZE_H

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


class LivenessRecognize : public QObject,public QRunnable
{
    Q_OBJECT
public:
    explicit LivenessRecognize(QObject *parent = nullptr);
protected:
    void run();
private :
    int ColorSpaceConvert(MInt32 width, MInt32 height, MInt32 format, MUInt8 *imgData, ASVLOFFSCREEN &offscreen);

private slots:
    void recvImageData(const QImage &image,const QSize &size,const QMap<QString,ASF_FaceFeature> &fDB,const MHandle &mhandle);
    void recvVideoData(const QVideoFrame &frame,const QSize &size,const QMap<QString,ASF_FaceFeature> &fDB);
signals:
    void sendDetectedData(const QString&);
    void sendLogStr(const QString&);
    void sendLrResult(const QString&);

private:
    MHandle mHandle=NULL;               // SDK Engine
    QImage mImage;                       // need pass by
    QSize mSize;                        // need pass by
    QMap<QString,ASF_FaceFeature> featureDB;   // face feature base,for compare face, need recvfrom main thread
};

#endif // LIVENESSRECOGNIZE_H
