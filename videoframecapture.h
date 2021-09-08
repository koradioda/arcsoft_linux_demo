#ifndef VIDEOFRAMECAPTURE_H
#define VIDEOFRAMECAPTURE_H

#include <QAbstractVideoSurface>


class VideoFrameCapture : public QAbstractVideoSurface
{
    Q_OBJECT
public:
    VideoFrameCapture(QObject *parent=0);
    ~VideoFrameCapture();
    QList<QVideoFrame::PixelFormat> supportedPixelFormats(QAbstractVideoBuffer::HandleType type = QAbstractVideoBuffer::NoHandle) const;
    bool present(const QVideoFrame &frame);
signals:
    void frameAvailable(const QVideoFrame &videoFrame);

};

#endif // VIDEOFRAMECAPTURE_H
