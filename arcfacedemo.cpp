#include "arcfacedemo.h"
#include "ui_arcfacedemo.h"
#include <QFileDialog>
#include <QMessageBox>
#include <string.h>
#include <QThread>
#include <QThreadPool>   //  thread pool

FaceRecognition::FaceRecognition(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::FaceRecognition)
{
    ui->setupUi(this);
    cameraState=false;
    fDatabase =new  FacesDataBase("mFeature.db");
    qDebug()<<QThread::currentThread();
    qRegisterMetaType<QMap<QString,ASF_FaceFeature>>("QMap<QString,ASF_FaceFeature>");

    QThreadPool::globalInstance()->setMaxThreadCount(MAX_THREAD);   // max thread numb
    initStatusBar();

    initSQLiteDB();               // 数据库初始化
    initRegisterListWidget();      // init list widget
    slotSet();
    getBaseInfo();                // check some basic information,like appid and key , and to activate sdk
    init();                       // init ASF_ENGINE,初始化引擎
    btnOperateCameraState();
    changeBtnStates(cameraState);

}

void FaceRecognition::recvLogStr(const QString &logStr)
{
    ui->editOutLog->append(logStr);
}

void FaceRecognition::recvCompareResult(const QString &resStr)
{
    cmpResStr = resStr;
}

void FaceRecognition::recvResultFromFd(const QImage &image, const QRect &rect, const QString &str)
{
    mImage = image;
    mRect = rect;
    mStrFd = str;
}

void FaceRecognition::recvResultFromLr(const QString &str)
{
    mStrLr = str;
}

// 活体阈值
void FaceRecognition::getLivenessThreshold()
{
    mThreshold = ui->Threshold->text().toFloat();
    emit sendThresholdToVft(mThreshold);

}
// 人脸对比阈值
void FaceRecognition::changeFaceLine()
{
    mFaceLine = ui->faceThreshold->text().toFloat();
    emit sendFaceLine(mFaceLine);
}

// compare Result
void FaceRecognition::compareResult()
{
    if(!cmpResStr.isEmpty())
        ui->editOutLog->append(cmpResStr);
    else
    {
        QMessageBox::warning(this,"Tips","Please choose image!");
    }
}

// change btnOperateCamera state
void FaceRecognition::btnOperateCameraState()
{
    if(featureDB.isEmpty()){
        ui->btnOperateCamera->setEnabled(false);
        ui->btnCompareFace->setEnabled(false);
    }
    else{
        ui->btnOperateCamera->setEnabled(true);
        ui->btnCompareFace->setEnabled(true);
    }
}

// slot sets
void FaceRecognition::slotSet()
{
    // some signals and response slots
    connect(ui->btnSelectImage,SIGNAL(clicked()),this,SLOT(loadImage()));
    connect(ui->btnOperateCamera,SIGNAL(clicked()),this,SLOT(operateCamera()));    // open camera
    connect(ui->btnRegister,SIGNAL(clicked()),this,SLOT(registerFaceDBase()));      // register face base
    connect(ui->btnClear,SIGNAL(clicked()),this,SLOT(clearFaceBase()));             // clear database
    connect(ui->btnCompareFace,SIGNAL(clicked()),this,SLOT(compareResult()));
    connect(this,SIGNAL(sendLabelData(QImage,QRect,QString)),
            ui->previewLabel,SLOT(recvDetectedDataFd(QImage,QRect,QString)));
    connect(this,SIGNAL(sendPreview(QImage)),ui->previewLabel,SLOT(recvGuiImage(QImage)));
    connect(ui->Threshold,SIGNAL(editingFinished()),this,SLOT(getLivenessThreshold()));
    connect(ui->faceThreshold,SIGNAL(editingFinished()),this,SLOT(changeFaceLine()));

}

// clear face data base
void FaceRecognition::clearFaceBase()
{
    ui->registerListWidget->clear();
    featureDB.clear();
    if(fDatabase->deleteTable())
    {
        ui->editOutLog->append("clear database successfully !\n");
    }

    if(featureDB.isEmpty())
        ui->editOutLog->append("Face data has cleared success !\n");
    else
        ui->editOutLog->append("Face data has cleared failed !\n");

    fDatabase->createTable();
    btnOperateCameraState();

}

// got face feature ************************************************************************************ need't to modify
int FaceRecognition::extractFeature(const QString &imageName,const QString &lastName,QImage *Icon)
{
    std::string str = imageName.toStdString();
    const char *imgName = str.c_str();        // 转换为char* 模式，便于得到处理

    QImage img = QImage(imageName).convertToFormat(QImage::Format_RGB888);
    img = img.scaled(img.width()-img.width()%4,img.height()-img.height()%2,Qt::IgnoreAspectRatio);   // 规范化图片格式：sdk只能处理宽是4的倍数，高是2的倍数的图片

    int width =img.width();
    int height = img.height();
    int format = ASVL_PAF_RGB24_B8G8R8;   // 图片格式

    /*   process image  */
    ASVLOFFSCREEN offscreen = {0,0,0,{0},{0}};          // 图片信息
    ColorSpaceConvert(width, height, format, img.bits(), offscreen);        // 根据format 获取图片信息，保存在offscreen中

    /*     face data   */
    ASF_MultiFaceInfo detectedFaces = {0,0,0,0};            // 检测到的多张人脸信息
    ASF_SingleFaceInfo SingleDetectedFaces = {{},0};        // 存储来自多张人脸信息中的单张信息
    ASF_FaceFeature feature = {0,0};

     /*     feature extract     */
    MRESULT res = ASFDetectFacesEx(m_imageEngine, &offscreen, &detectedFaces);   //  人脸检测，得到多张人脸信息
    if((res!=MOK && detectedFaces.faceNum > 0) || detectedFaces.faceNum <1)
    {
        ui->editOutLog->append(QString("%1 ASFDetectFaces 1 fail: %2\n").arg(imgName).arg(res));
        return -1;
    }
    else
    {   //保存单张人脸信息，为了之后进行特征提取
        SingleDetectedFaces.faceRect.left = detectedFaces.faceRect[0].left;
        SingleDetectedFaces.faceRect.top = detectedFaces.faceRect[0].top;
        SingleDetectedFaces.faceRect.right = detectedFaces.faceRect[0].right;
        SingleDetectedFaces.faceRect.bottom = detectedFaces.faceRect[0].bottom;        // note: w = right-left,h = bottom-top
        SingleDetectedFaces.faceOrient = detectedFaces.faceOrient[0];
        SingleDetectedFaces.faceDataInfo = detectedFaces.faceDataInfoList[0];

        // 截取人脸框区域的图像
        *Icon = img.copy(SingleDetectedFaces.faceRect.left,
                               SingleDetectedFaces.faceRect.top,
                               SingleDetectedFaces.faceRect.right - SingleDetectedFaces.faceRect.left,
                               SingleDetectedFaces.faceRect.bottom - SingleDetectedFaces.faceRect.top);    // 获取人脸框 人脸大小的图片区域

        MInt32 processMask = ASF_MASKDETECT;
        ASF_MaskInfo maskInfo = {0};
        MRESULT res = ASFProcessEx(m_imageEngine,&offscreen,&detectedFaces,processMask);   // 进行口罩检测，特征提取时需要用到。兼容了口罩识别
        if(res!=MOK){
            ui->editOutLog->append(QString("face mask process error:%1\n").arg(res));
        }
        res  = ASFGetMask(m_imageEngine,&maskInfo);  // 获取口罩信息
        if(res!=MOK){
            return -1;
        }
        // 单人脸特征提取，选择人脸识别模式
        res = ASFFaceFeatureExtractEx(m_imageEngine, &offscreen, &SingleDetectedFaces,ASF_RECOGNITION, maskInfo.maskArray[0],&feature);     //  获取脸部特征

        if (res != MOK)
        {
            ui->editOutLog->append(QString("%1 ASFDetectFaces 1 fail: %2\n").arg(imgName).arg(res));
            return -1;
        }
        else
        {
            ASF_FaceFeature ff = {0,0};
            ff.feature = (MByte *)malloc(feature.featureSize);
            ff.featureSize = feature.featureSize;
            memset(ff.feature,0,ff.featureSize);
            memcpy(ff.feature,feature.feature,ff.featureSize);

            /*   detect successed ,svae to map */
            QString person = lastName;
            person.truncate(person.lastIndexOf('.'));
            featureDB.insert(person,ff);  // 提取的特征保存到特征map中，用于人脸检测 1:N 的检测

            // process faceture 特征字节数组
            QByteArray farray = QByteArray::fromRawData((char *)ff.feature,ff.featureSize);    //  uchar* 数据 转 binary 数据

            // img data deal
            QByteArray imgData;     // 图片数据
            QBuffer inBuffer( &imgData );
            QPixmap pix = QPixmap::fromImage(*Icon);   // 讲人脸框区域的数据进行保存
            inBuffer.open( QIODevice::WriteOnly );
            pix.save( &inBuffer,"PNG");             //  必须指定存储数据的格式

            fDatabase->insertItem(person,ff.featureSize,farray,imgData);   // 数据库中永久插入数据，再次启动可以从数据库中进行加载
        }
    }

    return 0;
}

// register face database ****************************************************************************** need't to modify
void FaceRecognition::registerFaceDBase()
{
    // 扫描对应的目录,g获取照片, 将名字保存在链表中(by QStringList())
    QString dirName = QFileDialog::getExistingDirectory(this,"select directory","../",QFileDialog::ShowDirsOnly|QFileDialog::DontUseNativeDialog);
    ui->editOutLog->append("Directory is :"+dirName+"\n"); // log : show directory in log window

    // scan all file in dirName by QDir
    dir = QDir(dirName);                                                // 获取指定目录的数据
    QStringList imageList;
    imageList<< "*.YUYV"<<"*.NV21"<<"*.jpg"<<"*.bmp"<<"*.png"<<"*.jpeg";          // 对文件类型进行过滤，只保留指定类型的数据

    dir.setNameFilters(imageList);                                  //
    MInt32 imageCount = dir.count();                                // 获取扫描得到文件数量，

    for(int i=0;i<imageCount;++i)
    {   // 循环遍历每个文件，提取相关的特征
        QImage Icon;                // save a face picture
        // extract Feature  , and show the image in list widget
        if(extractFeature(dir.absoluteFilePath(dir[i]),dir[i],&Icon) > -1)
        {   // 提取特征，并将得到的人脸框部分作为icon添加到列表控件中
            QListWidgetItem *imageItem = new QListWidgetItem();
            QString name = dir[i];
            name.truncate(name.indexOf('.'));   // 获取图片名子，作为标识
            imageItem->setIcon(QIcon(QPixmap::fromImage(Icon)));
            imageItem->setText(name);
            ui->registerListWidget->addItem(imageItem);
        }

    }
    ui->registerListWidget->show();                                     // show all image
    ui->editOutLog->append("register success!"+QString(" all are %1 !\n").arg(imageCount)); // add
    btnOperateCameraState();

}

// ###### open camera ######
void FaceRecognition::startVideoRecognize()
{   // 进入人脸识别，打开摄像头和识别线程，通过摄像头线程发送帧数据给识别线程，再将结果和图片传递给主线程绘制
    // camera thread
    workCam = new threadCam();   // 相机线程对象
    workCameraThread = new QThread(this);
    workCam->moveToThread(workCameraThread);
    // video ft thread
    vFt = new videoFt();            // 人脸识别线程
    videoFtWorkThread = new QThread(this);      // 这些对象运行完后,通过信号槽会自动删除
    vFt->moveToThread(videoFtWorkThread);

    //  connect(workCam,SIGNAL(sendCapture(QImage,QRect,QString)),ui->previewLabel,SLOT(recvCamData(QImage,QRect,QString)));
    connect(this,SIGNAL(sendDataToCam(QSize,QMap<QString,ASF_FaceFeature>)),
            workCam,SLOT(recvDataFromGUI(QSize,QMap<QString,ASF_FaceFeature>)));        // 主线程发送数据给Camera线程
    connect(this,SIGNAL(startCam()),workCam,SLOT(cameraWork()));

    connect(workCam,SIGNAL(sendDataToVideoFt(QImage,QSize,QMap<QString,ASF_FaceFeature>)),
            vFt,SLOT(doWork(QImage,QSize,QMap<QString,ASF_FaceFeature>)));              //  Camera线程发送数据给 视频检测线程
    connect(vFt,SIGNAL(sendDetectedVideoData(QImage,QRect,QString)),
            ui->previewLabel, SLOT(recvDetectedVideoData(QImage,QRect,QString)));

    // ******* set threshold of liveness threshold *******
    connect(this,SIGNAL(sendThresholdToVft(float)),vFt,SLOT(recvThresholdFromGui(float)));
    connect(this,SIGNAL(sendFaceLine(float)),vFt,SLOT(recvFaceLineFromGui(float)));


    emit sendDataToCam(ui->previewLabel->size(),featureDB);      // Cam got size and FDB
    emit sendThresholdToVft(mThreshold);
    emit startCam();
    workCameraThread->start();
    videoFtWorkThread->start();
    if(workCam->isAvailable()){
        cameraState = true;
        changeBtnStates(cameraState);               // 打开摄像机，部分按钮不可用
    }
    else{
        workCameraThread->quit();               // 摄像机不可用，则退出视频和camera线程
        workCameraThread->wait();               // 退出相机线程
        videoFtWorkThread->quit();
        videoFtWorkThread->wait();              // 退出视频检测线程
    }
}

void FaceRecognition::changeBtnStates(bool camState)
{
    if(camState){
        ui->btnCompareFace->setEnabled(false);
        ui->btnRegister->setEnabled(false);
        ui->btnClear->setEnabled(false);
        ui->btnOperateCamera->setText(tr("Close Camera"));
    }
    else{
        ui->btnCompareFace->setEnabled(true);
        ui->btnRegister->setEnabled(true);
        ui->btnClear->setEnabled(true);
        ui->btnOperateCamera->setText(tr("Open Camera"));
    }
}
// ###### close camera ######
void FaceRecognition::stopVideoRecognize()
{
    // close thread Cam
    connect(this,SIGNAL(stopCam()),workCam,SLOT(stopCamera()));
    connect(workCameraThread,SIGNAL(finished()),workCam,SLOT(deleteLater()));   //摄像机关闭信号槽，线程完成后，删除对应的对象
    // close thread video ft
    connect(this,SIGNAL(stopCam()),vFt,SLOT(closeVideoFt()));
    connect(videoFtWorkThread,SIGNAL(finished()),vFt,SLOT(deleteLater()));      // 视频检测关闭信号槽

    emit stopCam();
    workCameraThread->quit();
    workCameraThread->wait();               // 退出相机线程
    videoFtWorkThread->quit();
    videoFtWorkThread->wait();              // 退出视频检测线程
    emit sendLabelData(QImage(),QRect(),QString());

    cameraState = false;
    changeBtnStates(cameraState);               // 关闭摄像机，部分按钮可用
    cmpResStr.clear();
}

// ###### judge open or close camera ######
void FaceRecognition::operateCamera()
{
    switch(ui->btnOperateCamera->text()==tr("Open Camera"))
    {
    case true:
        startVideoRecognize();
        if(workCam->isAvailable())
            ui->editOutLog->append("opened camera!\n");
        else
            ui->editOutLog->append("camera failed!\n");
        break;
    case false:
        stopVideoRecognize();
        ui->editOutLog->append("closed camera!\n");
        break;
    }
}

// load Image and show in label
void FaceRecognition::loadImage()
{
    // 加载图片，用于单张图片的人脸识别
    QString fileName = QFileDialog::getOpenFileName(this,"open file","./images","AllFile (*.*)",
                                                    nullptr,QFileDialog::DontUseNativeDialog);  // 打开文件对话框获取文件名
    if(fileName.isEmpty()) {   // 判断是否加载了图片
        ui->editOutLog->append("Load Image canceled!\n");
        return;
    }
    localImage = QImage(fileName);
    localImage = localImage.scaled(ui->previewLabel->size(),Qt::KeepAspectRatio);   // 改变图片的尺度规格
    localImage = localImage.convertToFormat(QImage::Format_RGB888);                 // 改变图片的格式
    localImage = localImage.scaled(localImage.width()-localImage.width()%4,
                                   localImage.height()+localImage.height()%2,Qt::KeepAspectRatio);   // 改变图片的规格

    emit sendPreview(localImage);     // 发送图片给 previewLabel 显示预览
    stillImageFR();

}

// show time in statuBar
void FaceRecognition::updateTime()
{
    QDateTime date = QDateTime::currentDateTime();   // update current time
    timeLabel->setText(date.toString());
}

// get sdk basic info
void FaceRecognition::getBaseInfo()
{
    ui->editOutLog->append(QString("\n************* ArcFace SDK Info *****************\n"));
    MRESULT res = MOK;
    ASF_ActiveFileInfo activeFileInfo = {0};
    res = ASFGetActiveFileInfo(&activeFileInfo);
    if (res != MOK){
            ui->editOutLog->append(QString("ASFGetActiveFileInfo fail: %1\n").arg(res));
    }
    else{
        //这里仅获取了有效期时间，还需要其他信息直接打印即可
        QString startDateTime;
        timestampToTime(activeFileInfo.startTime, startDateTime);
        ui->editOutLog->append("Start time: "+startDateTime+"\n");
        QString endDateTime;
        timestampToTime(activeFileInfo.endTime, endDateTime);
        ui->editOutLog->append("End time: " + endDateTime + "\n");
    }

    //SDK版本信息
    const ASF_VERSION version = ASFGetVersion();
    ui->editOutLog->append(QString("\nVersion:%1\n").arg( version.Version));
    ui->editOutLog->append(QString("BuildDate:%1\n").arg(version.BuildDate));
    ui->editOutLog->append(QString("CopyRight:%1\n").arg(version.CopyRight));

    ui->editOutLog->append(QString("\n************* Face Recognition *****************\n"));

    QSettings *KeyInfo = new QSettings("settings.ini",QSettings::IniFormat);
    QString appID = KeyInfo->value("/ActivateInfo/appID").toString();
    QString sdkKEY = KeyInfo->value("/ActivateInfo/sdkKEY").toString();
    QString activeKEY = KeyInfo->value("/ActivateInfo/activeKEY").toString();
    delete KeyInfo;
    QByteArray ba = appID.toLatin1();
    APPID = ba.data();
    QByteArray bs = sdkKEY.toLatin1();
    SDKKEY = bs.data();
    QByteArray bc = activeKEY.toLatin1();
    ACTIVEKEY = bc.data();
    res = ASFOnlineActivation(APPID, SDKKEY, ACTIVEKEY);
    qDebug()<<APPID<<" "<<SDKKEY<<" "<< ACTIVEKEY;
    if (MOK != res && MERR_ASF_ALREADY_ACTIVATED != res)
            ui->editOutLog->append(QString("ASFOnlineActivation fail: %1\n").arg(res));
    else
           ui->editOutLog->append(QString("ASFOnlineActivation sucess: %1\n").arg(res));

}

// init FR_Engine
void FaceRecognition::init()
{
    // inital ASF_ENGINE
    MRESULT res = MOK;
    MInt32 mask = ASF_FACE_DETECT | ASF_FACERECOGNITION | ASF_AGE | ASF_GENDER | ASF_LIVENESS | ASF_IR_LIVENESS | ASF_MASKDETECT; //set functions we need
    res = ASFInitEngine(ASF_DETECT_MODE_IMAGE, ASF_OP_0_ONLY, FACENUM, mask, &m_imageEngine); // this just open still image recognition
//    res = ASFInitEngine(ASF_DETECT_MODE_VIDEO, ASF_OP_0_ONLY, NSCALE, FACENUM, mask, &m_imageEngine); // this just open still image recognition

    if (res != MOK)
        ui->editOutLog->append(QString("ASFInitEngine fail: %1\n").arg(res));
    else
        ui->editOutLog->append(QString("ASFInitEngine success!\n"));  // init ASFENGINE sucessful,got asf_engine m_imageEngine
}

// convert image color and format
int FaceRecognition::ColorSpaceConvert(MInt32 width, MInt32 height, MInt32 format, MUInt8 *imgData, ASVLOFFSCREEN &offscreen)
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

// convert timeStamp to normal time format
void FaceRecognition::timestampToTime(char *timeStamp,QString &dateTime)
{
    time_t time = atoll(timeStamp);
    QDateTime date = QDateTime::fromTime_t(time); // convert to QDateTime "y-m-d-..."
    dateTime = date.toString();
}

void FaceRecognition::initRegisterListWidget()
{
    btnOperateCameraState();
    ui->registerListWidget->setViewMode(QListView::IconMode);    // 列表空间显示模式
    ui->registerListWidget->setIconSize(QSize(70,80));           // Icon的大小
    ui->registerListWidget->setMovement(QListView::Static);      // 是否可移动
    ui->registerListWidget->setSpacing(15);                      // 设置间隔

    // 加载数据，显示图片
    QMap<QString,QPixmap>::iterator it = pixMap.begin();
    while(it!=pixMap.end())
    {
        // set to listWidget
        QListWidgetItem *imageItem = new QListWidgetItem();
        imageItem->setIcon(QIcon(it.value()));
        imageItem->setText(it.key());
        ui->registerListWidget->addItem(imageItem);
        ++it;
    }
    pixMap.clear();   // 清楚加载后的临时图片数据
}

void FaceRecognition::initStatusBar()
{
    // show system time in statusbar
    timeLabel = new QLabel(statusBar());
    ui->statusbar->addWidget(timeLabel);
    timer = new QTimer(this);
    connect(timer,SIGNAL(timeout()),this,SLOT(updateTime()));
    timer->start(1000);
}

void FaceRecognition::stillImageFR()
{
    FaceDetect *fd = new FaceDetect;        // 人脸检测
    connect(this,SIGNAL(sendImageData(QImage,QSize,QMap<QString,ASF_FaceFeature>,MHandle)),
            fd,SLOT(recvImageData(QImage,QSize,QMap<QString,ASF_FaceFeature>,MHandle)));           // send data to fd
    connect(fd,SIGNAL(sendLogStr(QString)),this,SLOT(recvLogStr(QString)));                 // recv log from fd
    connect(fd,SIGNAL(sendDetectedData(QImage,QRect,QString)),
            ui->previewLabel,SLOT(recvDetectedDataFd(QImage,QRect,QString)));                             // recv data from fd
    connect(fd,SIGNAL(sendCompareResult(QString)),this,SLOT(recvCompareResult(QString)));   // recv compare result

    LivenessRecognize *lr = new LivenessRecognize;      // 活体识别
    connect(this,SIGNAL(sendImageData(QImage,QSize,QMap<QString,ASF_FaceFeature>,MHandle)),
            lr,SLOT(recvImageData(QImage,QSize,QMap<QString,ASF_FaceFeature>,MHandle)));           // send data to lr
    connect(lr,SIGNAL(sendLogStr(QString)),this,SLOT(recvLogStr(QString)));                 // recv log from lr
    connect(lr,SIGNAL(sendLrResult(QString)),ui->previewLabel,SLOT(recvDetectedDataLr(QString)));         // recv res from lr

    //  发送基本参数数据，便于特征提取和绘制
    emit sendImageData(localImage,ui->previewLabel->size(),featureDB,m_imageEngine);         // send image data

    QThreadPool::globalInstance()->start(fd);               // open threads，放入线程进行相关识别
    QThreadPool::globalInstance()->start(lr);
}

void FaceRecognition::deleteSingleFaceData(QImage &img)         // 删除数据库中的一条数据
{
    img = img.convertToFormat(QImage::Format_RGB888);
    img = img.scaled(img.width()-img.width()%4,img.height()-img.height()%2,Qt::IgnoreAspectRatio);   // regulate image size

    int width =img.width();
    int height = img.height();
    int format = ASVL_PAF_RGB24_B8G8R8;
    /*   process image  */
    ASVLOFFSCREEN offscreen = {0,0,0,{0},{0}};
    ColorSpaceConvert(width, height, format, img.bits(), offscreen);

    /*     face data   */
    ASF_MultiFaceInfo detectedFaces = {0};
    ASF_SingleFaceInfo SingleDetectedFaces = {0};
    ASF_FaceFeature feature = {0,0};

     /*     feature extract     */
    ASFDetectFacesEx(m_imageEngine, &offscreen, &detectedFaces);   // detect face, 得到图像数据中的人脸信息

    SingleDetectedFaces.faceRect.left = detectedFaces.faceRect[0].left;
    SingleDetectedFaces.faceRect.top = detectedFaces.faceRect[0].top;
    SingleDetectedFaces.faceRect.right = detectedFaces.faceRect[0].right;
    SingleDetectedFaces.faceRect.bottom = detectedFaces.faceRect[0].bottom;        // note: w = right-left,h = bottom-top
    SingleDetectedFaces.faceOrient = detectedFaces.faceOrient[0];
    SingleDetectedFaces.faceDataInfo = detectedFaces.faceDataInfoList[0];

    MInt32 processMask = ASF_MASKDETECT;
    ASF_MaskInfo maskInfo = {0};
    MRESULT res = ASFProcessEx(m_imageEngine,&offscreen,&detectedFaces,processMask);   // 进行口罩检测，特征提取时需要用到
    if(res!=MOK){
        ui->editOutLog->append(QString("face mask process error:%1\n").arg(res));
    }
    res  = ASFGetMask(m_imageEngine,&maskInfo);  // 获取口罩信息
    if(res!=MOK){
        return ;
    }
    // 单人脸特征提取
    res = ASFFaceFeatureExtractEx(m_imageEngine, &offscreen, &SingleDetectedFaces,ASF_RECOGNITION, maskInfo.maskArray[0],&feature);     //  获取脸部特征

    if (res != MOK){
        ui->editOutLog->append(QString("ASFDetectFaces fail: %1\n").arg(res));
        return;
    }
    else{
        MFloat confidenceLevel=0.0;
        QMap<QString,ASF_FaceFeature>::iterator it = featureDB.begin();

        for(;it!=featureDB.end();++it){
            ASFFaceFeatureCompare(m_imageEngine,&it.value(),&feature,&confidenceLevel);   //  feature compare,feature is in faceData,it is data which we want find

            if(confidenceLevel > 0.95){
                break;
            }
        }

        // delete in data base
        QByteArray farray = QByteArray::fromRawData((char*)it.value().feature,it.value().featureSize);          // make the feature turn to binary data
        if(fDatabase->deleteItem(farray))            // 删除数据库中的特征值为 farray 的数据
        {
            featureDB.erase(it);
            ui->editOutLog->append(QString("%1 delete successfully !").arg(it.key()));
        }
        else
        {
            qDebug()<<"data base delete error !";
        }
    }
}

FaceRecognition::~FaceRecognition()
{
    delete ui;
    delete timer;
    delete fDatabase;
    delete timeLabel;

    ASFUninitEngine(m_imageEngine);
}

void FaceRecognition::on_registerListWidget_customContextMenuRequested(const QPoint &pos)
{
    QListWidgetItem *curItem = ui->registerListWidget->itemAt(pos);     // item's point,  列表空间创建又击菜单
    if(curItem == NULL)
        return;             // the point of clicked has no item  ,return

    QMenu *listMenu = new QMenu(this);            // menu
    QAction *deleteItem = new QAction("Delete",this);
    QAction *clearAll = new QAction("Clear",this);
    listMenu->addAction(deleteItem);
    listMenu->addAction(clearAll);

    // signals to  slots
    connect(deleteItem,SIGNAL(triggered()),this,SLOT(deleteSingleItem()));
    connect(clearAll,SIGNAL(triggered()),this,SLOT(deleteAllItem()));

    listMenu->exec(QCursor::pos());
    // delete all component
    delete listMenu;
    delete deleteItem;
    delete clearAll;
}

void FaceRecognition::deleteSingleItem()        // 删除列表控件的单个项目
{           /*** delete one item at the same time delete database single data ***/
    int op = QMessageBox::warning(nullptr,"clear warning!",
                                  "Did you want to clear all infomations?",
                                  QMessageBox::Yes,QMessageBox::No);
    if(op != QMessageBox::Yes)
        return;

    // get item icon
    int itemRow = ui->registerListWidget->currentRow();
    QListWidgetItem *item = ui->registerListWidget->takeItem(itemRow);                  // get item data;
    QString itemName = item->text();
    ui->editOutLog->append(QString("%1removed!\n").arg(itemName));
    QImage img = item->icon().pixmap(ui->registerListWidget->iconSize()).toImage();    // get item from listwidget item

    deleteSingleFaceData(img);
    if(featureDB.isEmpty())
        btnOperateCameraState();

}

void FaceRecognition::deleteAllItem()
{           /*** delete all items ***/
    int op = QMessageBox::warning(nullptr,"clear warning!",
                                  "Did you want to clear all infomations?",
                                  QMessageBox::Yes,QMessageBox::No);
    if(op == QMessageBox::Yes)
        clearFaceBase();
}

// init database
void FaceRecognition::initSQLiteDB()
{
    fDatabase->createTable();
    fDatabase->loadDataBase(featureDB,pixMap);  // 从数据库中加载全部数据
}


