#ifndef VIEWLABEL_H
#define VIEWLABEL_H

/*####################################################
######################################################
##########  专门用于绘制显示图片和识别结果的类   ###########
######################################################
######################################################*/

#include <QLabel>
#include <QWidget>
#include <QLabel>
#include <QPoint>
#include <QPainter>
#include <QString>

class ViewLabel : public QLabel
{
    Q_OBJECT
public:
    ViewLabel(QWidget *parent = 0);

private:

protected:
    void paintEvent(QPaintEvent *event);
    void upDate();

private slots:
    void recvCamData(const QImage &image,const QRect &rect,const QString &strFd);
    void recvDetectedDataFd(const QImage &image,const QRect &rect,const QString &strFd);
    void recvDetectedDataLr(const QString &strLr);
    void recvDetectedVideoData(const QImage &image,const QRect &rect,const QString &strFd);
    void recvGuiImage(const QImage &image);

signals:
    void canPaint(const bool &bFd,const bool &Lr);

private:
    QImage mImage;
    QRect mRect;
    QString mStrFd;
    QString mStrLr;
    bool bFd=false;
    bool bLr=false;
};

#endif // VIEWLABEL_H
