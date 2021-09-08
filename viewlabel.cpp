#include "viewlabel.h"

ViewLabel::ViewLabel(QWidget *parent):QLabel(parent)
{

}

void ViewLabel::paintEvent(QPaintEvent *event)
{

    QLabel::paintEvent(event);
    // 画出图像
    QPainter painter(this);   //
    if (mImage.isNull())
        return;

    // 将图像按比例缩放成和窗口一样大小
    QImage img = mImage.scaled(this->size(), Qt::KeepAspectRatio);       // necessary,保证图形能在 label 中显示
    int x = this->width()/2 - img.width()/2;
    int y = this->height()/2 - img.height()/2;          // x,y is the (left,top) of pic start point
    painter.drawImage(QPoint(x, y), img);               // paint picture at mid,居中显示画图

    // 画矩形框
    QPainter paint;
    paint.begin(this);
    int height = mRect.bottom()-mRect.top();
    int widget = mRect.right()-mRect.left();  // 保存临时的矩形框数据

    paint.setPen(QPen(Qt::red, 1, Qt::SolidLine, Qt::RoundCap));        // 设置画笔
    paint.drawRect(mRect.left()+x,mRect.top()+y,widget,height);         // 人脸框的位置

    if((mRect.top()+y-40)<5){
        paint.drawText(x,20+y,mStrFd+mStrLr);       // 画在 label 的左上方
    }
    else{
        QRect rt={mRect.left()+x,mRect.top()+y-40,130,40};
        QTextOption option(Qt::AlignLeft | Qt::AlignVCenter);
        option.setWrapMode(QTextOption::WordWrap);
        paint.drawText(rt,mStrFd+mStrLr,option);                            // 信息画在人脸框的上方
    }
    paint.end();
//    paint.drawText(mRect.left()+x,mRect.top()+y,mStrFd+mStrLr);         // 画在矩形框上方

}

void ViewLabel::upDate()
{
    update();
    bFd = false;
    bLr = false;
}

void ViewLabel::recvCamData(const QImage &image, const QRect &rect, const QString &strFd)
{
    mImage = image;
    mRect = rect;
    mStrFd = strFd;
    if(strFd.isEmpty())
        mStrLr="";
    update();
}

void ViewLabel::recvDetectedDataFd(const QImage &image,const QRect &rect,const QString &strFd)   //  update painter's parameters
{
    mImage = image;
    mRect = rect;
    mStrFd = strFd;
    bFd = true;
    if(bFd && bLr)
        upDate();

}

void ViewLabel::recvDetectedDataLr(const QString &strLr)
{
    mStrLr = strLr;
    bLr = true;
    if(bFd && bLr)
        upDate();
}

void ViewLabel::recvDetectedVideoData(const QImage &image, const QRect &rect, const QString &strFd)
{
    mImage = image;
    mRect = rect;
    mStrFd = strFd;
    mStrLr = "";
    update();
}

void ViewLabel::recvGuiImage(const QImage &image)
{
    mImage = image;
    mRect = QRect();
    mStrFd = "";
    mStrLr ="";
    update();
}


