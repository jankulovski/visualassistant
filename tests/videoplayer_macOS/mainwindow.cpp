#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QFileInfo>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <QDebug>
#include <QTMultimedia>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    scene = new QGraphicsScene(this);
    ui->graphicsView->setScene(scene);

    player = new QMediaPlayer(this);

    videoSurface = new VideoSurface(this);
    player->setVideoOutput(videoSurface);

    ui->btnPlay->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    ui->btnPlay->setEnabled(false);

    ui->cBoxEffects->addItems(QStringList() << "None" << "Custom 1" << "Flip" << "Canny" << "Sobel" <<
                              "Erode" << "Dilate" << "MedianBlur" << "Blur" << "Gaussian" << "Laplacian" <<
                              "Qt Mirrored" << "Qt RGB Swapped");

    connect(player, SIGNAL(stateChanged(QMediaPlayer::State)), this, SLOT(setState(QMediaPlayer::State)));
    connect(videoSurface, SIGNAL(frameAvailable(QImage)), this, SLOT(processFrame(QImage)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setState(QMediaPlayer::State state)
{
    if (state != playerState) {
        playerState = state;
    }
}

void MainWindow::on_btnPlay_clicked() {
    switch(playerState) {
    case QMediaPlayer::PausedState:
    case QMediaPlayer::StoppedState:
        ui->btnPlay->setIcon(style()->standardIcon(QStyle::SP_MediaStop));
        ui->btnPlay->setText("Stop");
        player->play();
        break;
    case QMediaPlayer::PlayingState:
        ui->btnPlay->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
        ui->btnPlay->setText("Play");
        player->stop();
        break;
    }

    fraren = 0;
    stime = cv::getTickCount();
}

void MainWindow::on_btnOpen_clicked()
{
    QFileDialog fileDialog(this);
    fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
    fileDialog.setWindowTitle(tr("Open Video"));
    QStringList supportedMimeTypes = player->supportedMimeTypes();

    if (!supportedMimeTypes.isEmpty()) {
        fileDialog.setMimeTypeFilters(supportedMimeTypes);
    }

    fileDialog.setDirectory(QStandardPaths::standardLocations(QStandardPaths::MoviesLocation)
                            .value(0, QDir::homePath()));

    if (fileDialog.exec() == QDialog::Accepted) {
        player->setMedia(fileDialog.selectedUrls().constFirst());
        ui->btnPlay->setEnabled(true);
    }
}

void MainWindow::processFrame(QImage frame)
{
    if(ceffect != ui->cBoxEffects->currentIndex()) {
        _t_avg_t = 0;
        _t_c_f = 0;
        ceffect = ui->cBoxEffects->currentIndex();
    }

    int64 e1 = cv::getTickCount();

    QImage dimg = applyEffect(frame);
    QPixmap image = QPixmap::fromImage(dimg);

    ui->graphicsView->scene()->clear();
    scene->addPixmap(image);
    scene->setSceneRect(image.rect());
    ui->graphicsView->setScene(scene);
    ui->graphicsView->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);

    ++fraren;

    int64 e2 = cv::getTickCount();
    double time = (e2 - e1) / cv::getTickFrequency();

    _t_avg_t += time;
    ++_t_c_f;

    ui->label_sync->setText(QString("%1 : %2 # %3").arg(QString::number( (player->position() / 1000) * 60 ))
                            .arg(QString::number( fraren ))
                            .arg(QString::number( ((player->position() / 1000) * 60) - fraren )));

    ui->label_fps->setText(QString("%1").arg(QString::number( fraren / ((cv::getTickCount() - stime) /
                                                                        cv::getTickFrequency()) )));
    ui->label_ptavg->setText(QString("%1 s.").arg(QString::number(_t_avg_t/_t_c_f)));
    ui->label_ptpf->setText(QString("%1 s.").arg(QString::number(time)));
    ui->label_bfs->setText(QString("%1 %").arg(QString::number(player->bufferStatus())));
    ui->label_pbr->setText(QString("%1").arg(QString::number(player->playbackRate())));
    ui->label_abr->setText(QString("%1").arg(player->metaData(QMediaMetaData::AudioBitRate).toString()));
    ui->label_vbr->setText(QString("%1").arg(player->metaData(QMediaMetaData::VideoBitRate).toString()));
    ui->label_vfr->setText(QString("%1").arg(player->metaData(QMediaMetaData::VideoFrameRate).toString()));
}

cv::Mat lut(1, 256, CV_8U);

void calculate_lut(int intensity) {
    uchar* p = lut.ptr();
    for ( int i = 0; i < 256; ++i)
        p[i] = cv::saturate_cast<uchar>(intensity * 0.01 * i);
}

cv::Mat custom_1(cv::Mat frame)
{
    cv::Mat framegray;

    int kernel = 3;
    int windowSize = 2;
    int constant = 2;
    int median = 3;
    int intensity = 80;

    (windowSize > 3 && windowSize % 2 == 0) ? windowSize++ : windowSize < 3 ? windowSize = 3 : windowSize;
    (kernel > 3 && kernel % 2 == 0) ? kernel++ : kernel < 3 ? kernel = 3 : kernel;
    (median > 3 && median % 2 == 0) ? median++ : median < 3 ? median = 3 : median;

    calculate_lut(intensity);

    cvtColor(frame, framegray, cv::COLOR_BGR2GRAY );

    GaussianBlur(framegray, framegray, cv::Size(kernel, kernel), 0, 0, cv::BORDER_DEFAULT);
    adaptiveThreshold(framegray, framegray, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY_INV,
                      windowSize, constant);

    cv::Mat tmp;
    frame.copyTo(tmp, framegray);
    cv::LUT(tmp, lut, tmp);
    frame.setTo(cv::Scalar(0, 0, 0), framegray);
    add(frame, tmp, frame);

    return frame;
}

inline QImage mat_to_qimage(cv::Mat &mat, QImage::Format format)
{
    return QImage(mat.data, mat.cols, mat.rows,
                  static_cast<int>(mat.step), format);
}

inline cv::Mat qimage_to_mat(QImage &img, int format)
{
    return cv::Mat(img.height(), img.width(),
                   format, img.bits(), img.bytesPerLine());
}

QImage mat_to_qimage(cv::Mat &mat)
{
    if(!mat.empty()){
        switch(mat.type()){
        case CV_8UC3: return mat_to_qimage(mat, QImage::Format_RGB888);
        case CV_8U: return mat_to_qimage(mat, QImage::Format_Indexed8);
        case CV_8UC4: return mat_to_qimage(mat, QImage::Format_ARGB32);
        }
    }
    return {};
}

cv::Mat qimage_to_mat(QImage &img)
{
    if(img.isNull()){
        return cv::Mat();
    }

    switch (img.format()) {
    case QImage::Format_RGB888:{
        auto result = qimage_to_mat(img, CV_8UC3);
        cv::cvtColor(result, result, CV_RGB2BGR);
        return result;
    }
    case QImage::Format_Indexed8:{
        return qimage_to_mat(img, CV_8U);
    }
    case QImage::Format_RGB32:
    case QImage::Format_ARGB32:
    case QImage::Format_ARGB32_Premultiplied:{
        return qimage_to_mat(img, CV_8UC4);
    }
    default:
        break;
    }
    return {};
}

QImage MainWindow::applyEffect(QImage frame)
{
    switch (ui->cBoxEffects->currentIndex()) {
    case 1:
        // Custom 1
    {
        original = qimage_to_mat(frame);
        applied = custom_1(original);
        return mat_to_qimage(applied);
    }
    case 2:
        // Flip
    {
        original = qimage_to_mat(frame);
        original.copyTo(applied);
        flip(original,applied,1);
        return mat_to_qimage(applied);
    }
    case 3:
        // Canny
    {
        cv::Mat pre_canny;
        original = qimage_to_mat(frame);
        cvtColor(original, pre_canny, CV_RGB2GRAY);
        Canny(pre_canny, applied, 0, 0, 3);
        return mat_to_qimage(applied);
    }
    case 4:
        // Sobel
    {
        original = qimage_to_mat(frame);
        Sobel(original, applied,CV_8U,1,0,3,0.4,128);
        return mat_to_qimage(applied);
    }
    case 5:
        // Erode
    {
        original = qimage_to_mat(frame);
        erode(original, applied, cv::Mat());
        return mat_to_qimage(applied);
    }
    case 6:
        // Dilate
    {
        original = qimage_to_mat(frame);
        dilate(original, applied, cv::Mat());
        return mat_to_qimage(applied);
    }
    case 7:
        // MedianBlur
    {
        original = qimage_to_mat(frame);
        medianBlur(original, applied,5);
        return mat_to_qimage(applied);
    }
    case 8:
        // Blur
    {
        original = qimage_to_mat(frame);
        blur(original,applied,cv::Size(5,5));
        return mat_to_qimage(applied);
    }
    case 9:
        // Gaussian
    {
        original = qimage_to_mat(frame);
        GaussianBlur(original, applied, cv::Size(5,5),1.5);
        return mat_to_qimage(applied);
    }
    case 10:
        // Laplacian
    {
        original = qimage_to_mat(frame);
        Laplacian(original, applied, CV_16S, 3);
        return mat_to_qimage(applied);
    }
    case 11:
        // Qt mirrored
    {
        return frame.mirrored(false,true);
    }
    case 12:
        // Qt RGB swapped
    {
        return frame.rgbSwapped();
    }
    case 0:
    default:
        return frame;
    }
}
