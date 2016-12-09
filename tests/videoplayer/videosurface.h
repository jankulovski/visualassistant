#ifndef VIDEOSURFACE_H
#define VIDEOSURFACE_H

#include <QtWidgets>
#include <QAbstractVideoSurface>

class VideoSurface : public QAbstractVideoSurface
{
    Q_OBJECT
public:
    VideoSurface(QObject *parent = 0);

    QList<QVideoFrame::PixelFormat> supportedPixelFormats(
            QAbstractVideoBuffer::HandleType handleType = QAbstractVideoBuffer::NoHandle) const;

    bool present(const QVideoFrame &frame);


signals:
    void frameAvailable(QImage frame);
};

#endif // VIDEOSURFACE_H
