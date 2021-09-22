#ifndef ARCFACEDEMO_H
#define ARCFACEDEMO_H

#include <QMainWindow>
#include "videoframecapture.h"
#include "inc/amcomdef.h"
#include "inc/arcsoft_face_sdk.h"
#include "inc/asvloffscreen.h"
#include "inc/merror.h"
#include "facedetect.h"
#include "livenessrecognize.h"
#include <QDateTime>
#include <QString>
#include <QTimer>
#include <QLabel>
#include <QDebug>
#include <QCamera>
#include <QDir>
#include <QMap>
#include <QPaintEvent>
#include <QPainter>
#include <QEvent>
#include <QtConcurrent>
#include <QMetaType>
#include <threadcam.h>
#include "videoft.h"
#include <QSqlDatabase> // sql refer package
#include <QSqlError>
#include <QSqlQuery>
#include <facesdatabase.h>


#define APPID "Bbvyu5GeUE8eaBhyLsNcp49HW6tuPx3sqWog8i9S41Q7"
#define SDKKEY "EvvwdWPFj7XjL1siCQiSD2qGb9XanUowPRYyVML8o3wL"
#define ACTIVEKEY "8281-116U-R3YU-ZY85"

#define NSCALE 16
#define FACENUM 5
#define MAX_THREAD  15

#define SafeFree(p) { if ((p)) free(p); (p) = NULL; }
#define SafeArrayDelete(p) { if ((p)) delete [] (p); (p) = NULL; }
#define SafeDelete(p) { if ((p)) delete (p); (p) = NULL; }


QT_BEGIN_NAMESPACE
namespace Ui { class FaceRecognition; }
QT_END_NAMESPACE

class FaceRecognition : public QMainWindow
{
    Q_OBJECT
public:
    FaceRecognition(QWidget *parent = nullptr);
    ~FaceRecognition();
private:
    void getBaseInfo();
    void init();
    void timestampToTime(char* timeStamp,QString &dateTime); // convert time
    int ColorSpaceConvert(MInt32 width, MInt32 height, MInt32 format, MUInt8* imgData, ASVLOFFSCREEN& offscreen);  // convert diffrent color
    int extractFeature(const QString &imageName,const QString &lastName,QImage *Icon);  //feature extract
    void btnOperateCameraState();
    void slotSet();
    void initRegisterListWidget();
    void initStatusBar();
    void stillImageFR();
    void deleteSingleFaceData(QImage &img);
    void initSQLiteDB();
    void changeBtnStates(bool camState);

private slots:
    void updateTime();  // time update
    void loadImage();
    void operateCamera();
    void startVideoRecognize();
    void stopVideoRecognize();
    void registerFaceDBase();
    void clearFaceBase();
    void compareResult();
    void recvLogStr(const QString &logStr);
    void recvCompareResult(const QString &logStr);
    void recvResultFromFd(const QImage &image,const QRect &rect,const QString &str);
    void recvResultFromLr(const QString &str);
    void getLivenessThreshold();
    void on_registerListWidget_customContextMenuRequested(const QPoint &pos);
    void deleteSingleItem();
    void deleteAllItem();
    void changeFaceLine();

signals:
    void sendLabelData(const QImage &image,const QRect &rect,const QString &str);  // to MLabel.h
    void sendVideoData(const QVideoFrame &frame,const QSize &size,const QMap<QString,ASF_FaceFeature> &fDB);       // to fd thread
    void sendImageData(const QImage &localImage,const QSize &size,const QMap<QString,ASF_FaceFeature> &fDB,const MHandle &mHandle);   // to fd thread
    void stopCam();
    void startCam();
    void sendDataToCam(const QSize &size,const QMap<QString,ASF_FaceFeature> &fDB);
    void sendPreview(const QImage &image);
    void sendFaceData(const ASVLOFFSCREEN& aof,const ASF_MultiFaceInfo &mfi);
    void sendThresholdToVft(const float &thres);
    void sendFaceLine(const float &FaceLine);


private:
    Ui::FaceRecognition *ui;
    MHandle m_imageEngine = NULL; // init image engine
    MHandle m_videoftEngine = NULL; //init video engine
    MHandle m_imamefrEngine = NULL;
    MHandle m_imageflEngine = NULL;
    QTimer *timer; //time count
    QLabel *timeLabel;
    QDir dir;

    QMap<QString,ASF_FaceFeature> featureDB;  // face feature base,for compare face,

    float score=0.0;
    QString predict;
    QString cmpResStr;
    QImage localImage;
    QVideoFrame mFrame;
    ASVLOFFSCREEN mOffscreen= {0,0,0,{0},{0}};
    ASF_MultiFaceInfo mDetectedFaces = {0};

    // *camera thread*
    threadCam *workCam ;
    QThread *workCameraThread;
    bool cameraState;

    // detected result data
    QImage mImage;
    QRect mRect;
    QString mStrFd;
    QString mStrLr;

    // video thread object
    videoFt *vFt;
    QThread *videoFtWorkThread;
    float mThreshold=0.7;           // 活体检测阈值
    float mFaceLine = 0.5;          // 人脸对比阈值

    // sql obj
    FacesDataBase *fDatabase;
    QMap<QString,QPixmap> pixMap;

};
#endif // ARCFACEDEMO_H
