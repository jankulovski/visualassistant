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

#include <opencv/cv.hpp>
#include <opencv/highgui.h>

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
    void processFrame(QImage frame);
    void methodChanged(const QString &method);
    void opticsChanged(const QString &optic);
    void updatePreProcessNeeded();

private:
    QMediaPlayer mediaPlayer;
    QGraphicsVideoItem *videoItem;
    QAbstractButton *playButton;
    QSlider *positionSlider;

    QGraphicsScene *scene;
    QGraphicsView *graphicsView;

    QComboBox *methodsControlsCombo;
    QComboBox *opticsControlsCombo;

    QJsonArray settings;
    QJsonArray settingsOptics;

    QMap<QString, QSlider*> methodSettingsUi;
    QMap<QString, QSlider*> opticalSettingsUi;

    QImage Mat2QImage(cv::Mat const& src);
    QImage applyEffect(QImage frame, const QString method);
    cv::Mat postProcessFrame(cv::Mat frame, const QString &method);
    cv::Mat preProcessFrame(cv::Mat frame, const QString &method);

    cv::Mat QImage2Mat(QImage const& src);
    cv::Mat original;
    cv::Mat applied;

    QVBoxLayout *methodSettingsVBox_1;
    QVBoxLayout *opticalSettingsVBox_1;

    void loadSettings(const QString &filename);
    void loadSettingsOptics(const QString &filename);
    void loadMethodSettings(const QString &method);
    void loadOpticSettings(const QString &method);
    void adjustMethodSettingsSlider(const QString &sname, const int &min, const int &max, const int &def);
    void adjustOpticalSettingsSlider(const QString &sname, const int &min, const int &max, const int &def);
    void cleanSettingsLayout(QLayout* layout);

    const QJsonObject getMethodSettings(const QString &method);
    const QJsonObject getOpticsSettings(const QString &method);

    int getMSetting(const QString &name);
    int getOSetting(const QString &name);

    bool isPreProcessNeeded = false;
};

#endif

