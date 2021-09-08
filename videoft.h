#ifndef VIDEOFT_H
#define VIDEOFT_H

#include <QObject>
#include "inc/amcomdef.h"
#include "inc/arcsoft_face_sdk.h"
#include "inc/asvloffscreen.h"
#include "inc/merror.h"
#include <QThread>
#include <QImage>
#include <QMap>
#include <QtConcurrent>
#include <QThreadPool>
#include <QTimer>
#include <QSharedPointer>
#include "mutilface.h"

#define NSCALE 16
#define FACENUM 5

class videoFt : public QObject
{
    Q_OBJECT
public:
    explicit videoFt(QObject *parent = nullptr);
    ~videoFt();
private:
    MRESULT featureExtractEx(QSharedPointer<OffScreen> offscreen, QSharedPointer<MutilFace> detectedFaces,ASF_FaceFeature *newFeature);
    MInt32 livenessDetectEx(QSharedPointer<OffScreen> offscreen,QSharedPointer<MutilFace> detectedFaces);

    void threadFr(QSharedPointer<OffScreen> offscreen,QSharedPointer<MutilFace> detectedFaces);

private slots:
    void doWork(const QImage &image,const QSize &size,const QMap<QString,ASF_FaceFeature> &fDB);
    void recvDataFromCam(const QImage &image,const QSize &size,const QMap<QString,ASF_FaceFeature> &fDB);
    void closeVideoFt();
    int ColorSpaceConvert(MInt32 width, MInt32 height, MInt32 format, MUInt8 *imgData, ASVLOFFSCREEN &offscreen);
    void frameLook();
    void recvThresholdFromGui(const float &thres);
    void setThreshold();
    void recvFaceLineFromGui(const float &faceLine);

signals:
    void sendDetectedVideoData(const QImage &,const QRect &,const QString &);
    void recvedThreshold();

private :
    MHandle mVideoFtEngine = NULL;
    MHandle mImageEngine=NULL;

    // data needed recv from others
    QImage mImage;          // data need to deal
    QSize mSize;            // echo in label size
    QMap<QString,ASF_FaceFeature> featureDB;        // compare data

    int frames;   // 计算同一张脸的识别帧数，可用于后面人脸识别的判断优化
    //
    MInt32 preFaceID = -1;   // 记录人脸识别的faceID，判断faceID是否发生变化
    MInt32 frCount;         // 记录同一张脸的识别次数
    MBool idChanged = false;        // 标识人脸框的脸是否变化
    // result
    QString detectData;     // 保存识别后的结果
    QString predict;        // 保存人脸库对比后的结果
    float score;            // 识别得分
    // frame test
    QTimer *timer;
    // liveness and face compare threshold
    ASF_LivenessThreshold mThreshold={0.5,0.7,0.65};   // 活体识别相关阈值
    MFloat mFaceLine = 0.7;         // 人脸识别阈值


};

#endif // VIDEOFT_H
