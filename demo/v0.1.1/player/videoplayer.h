#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H

#include <QMediaPlayer>
#include <QMovie>
#include <QWidget>
#include <QtWidgets>
#include <QVideoSurfaceFormat>
#include <QGraphicsVideoItem>
#include <QJsonDocument>
#include <QJsonArray>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

QT_BEGIN_NAMESPACE
class QAbstractButton;
class QSlider;
class QGraphicsVideoItem;
QT_END_NAMESPACE


class VideoPlayer : public QWidget
{
    Q_OBJECT

public:
    VideoPlayer(QWidget *parent = 0);
    ~VideoPlayer();

    QSize sizeHint() const { return QSize(800, 600); }

public slots:
    void openFile();
    void play();

private slots:
    void mediaStateChanged(QMediaPlayer::State state);
    void positionChanged(qint64 position);
    void durationChanged(qint64 duration);
    void setPosition(int position);
    void processFrame(const QImage &frame);
    void methodChanged(const QString &method);

private:
    QMediaPlayer mediaPlayer;
    QGraphicsVideoItem *videoItem;
    QAbstractButton *playButton;
    QSlider *positionSlider;

    QGraphicsScene *scene;
    QGraphicsView *graphicsView;

    QComboBox *methodsControlsCombo;

    QJsonArray settings;

    QMap<QString, QSlider*> methodSettingsUi;
    QMap<QString, QSlider*> opticalSettingsUi;

    QImage Mat2QImage(cv::Mat const& src);
    QImage applyEffect(QImage frame, const QString method);
    QImage applyFrameEffect(QImage frame, const QString &method);

    cv::Mat QImage2Mat(QImage const& src);
    cv::Mat original;
    cv::Mat applied;

    void loadSettings(const QString &filename);
    void loadMethodSettings(const QString &method);
    void adjustMethodSettingsSlider(const QString &sname, const int &min, const int &max, const int &def);

    const QJsonObject getMethodSettings(const QString &method);


};

#endif

