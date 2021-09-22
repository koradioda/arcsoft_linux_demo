#include "facedetect.h"
#include <QDebug>
#include <QThread>

FaceDetect::FaceDetect(QObject *parent) : QObject(parent),QRunnable()
{
    this->setAutoDelete(true);      // auto delete after delete
}

void FaceDetect::run()
{
    /*#### 实现人脸识别 ####*/
    if(mImage.isNull() || mHandle==NULL)
    {
        emit sendDetectedData(QImage(),QRect(),QString());
        return;
    }

    QRect rect;
    int width =mImage.width();
    int height = mImage.height();
    int format = ASVL_PAF_RGB24_B8G8R8;
    //  read image and get feature
    ASVLOFFSCREEN offscreen = {0,0,0,{0},{0}};
    ColorSpaceConvert(width, height, format, mImage.bits(), offscreen);
    ASF_MultiFaceInfo detectedFaces = {0};
    ASF_SingleFaceInfo SingleDetectedFaces = {{0,0,0,0},0,{0,0}};
    ASF_FaceFeature newFeature = {0,0};

    MRESULT res = ASFDetectFacesEx(mHandle, &offscreen, &detectedFaces);
    if (res != MOK && detectedFaces.faceNum > 0)
    {
        emit sendLogStr(QString(" ASFDetectFacesEx fail: %1\n").arg(res));
        return;
    }
    else
    {
        SingleDetectedFaces.faceRect.left = detectedFaces.faceRect[0].left;
        SingleDetectedFaces.faceRect.top = detectedFaces.faceRect[0].top;
        SingleDetectedFaces.faceRect.right = detectedFaces.faceRect[0].right;
        SingleDetectedFaces.faceRect.bottom = detectedFaces.faceRect[0].bottom;
        SingleDetectedFaces.faceOrient = detectedFaces.faceOrient[0];
        SingleDetectedFaces.faceDataInfo = detectedFaces.faceDataInfoList[0];

        rect.setTop(detectedFaces.faceRect[0].top);
        rect.setBottom(detectedFaces.faceRect[0].bottom);
        rect.setLeft(detectedFaces.faceRect[0].left);
        rect.setRight(detectedFaces.faceRect[0].right);

        MInt32 processMask = ASF_MASKDETECT;
        ASF_MaskInfo maskInfo = {0};
        MRESULT res = ASFProcessEx(mHandle,&offscreen,&detectedFaces,processMask);
        if(res!=MOK)
        {qDebug()<<"人脸面罩检测错误:"<<res;}
        res  = ASFGetMask(mHandle,&maskInfo);  // 获取口罩信息
        if(res!=MOK){
            qDebug()<<"获取面罩信息错误:"<<res;
            return ;
        }
        // 单人脸特征提取
        res = ASFFaceFeatureExtractEx(mHandle, &offscreen, &SingleDetectedFaces,ASF_RECOGNITION, maskInfo.maskArray[0],&newFeature);     //  获取脸部特征
        if (res != MOK)
        {
            emit sendLogStr(QString("ASFFaceFeatureExtractEx fail: %1\n").arg(res));
            qDebug()<<"tezhengjiance,特征提取";
            return ;
        }
    }
    // 单人脸特征比对
    MFloat confidenceLevel=0.7;
    MFloat maxScore=0.0;

    QMap<QString,ASF_FaceFeature>::iterator it = featureDB.begin();
    for(;it != featureDB.end();++it)
    {
        res = ASFFaceFeatureCompare(mHandle, &it.value(), &newFeature, &confidenceLevel);
        if (res != MOK)
        {
            emit sendLogStr(QString("ASFFaceFeatureCompare fail: %1\n").arg(res));
//            return ;
        }
        if(confidenceLevel>maxScore)
        {
            maxScore = confidenceLevel;
            predict = it.key();
            score = maxScore;
        }
    }
    if(maxScore<0.8)
        predict = "UnFound ";

    emit sendCompareResult(QString(" %1 : score %2\n").arg(predict).arg(maxScore));
    emit sendDetectedData(mImage,rect,predict);  // sub thread send data to MLabel.h

}

int FaceDetect::ColorSpaceConvert(MInt32 width, MInt32 height, MInt32 format, MUInt8 *imgData, ASVLOFFSCREEN &offscreen)
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

void FaceDetect::recvImageData(const QImage &image, const QSize &size, const QMap<QString, ASF_FaceFeature> &fDB,const MHandle &mhandle)
{   // recv image data from main main thread
    mSize = size;
    mImage = image;
    featureDB = fDB;
    mHandle = mhandle;
}

void FaceDetect::recvVideoData(const QVideoFrame &frame, const QSize &size, const QMap<QString, ASF_FaceFeature> &fDB)
{
    mSize = size;
//    mImage = frame.image();
    if(featureDB.isEmpty())
        featureDB = fDB;
}

