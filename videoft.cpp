#include "videoft.h"
#include <QDebug>


videoFt::videoFt(QObject *parent) : QObject(parent)
{   // 人脸识别相关初始化
    frames=0;
    detectData="";
    predict="";
    score=0.0;
    frCount=0;      // 初始化基本参数
    MRESULT res = MOK;
    MInt32 mask = ASF_FACE_DETECT | ASF_LIVENESS | ASF_FACERECOGNITION | ASF_MASKDETECT ;           // 用于人脸跟踪
    res = ASFInitEngine(ASF_DETECT_MODE_VIDEO, ASF_OP_ALL_OUT, FACENUM, mask, &mVideoFtEngine);     // 跟踪人脸框
    if (res != MOK){
        qDebug()<<"init video engine error:"<<res;
        return ;
    }

    // 初始化图片句柄，用于年龄活体等识别
    mask = ASF_FACE_DETECT | ASF_FACERECOGNITION | ASF_AGE | ASF_GENDER | ASF_LIVENESS | ASF_MASKDETECT;        // 用于单张图片的信息检测，性别，年龄，活体等信息
    res = ASFInitEngine(ASF_DETECT_MODE_IMAGE, ASF_OP_ALL_OUT, FACENUM, mask, &mImageEngine);
    if (res != MOK)
    {
        qDebug()<<"init image engine error:"<<res;
        return ;
    }
    connect(this,SIGNAL(recvedThreshold()),this,SLOT(setThreshold()));      // 改变识别活体识别阈值

}

videoFt::~videoFt()
{
     emit sendDetectedVideoData(QImage(),QRect(),QString());
     ASFUninitEngine(mVideoFtEngine);
     ASFUninitEngine(mImageEngine);// destroy engine
     qDebug()<<QThread::currentThread()<<"end by videoFt";
}

MRESULT videoFt::featureExtractEx(QSharedPointer<OffScreen> offscreen, QSharedPointer<MutilFace> detectedFaces, ASF_FaceFeature *newFeature)
{
    MRESULT res=MOK;
    ASF_SingleFaceInfo SingleDetectedFaces = {{0,0,0,0},0,{0,0}};
    SingleDetectedFaces.faceRect.left = detectedFaces->pMultiFaceInfo->faceRect[0].left;
    SingleDetectedFaces.faceRect.top = detectedFaces->pMultiFaceInfo->faceRect[0].top;
    SingleDetectedFaces.faceRect.right = detectedFaces->pMultiFaceInfo->faceRect[0].right;
    SingleDetectedFaces.faceRect.bottom = detectedFaces->pMultiFaceInfo->faceRect[0].bottom;
    SingleDetectedFaces.faceOrient = detectedFaces->pMultiFaceInfo->faceOrient[0];                  // 获取单张人脸信息，用于特征提取
    SingleDetectedFaces.faceDataInfo = detectedFaces->pMultiFaceInfo->faceDataInfoList[0];

    MInt32 processMask = ASF_MASKDETECT;
    ASF_MaskInfo maskInfo = {0};
    res = ASFProcessEx(mImageEngine,offscreen->pOffScreen,detectedFaces->pMultiFaceInfo,processMask);   // 进行口罩检测，特征提取时需要用到
    if(res!=MOK){
        qDebug()<<"Video face mask detect error:"<<res;
        return res;
    }
    res  = ASFGetMask(mImageEngine,&maskInfo);  // 获取口罩信息
    if(res!=MOK){
        return res;
    }
    res = ASFFaceFeatureExtractEx(mImageEngine, offscreen->pOffScreen, &SingleDetectedFaces,ASF_RECOGNITION, maskInfo.maskArray[0],newFeature);    // cost time
    if (res != MOK){
        qDebug()<<"Video ASFFaceFeatureExtractEx error:"<<res;
        return res;
    }

    return res;
}

MInt32 videoFt::livenessDetectEx(QSharedPointer<OffScreen> offscreen, QSharedPointer<MutilFace> detectedFaces)
{
    MRESULT res=MOK;
    QString age,gender,isLive;
    MInt32 liveness = 0;
    MInt32 mask = ASF_AGE | ASF_GENDER | ASF_LIVENESS | ASF_MASKDETECT;                     // open age ,gander & liveness detect
    if(detectedFaces->pMultiFaceInfo->faceNum>0)
    {
        res = ASFProcessEx(mImageEngine,offscreen->pOffScreen,detectedFaces->pMultiFaceInfo,mask);
        if(res!=MOK)
        {
            qDebug()<<"ASFProcess error:"<<res;
            return 0;
        }
        else
        {
            // get age  获取年龄信息
            ASF_AgeInfo ageInfo = {0,0};
            ASFGetAge(mImageEngine,&ageInfo);
            age = QString(":age:%1").arg(ageInfo.ageArray[0]);
            // get gender
            ASF_GenderInfo genderInfo = {0,0};
            ASFGetGender(mImageEngine,&genderInfo);
            gender = QString(" sex:%1").arg(genderInfo.genderArray[0]==0? "Boy":"Girl");
            // liveness
            ASF_LivenessInfo livenessInfo = {0,0};
            ASFGetLivenessScore(mImageEngine,&livenessInfo);
            liveness = livenessInfo.isLive[0];
            isLive = QString(",isLive:%1").arg(livenessInfo.isLive[0]>0? "True":"False");
        }
        detectData = age+gender+isLive;
    }
    else
    {
        detectData = "";
        return 0;
    }

    return liveness;      // return liveness detect turn
}

void videoFt::threadFr(QSharedPointer<OffScreen> offscreen, QSharedPointer<MutilFace> detectedFaces)
{

    ASF_FaceFeature newFeature={0,0};           // 提取的特征，用于比较
    // 活体，性别，年龄检测，线程
    QFuture<MInt32> lrture = QtConcurrent::run(this,&videoFt::livenessDetectEx,
                                                offscreen,detectedFaces);
    // 人脸识别 线程
    QFuture<MRESULT> ffture = QtConcurrent::run(this,&videoFt::featureExtractEx,
                                                offscreen,detectedFaces,&newFeature);

    MRESULT res = ffture.result();
    MInt32 isLiveness = lrture.result();          // 等待两个线程的结果

    if(res==MOK && isLiveness>0)                  // 特征提取成功，且是活体时，才进行人脸库对比
    {
        // feature compare
        MFloat confidenceLevel=0.0;
        MFloat maxScore=0.0;
        QMap<QString,ASF_FaceFeature>::iterator it = featureDB.begin();
        for(;it != featureDB.end();++it){
            res = ASFFaceFeatureCompare(mImageEngine, &it.value(), &newFeature, &confidenceLevel);
            if (res != MOK){
                qDebug()<<"compare error:"<<res;
            }

            if(confidenceLevel>maxScore){
                maxScore = confidenceLevel;
                predict = it.key();
                score = maxScore;
            }
            if(score>mFaceLine)
                break;          // 当对比大于给定值时，提前结束，减少对比时间
        }
        if(score<mFaceLine)
            predict = "UnFound";
    }
    else{
        predict="UnFound";
        detectData = ":fake";
        score = 0.0;
    }
}

void videoFt::doWork(const QImage &mImage,const QSize &mSize,const QMap<QString,ASF_FaceFeature> &fDB)
{
    if(featureDB.isEmpty())
        featureDB =fDB;
    QString str="";
    MRESULT res = MOK;      //
    QImage img = mImage;
    QRect rect=QRect();
    img = img.convertToFormat(QImage::Format_RGB888);                                               // turn to rgb888                                                // regulate image size
    img = img.scaled(mSize.width()-mSize.width()%4,mSize.height()-mSize.height()%2,Qt::KeepAspectRatio);

    int width =img.width();
    int height = img.height();
    int format = ASVL_PAF_RGB24_B8G8R8;
    //  read image and get feature
    ASVLOFFSCREEN offscreen={0,0,0,{0},{0}};
    ASF_MultiFaceInfo detectedFaces = {0};

    ColorSpaceConvert(width, height, format, img.bits(), offscreen);
    res = ASFDetectFacesEx(mVideoFtEngine, &offscreen, &detectedFaces);  // 视频内的人脸检测，检测人脸信息
    if ((res != MOK && detectedFaces.faceNum>0)  || detectedFaces.faceNum<1){   // 如果检测失败|人脸检测不到则直接发送空矩阵和字符
        if(res!=MOK)
            qDebug()<<"ASFDetectFacesEx fail:"<<res;
        rect = QRect();
        str="";
        emit sendDetectedVideoData(img,rect,str);
        return ;
    }
    else{ // 获取人脸框
        rect.setTop(detectedFaces.faceRect[0].top);
        rect.setBottom(detectedFaces.faceRect[0].bottom);
        rect.setLeft(detectedFaces.faceRect[0].left);
        rect.setRight(detectedFaces.faceRect[0].right);
    }

    if(preFaceID != detectedFaces.faceID[0]){
        // if id has changed ,then extract feature
        idChanged = true;
        str="";
        predict="";
        detectData="";
        preFaceID = detectedFaces.faceID[0];
        frCount = 0;
    }
//    if(((idChanged && detectedFaces.faceNum>0) || (predict=="UnFound" && frames>24) || (str=="" && frames>24))&&detectedFaces.faceNum>0)
    if((idChanged || (frCount<=3 && predict=="UnFound" && frames>15)) && detectedFaces.faceNum>0)
    {   // condition need optimize
        QSharedPointer<OffScreen> offscreenPtr( new OffScreen(offscreen));
        QSharedPointer<MutilFace> detectedFacesPtr( new MutilFace(detectedFaces));
        QFuture<void> frThread = QtConcurrent::run(this,&videoFt::threadFr, offscreenPtr, detectedFacesPtr);
        idChanged = false;
        ++frCount;
        frames = 0;
    }

//    emit sendCompareResult(QString(" %1 : score %2\n").arg(predict).arg(score));
    str = predict+detectData;
    emit sendDetectedVideoData(img,rect,str);               // sub thread send data to MLabel.h
    ++frames;
}

void videoFt::recvDataFromCam(const QImage &image, const QSize &size, const QMap<QString, ASF_FaceFeature> &fDB)
{
    // init base data
    mImage = image;
    mSize = size;
    if(featureDB.isEmpty())
        featureDB = fDB;
}

void videoFt::closeVideoFt()
{

}

int videoFt::ColorSpaceConvert(MInt32 width, MInt32 height, MInt32 format, MUInt8 *imgData, ASVLOFFSCREEN &offscreen)
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

void videoFt::frameLook()
{
    qDebug()<<"frames :"<<frames;
}

void videoFt::recvThresholdFromGui(const float &thres)
{
    mThreshold.thresholdmodel_BGR = thres;
    mThreshold.thresholdmodel_IR = 0.7;
    mThreshold.thresholdmodel_FQ = 0.65;
    emit recvedThreshold();
}

void videoFt::setThreshold()
{
    ASFSetLivenessParam(mImageEngine,&mThreshold);
}

void videoFt::recvFaceLineFromGui(const float &faceLine)
{
    mFaceLine = faceLine;
}
