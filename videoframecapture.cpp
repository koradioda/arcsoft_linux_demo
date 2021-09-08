#include "videoframecapture.h"

VideoFrameCapture::VideoFrameCapture(QObject *parent):
    QAbstractVideoSurface(parent)
{

}

// 处理相关的视频帧数据,必须重写（此处就是重写后的代码），用来获取视频帧并发送给
bool VideoFrameCapture:: present(const QVideoFrame &frame)
{
    if(frame.isValid())  // 判断获取的帧是不是有效
    {
        QVideoFrame videoFrame(frame);          // 复制原始帧的样本，因为无法对原始帧直接操作
        videoFrame.map(QAbstractVideoBuffer::ReadOnly);  // she zhi 以只读的形式放入内存

        emit frameAvailable(videoFrame);        // ,fa chu xin hao 直接发出视频帧
        videoFrame.unmap();                     // shi fang nei cun de de zhen

        return true;
    }
    return false;
}

// 所支持的图片格式
QList<QVideoFrame::PixelFormat> VideoFrameCapture:: supportedPixelFormats(QAbstractVideoBuffer::HandleType type) const
{
    Q_UNUSED(type);
    return QList<QVideoFrame::PixelFormat> ()
            << QVideoFrame::Format_ARGB32
            << QVideoFrame::Format_ARGB32_Premultiplied
            << QVideoFrame::Format_RGB32
            << QVideoFrame::Format_RGB24
            << QVideoFrame::Format_RGB565
            << QVideoFrame::Format_RGB555
            << QVideoFrame::Format_ARGB8565_Premultiplied
            << QVideoFrame::Format_BGRA32
            << QVideoFrame::Format_BGRA32_Premultiplied
            << QVideoFrame::Format_BGR32
            << QVideoFrame::Format_BGR24
            << QVideoFrame::Format_BGR565
            << QVideoFrame::Format_BGR555
            << QVideoFrame::Format_BGRA5658_Premultiplied
            << QVideoFrame::Format_AYUV444
            << QVideoFrame::Format_AYUV444_Premultiplied
            << QVideoFrame::Format_YUV444
            << QVideoFrame::Format_YUV420P
            << QVideoFrame::Format_YV12
            << QVideoFrame::Format_UYVY
            << QVideoFrame::Format_YUYV
            << QVideoFrame::Format_NV12
            << QVideoFrame::Format_NV21
            << QVideoFrame::Format_IMC1
            << QVideoFrame::Format_IMC2
            << QVideoFrame::Format_IMC3
            << QVideoFrame::Format_IMC4
            << QVideoFrame::Format_Y8
            << QVideoFrame::Format_Y16
            << QVideoFrame::Format_Jpeg
            << QVideoFrame::Format_CameraRaw
            << QVideoFrame::Format_AdobeDng;
}

VideoFrameCapture::~VideoFrameCapture()
{

}
