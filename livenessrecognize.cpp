#include "livenessrecognize.h"
#include <QDebug>
#include <QThread>

LivenessRecognize::LivenessRecognize(QObject *parent) : QObject(parent),QRunnable()
{
    this->setAutoDelete(true);
}

void LivenessRecognize::run()
{
    /*###   实现性别，年龄，活体的识别   ###*/

    if(mImage.isNull()){
        emit sendDetectedData(QString());
        return;
    }
    // Lr need additional engine,otherwise ,it would quit forcly
    MRESULT res = MOK;
    MInt32 mask;
    mask = ASF_FACE_DETECT | ASF_FACERECOGNITION | ASF_AGE | ASF_GENDER | ASF_LIVENESS | ASF_IR_LIVENESS | ASF_MASKDETECT; //set functions we need
    res = ASFInitEngine(ASF_DETECT_MODE_IMAGE, ASF_OP_0_ONLY, FACENUM, mask, &mHandle);

    if (res != MOK){
        emit sendLogStr(QString("ASFInitEngine fail: %1\n").arg(res));
        return ;
    }
    int width =mImage.width();
    int height = mImage.height();
    int format = ASVL_PAF_RGB24_B8G8R8;

    //  read image and get feature
    ASVLOFFSCREEN offscreen = {0,0,0,{0},{0}};
    ColorSpaceConvert(width, height, format, mImage.bits(), offscreen);
    ASF_MultiFaceInfo detectedFaces = {0,0,0,0};

    res = ASFDetectFacesEx(mHandle, &offscreen, &detectedFaces);
    if (res != MOK && detectedFaces.faceNum > 0)
    {
        emit sendLogStr(QString(" ASFDetectFacesEx fail: %2\n").arg(res));
        return;
    }
    mask = ASF_AGE | ASF_GENDER | ASF_LIVENESS | ASF_MASKDETECT; // open age , gander & liveness detect
    res = ASFProcessEx(mHandle,&offscreen,&detectedFaces,mask);
    if(res != MOK){
        emit sendLogStr(QString("ASFProcessEx error:%1").arg(res));
        return;
    }
    // get age info
    ASF_AgeInfo ageInfo = {0,0};
    ASFGetAge(mHandle,&ageInfo);
    QString age = QString(":age:%1").arg(ageInfo.ageArray[0]);

    // get gender info
    ASF_GenderInfo genderInfo = {0,0};
    ASFGetGender(mHandle,&genderInfo);
    QString gender = QString(" sex:%1").arg(genderInfo.genderArray[0]==0? "Boy":"Girl");
    // get liveness info
    ASF_LivenessInfo livenessInfo = {0,0};
    ASFGetLivenessScore(mHandle,&livenessInfo);
    QString isLive = QString(",isLive:%1").arg(livenessInfo.isLive[0] > 0? "True":"False");

    QString str = age + gender + isLive;

    emit sendLrResult(str);
    ASFUninitEngine(mHandle);                     // destroy engine
}

int LivenessRecognize::ColorSpaceConvert(MInt32 width, MInt32 height, MInt32 format, MUInt8 *imgData, ASVLOFFSCREEN &offscreen)
{
    offscreen.u32PixelArrayFormat = (unsigned int)format;
    offscreen.i32Width = width;
    offscreen.i32Height = height;

    switch (offscreen.u32PixelArrayFormat)
    {
    case ASVL_PAF_RGB24_B8G8R8:
            offscreen.pi32Pitch[0] = offscreen.i32Width * 3;
            offscreen.ppu8Plane[0] = imgData;
            break;
    case ASVL_PAF_I420:
            offscreen.pi32Pitch[0] = width;
            offscreen.pi32Pitch[1] = width >> 1;
            offscreen.pi32Pitch[2] = width >> 1;
            offscreen.ppu8Plane[0] = imgData;
            offscreen.ppu8Plane[1] = offscreen.ppu8Plane[0] + offscreen.i32Height*offscreen.i32Width;
            offscreen.ppu8Plane[2] = offscreen.ppu8Plane[0] + offscreen.i32Height*offscreen.i32Width * 5 / 4;
            break;
    case ASVL_PAF_NV12:
    case ASVL_PAF_NV21:
            offscreen.pi32Pitch[0] = offscreen.i32Width;
            offscreen.pi32Pitch[1] = offscreen.pi32Pitch[0];
            offscreen.ppu8Plane[0] = imgData;
            offscreen.ppu8Plane[1] = offscreen.ppu8Plane[0] + offscreen.pi32Pitch[0] * offscreen.i32Height;
            break;
    case ASVL_PAF_YUYV:
    case ASVL_PAF_DEPTH_U16:
        offscreen.pi32Pitch[0] = offscreen.i32Width * 2;
            offscreen.ppu8Plane[0] = imgData;
            break;
    case ASVL_PAF_GRAY:
            offscreen.pi32Pitch[0] = offscreen.i32Width;
            offscreen.ppu8Plane[0] = imgData;
            break;
    default:
            return 0;
    }
    return 1;
}

void LivenessRecognize::recvImageData(const QImage &image, const QSize &size, const QMap<QString, ASF_FaceFeature> &fDB,const MHandle &mhandle)
{
    Q_UNUSED(fDB);
    Q_UNUSED(mhandle);
    mImage = image;
    mSize = size;
}

void LivenessRecognize::recvVideoData(const QVideoFrame &frame, const QSize &size, const QMap<QString, ASF_FaceFeature> &fDB)
{
    Q_UNUSED(fDB);
    mSize = size;
//    mImage = frame.image();
}
