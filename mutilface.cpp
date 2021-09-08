#include "mutilface.h"

MutilFace::MutilFace(QObject *parent) : QObject(parent)
{
    pMultiFaceInfo = (LPASF_MultiFaceInfo)calloc(1, sizeof (ASF_MultiFaceInfo));  // apply basic memory in heap
}

MutilFace::MutilFace(ASF_MultiFaceInfo faceInfo) {
    //   申请多人脸信息需要的空间
    pMultiFaceInfo = (LPASF_MultiFaceInfo)calloc(1, sizeof (ASF_MultiFaceInfo));        // 为整个结构体申请空间

    pMultiFaceInfo->faceNum = faceInfo.faceNum;                                         // 拷贝人脸的数量
    pMultiFaceInfo->faceID = (int*)calloc(FACENUM,sizeof(int));                         // faceid 数组
    pMultiFaceInfo->faceOrient = (int*)calloc(FACENUM,sizeof(int));                     // 人脸角度数组
    pMultiFaceInfo->faceRect = (MRECT*)calloc(FACENUM,sizeof(MRECT));                   // 人脸框数组
    pMultiFaceInfo->faceIsWithinBoundary = (int*)calloc(FACENUM,sizeof(int));           // 检测人脸是否溢出边界
    pMultiFaceInfo->foreheadRect = (MRECT*)calloc(FACENUM,sizeof(MRECT));               // 人脸额头框框的区域
    // 人脸的属性特征
    pMultiFaceInfo->faceAttributeInfo.leftEyeOpen = (int*)calloc(FACENUM,sizeof (int));     // 左眼是否开
    pMultiFaceInfo->faceAttributeInfo.mouthClose = (int*)calloc(FACENUM,sizeof (int));      // 嘴巴是否打开
    pMultiFaceInfo->faceAttributeInfo.rightEyeOpen = (int*)calloc(FACENUM,sizeof (int));    // 右眼是否睁开
    pMultiFaceInfo->faceAttributeInfo.wearGlasses = (int*)calloc(FACENUM,sizeof (int));     // 是否戴眼镜
    // 人脸3d角度
    pMultiFaceInfo->face3DAngleInfo.roll = (float*)calloc(FACENUM,sizeof(float));           // 横滚角
    pMultiFaceInfo->face3DAngleInfo.yaw = (float*)calloc(FACENUM,sizeof(float));            // 偏航角
    pMultiFaceInfo->face3DAngleInfo.pitch = (float*)calloc(FACENUM,sizeof(float));          // 俯仰角
    // 多张人脸信息
    pMultiFaceInfo->faceDataInfoList = (LPASF_FaceDataInfo)calloc(FACENUM,sizeof(ASF_FaceDataInfo));        // 人脸数据列表,人脸数据暂时未申请，大小已经申请


    for (int i=0;i<faceInfo.faceNum;i++)
    {
        pMultiFaceInfo->faceID[i] = faceInfo.faceID[i];                            // 复制人脸ID
        pMultiFaceInfo->faceOrient[i] = faceInfo.faceOrient[i];                    // 拷贝人脸方向数据
        // 拷贝人脸框
        pMultiFaceInfo->faceRect[i].left = faceInfo.faceRect[i].left;
        pMultiFaceInfo->faceRect[i].right = faceInfo.faceRect[i].right;
        pMultiFaceInfo->faceRect[i].top = faceInfo.faceRect[i].top;
        pMultiFaceInfo->faceRect[i].bottom = faceInfo.faceRect[i].bottom;
        // 拷贝人脸属性特征
        pMultiFaceInfo->faceAttributeInfo.leftEyeOpen[i] = faceInfo.faceAttributeInfo.leftEyeOpen[i];
        pMultiFaceInfo->faceAttributeInfo.mouthClose[i] = faceInfo.faceAttributeInfo.mouthClose[i];
        pMultiFaceInfo->faceAttributeInfo.rightEyeOpen[i] = faceInfo.faceAttributeInfo.rightEyeOpen[i];
        pMultiFaceInfo->faceAttributeInfo.wearGlasses[i] = faceInfo.faceAttributeInfo.wearGlasses[i];
        // 拷贝人脸3D角度
        pMultiFaceInfo->face3DAngleInfo.roll[i] = faceInfo.face3DAngleInfo.roll[i];
        pMultiFaceInfo->face3DAngleInfo.yaw[i] = faceInfo.face3DAngleInfo.yaw[i];
        pMultiFaceInfo->face3DAngleInfo.pitch[i] = faceInfo.face3DAngleInfo.pitch[i];

        // 拷贝额头区域
        pMultiFaceInfo->foreheadRect[i].left = faceInfo.foreheadRect[i].left;
        pMultiFaceInfo->foreheadRect[i].right = faceInfo.foreheadRect[i].right;
        pMultiFaceInfo->foreheadRect[i].top = faceInfo.foreheadRect[i].top;
        pMultiFaceInfo->foreheadRect[i].bottom = faceInfo.foreheadRect[i].bottom;
        // 拷贝人脸是否溢出界面
        pMultiFaceInfo->faceIsWithinBoundary[i] = faceInfo.faceIsWithinBoundary[i];  // 对应人脸是否溢出
        // 拷贝人脸脸部数据
        pMultiFaceInfo->faceDataInfoList[i].dataSize = faceInfo.faceDataInfoList[i].dataSize;   // 人脸数据大小
        QByteArray faceData((char*)faceInfo.faceDataInfoList[i].data);
        pMultiFaceInfo->faceDataInfoList[i].data = (void*)calloc(1,faceData.size());        // 申请脸部信息存放的空间
        memcpy(pMultiFaceInfo->faceDataInfoList[i].data,faceInfo.faceDataInfoList[i].data,faceData.size());  // 拷贝人脸信息
    }
}

MutilFace::~MutilFace() {
    for(int i=0;i<pMultiFaceInfo->faceNum;++i){
        free(pMultiFaceInfo->faceDataInfoList[i].data);
    }  // 释放人脸数据的空间
    if(pMultiFaceInfo->faceDataInfoList!=nullptr){
        free(pMultiFaceInfo->faceDataInfoList);
    }   // 释放人脸列表空间，主要是dataSize

    if(pMultiFaceInfo->faceAttributeInfo.leftEyeOpen!=nullptr){
        //人脸属性特征释放空间
        free(pMultiFaceInfo->faceAttributeInfo.leftEyeOpen);
        free(pMultiFaceInfo->faceAttributeInfo.mouthClose);
        free(pMultiFaceInfo->faceAttributeInfo.rightEyeOpen);
        free(pMultiFaceInfo->faceAttributeInfo.wearGlasses);
        // 人脸3D角度释放空间
        free(pMultiFaceInfo->face3DAngleInfo.roll);
        free(pMultiFaceInfo->face3DAngleInfo.yaw);
        free(pMultiFaceInfo->face3DAngleInfo.pitch);
    }
    if(pMultiFaceInfo->faceIsWithinBoundary!=nullptr){
        free(pMultiFaceInfo->faceIsWithinBoundary);
    }   // 释放检测人脸是否溢出空间
    if (pMultiFaceInfo->faceID != nullptr){
        free(pMultiFaceInfo->faceID);
    }  // 释放人脸ID
    if (pMultiFaceInfo->faceOrient != nullptr){
        free(pMultiFaceInfo->faceOrient);
    }  // 释放人脸角度
    if (pMultiFaceInfo->faceRect != nullptr){
        free(pMultiFaceInfo->faceRect);
        free(pMultiFaceInfo->foreheadRect);
    }  // 释放 人脸框空间
    if (pMultiFaceInfo != nullptr) {
        free(pMultiFaceInfo);
    }  // 释放人脸结构体，包括faceNum，以及一些结构体
}



OffScreen::OffScreen(QObject *parent) :QObject(parent)
{
    pOffScreen = (LPASVLOFFSCREEN)calloc(1,sizeof(ASVLOFFSCREEN));   // bassic init
}

OffScreen::OffScreen(ASVLOFFSCREEN oScreen)                         // init image data
{
    pOffScreen = (LPASVLOFFSCREEN)calloc(1,sizeof(ASVLOFFSCREEN));   // bassic init
    pOffScreen->u32PixelArrayFormat = oScreen.u32PixelArrayFormat;   // image format
    pOffScreen->i32Width = oScreen.i32Width;
    pOffScreen->i32Height = oScreen.i32Height;                       // init image w & h

    // RGB model
    pOffScreen->pi32Pitch[0] = oScreen.i32Width*3;                   // 指定了大小为4的数组

    QByteArray bt((char*)oScreen.ppu8Plane[0]);
    MUInt32 len = bt.size();                                //  count size
    pOffScreen->ppu8Plane[0] = (MUInt8*)calloc(1,len);       //   copy data
    memcpy(pOffScreen->ppu8Plane[0],oScreen.ppu8Plane[0],len);

}

OffScreen::~OffScreen()
{
    if(pOffScreen->ppu8Plane[0] != nullptr)
        free(pOffScreen->ppu8Plane[0]);

//    free(pOffScreen->ppu8Plane);

    if(pOffScreen!=nullptr)
        free(pOffScreen);
}
