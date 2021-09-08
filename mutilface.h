#ifndef MUTILFACE_H
#define MUTILFACE_H

#include <QObject>
#include "inc/arcsoft_face_sdk.h"

#define FACENUM 5

// 多张人脸信息类
class MutilFace : public QObject
{
    Q_OBJECT
public:
    explicit MutilFace(QObject *parent = nullptr);
    ~MutilFace();
    MutilFace(ASF_MultiFaceInfo);       //  shenqing xianggaun deneicun

private:

public:
    LPASF_MultiFaceInfo pMultiFaceInfo;     // multi faceInfo
};



// 单张图片数据
class OffScreen:public QObject
{
    Q_OBJECT
public:
    explicit OffScreen(QObject *parent = nullptr);
    OffScreen(ASVLOFFSCREEN);
    ~OffScreen();


private:

public:
    LPASVLOFFSCREEN pOffScreen;


};

#endif // MUTILFACE_H
