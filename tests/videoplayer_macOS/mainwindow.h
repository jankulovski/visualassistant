#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QtMultimedia/QMediaPlayer>
#include <QtMultimedia/QMediaPlaylist>
#include <QVideoProbe>
#include <opencv2/core.hpp>

#include "videosurface.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_btnOpen_clicked();
    void on_btnPlay_clicked();
    void processFrame(QImage frame);

    QImage applyEffect(QImage frame);

public slots:
    void setState(QMediaPlayer::State state);

signals:
    void play();
    void pause();
    void stop();

private:

    double _t_avg_t = 0;
    int _t_c_f = 0;
    int fraren = 0;
    int ceffect = 0;
    int64 stime = 0;

    QImage Mat2QImage(cv::Mat const& src);
    cv::Mat QImage2Mat(QImage const& src);

    Ui::MainWindow *ui;

    QGraphicsScene *scene;

    cv::Mat original;
    cv::Mat applied;

    QMediaPlayer *player;
    QMediaPlayer::State playerState;

    VideoSurface *videoSurface;

};

#endif // MAINWINDOW_H
