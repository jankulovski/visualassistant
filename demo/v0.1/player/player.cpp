/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "player.h"

#include "playercontrols.h"
#include "playlistmodel.h"

#include <QMediaService>
#include <QMediaPlaylist>
#include <QVideoProbe>
#include <QMediaMetaData>
#include <QtWidgets>

#include "videosurface.h"

Player::Player(QWidget *parent)
    : QWidget(parent)
    , videoWidget(0)
    , coverLabel(0)
    , slider(0)
    , colorDialog(0)
{
//! [create-objs]
    player = new QMediaPlayer(this);
    // owned by PlaylistModel
    playlist = new QMediaPlaylist();
    player->setPlaylist(playlist);
//! [create-objs]

    connect(player, SIGNAL(durationChanged(qint64)), SLOT(durationChanged(qint64)));
    connect(player, SIGNAL(positionChanged(qint64)), SLOT(positionChanged(qint64)));
    connect(player, SIGNAL(metaDataChanged()), SLOT(metaDataChanged()));
    connect(playlist, SIGNAL(currentIndexChanged(int)), SLOT(playlistPositionChanged(int)));
    connect(player, SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)),
            this, SLOT(statusChanged(QMediaPlayer::MediaStatus)));
    connect(player, SIGNAL(bufferStatusChanged(int)), this, SLOT(bufferingProgress(int)));
    connect(player, SIGNAL(videoAvailableChanged(bool)), this, SLOT(videoAvailableChanged(bool)));
    connect(player, SIGNAL(error(QMediaPlayer::Error)), this, SLOT(displayErrorMessage()));

//! [2]
    videoWidget = new VideoWidget(this);
    videoWidget->setVisible(false);
    //player->setVideoOutput(videoWidget);
    VideoSurface* surface = new VideoSurface(this);
    player->setVideoOutput(surface);

    connect(surface, SIGNAL(frameAvailable(QImage)), this, SLOT(processFrame(QImage)));

    playlistModel = new PlaylistModel(this);
    playlistModel->setPlaylist(playlist);
//! [2]

    playlistView = new QListView(this);
    playlistView->setModel(playlistModel);
    playlistView->setCurrentIndex(playlistModel->index(playlist->currentIndex(), 0));
    playlistView->setVisible(false);
    playlistView->hide();

    QGroupBox *customControlsBox_1 = new QGroupBox(this);
    QGroupBox *customControlsBox_2 = new QGroupBox(this);
    QVBoxLayout *customVBox_1 = new QVBoxLayout();
    QVBoxLayout *customVBox_2 = new QVBoxLayout();
    customControlsCombo = new QComboBox(this);
    customControlsCombo->addItem("None");

    customControlsBox_1->setMinimumSize(100, 200);
    customControlsBox_2->setMinimumSize(100, 200);

    customVBox_1->addWidget(customControlsCombo);

    customVBox_1->addStretch(1);
    customControlsBox_1->setLayout(customVBox_1);

    customVBox_2->addStretch(1);
    customControlsBox_2->setLayout(customVBox_2);

    connect(playlistView, SIGNAL(activated(QModelIndex)), this, SLOT(jump(QModelIndex)));

    slider = new QSlider(Qt::Horizontal, this);
    slider->setRange(0, player->duration() / 1000);

    labelDuration = new QLabel(this);
    connect(slider, SIGNAL(sliderMoved(int)), this, SLOT(seek(int)));

    probe = new QVideoProbe(this);
    probe->setSource(player);

    QPushButton *openButton = new QPushButton(tr("Open"), this);

    connect(openButton, SIGNAL(clicked()), this, SLOT(open()));

    PlayerControls *controls = new PlayerControls(this);
    controls->setState(player->state());
    controls->setVolume(player->volume());
    controls->setMuted(controls->isMuted());

    connect(controls, SIGNAL(play()), player, SLOT(play()));
    connect(controls, SIGNAL(pause()), player, SLOT(pause()));
    connect(controls, SIGNAL(stop()), player, SLOT(stop()));
    connect(controls, SIGNAL(next()), playlist, SLOT(next()));
    connect(controls, SIGNAL(previous()), this, SLOT(previousClicked()));
    connect(controls, SIGNAL(changeVolume(int)), player, SLOT(setVolume(int)));
    connect(controls, SIGNAL(changeMuting(bool)), player, SLOT(setMuted(bool)));
    connect(controls, SIGNAL(changeRate(qreal)), player, SLOT(setPlaybackRate(qreal)));

    connect(controls, SIGNAL(stop()), videoWidget, SLOT(update()));

    connect(player, SIGNAL(stateChanged(QMediaPlayer::State)),
            controls, SLOT(setState(QMediaPlayer::State)));
    connect(player, SIGNAL(volumeChanged(int)), controls, SLOT(setVolume(int)));
    connect(player, SIGNAL(mutedChanged(bool)), controls, SLOT(setMuted(bool)));

    connect(customControlsCombo, SIGNAL(currentTextChanged(QString)), this, SLOT(methodChanged(QString)));

    fullScreenButton = new QPushButton(tr("FullScreen"), this);
    fullScreenButton->setCheckable(true);

    colorButton = new QPushButton(tr("Color Options..."), this);
    colorButton->setEnabled(false);
    connect(colorButton, SIGNAL(clicked()), this, SLOT(showColorDialog()));

    QBoxLayout *displayLayout = new QHBoxLayout;

    scene = new QGraphicsScene(this);
    graphicsView = new QGraphicsView(this);
    graphicsView->setScene(scene);
    graphicsView->setMinimumSize(300, 300);
    graphicsView->setBackgroundBrush(QBrush(Qt::black, Qt::SolidPattern));

    displayLayout->addWidget(graphicsView, 2);
    // displayLayout->addWidget(videoWidget, 2);
    // displayLayout->addWidget(playlistView);
    displayLayout->addWidget(customControlsBox_1, 1);
    displayLayout->addWidget(customControlsBox_2, 1);

    QBoxLayout *controlLayout = new QHBoxLayout;
    controlLayout->setMargin(0);
    controlLayout->addWidget(openButton);
    controlLayout->addStretch(1);
    controlLayout->addWidget(controls);
    controlLayout->addStretch(1);
    controlLayout->addWidget(fullScreenButton);
    controlLayout->addWidget(colorButton);

    QBoxLayout *layout = new QVBoxLayout;
    layout->addLayout(displayLayout);
    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->addWidget(slider);
    hLayout->addWidget(labelDuration);
    layout->addLayout(hLayout);
    layout->addLayout(controlLayout);

    setLayout(layout);

    sliders = new QList<QSlider*>();
    slidersLabels = new QList<QLabel*>();

    for(int i=0; i<15; ++i) {

        QSlider * slider = new QSlider(Qt::Horizontal);
        sliders->append(slider);
        slider->hide();

        QLabel *sliderLabel = new QLabel("");
        slidersLabels->append(sliderLabel);
        sliderLabel->hide();

        customVBox_1->addWidget(sliderLabel);
        customVBox_1->addWidget(slider);
    }

    loadSettings("settings.json");

    if (!isPlayerAvailable()) {
        QMessageBox::warning(this, tr("Service not available"),
                             tr("The QMediaPlayer object does not have a valid service.\n"\
                                "Please check the media service plugins are installed."));

        controls->setEnabled(false);
        playlistView->setEnabled(false);
        openButton->setEnabled(false);
        colorButton->setEnabled(false);
        fullScreenButton->setEnabled(false);
    }

    metaDataChanged();
}

Player::~Player()
{
}

void Player::processFrame(const QImage &frame)
{
    QPixmap image = QPixmap::fromImage(applyEffect(frame, customControlsCombo->currentText()));
    graphicsView->scene()->clear();
    scene->addPixmap(image);
    scene->setSceneRect(image.rect());
    graphicsView->setScene(scene);
    graphicsView->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
}

void Player::methodChanged(const QString &method)
{
    if(method != "None") {
        loadMethod(method);
    } else {
        for(int i = 0; i<sliders->length(); ++i) {
            (*slidersLabels)[i]->hide();
            (*sliders)[i]->hide();
        }
    }
}

void Player::loadSettings(const QString &filename)
{
    QFile jsonFile(filename);
    jsonFile.open(QFile::ReadOnly);
    QJsonDocument jsonResponse = QJsonDocument::fromJson(jsonFile.readAll());
    QJsonObject jsonObject = jsonResponse.object();

    settings = jsonObject["methods"].toArray();

    foreach (const QJsonValue & value, settings) {
        QJsonObject obj = value.toObject();
        customControlsCombo->addItem(obj["name"].toString());
    }
    customControlsCombo->setCurrentIndex(0);
}

const QJsonObject Player::getMethodSettings(const QString &method)
{
    foreach (const QJsonValue & setting, settings) {
        QJsonObject obj = setting.toObject();
        if(obj["name"].toString() == method) {
            return obj;
        }
    }
    throw EXCEPTION_ACCESS_VIOLATION;
}

void Player::loadMethod(const QString &method)
{
    QJsonObject obj = getMethodSettings(method);
    QJsonArray params = obj["params"].toArray();
    int index = 0;
    foreach (const QJsonValue & param, params) {
        QJsonObject obj = param.toObject();
        adjustSettingsSlider(index, obj["par_name"].toString(), obj["min"].toInt(), obj["max"].toInt(), obj["default"].toInt());
        ++index;
    }
    for(int i = index; i<sliders->length();++i) {
        (*slidersLabels)[index]->hide();
        (*sliders)[index]->hide();
    }
}

void Player::adjustSettingsSlider(const int index, const QString &sname, const int &min, const int &max, const int &def)
{
    (*slidersLabels)[index]->setText(sname);
    (*slidersLabels)[index]->show();
    (*sliders)[index]->setRange(min, max);
    (*sliders)[index]->setValue(def);
    (*sliders)[index]->show();
}

bool Player::isPlayerAvailable() const
{
    return player->isAvailable();
}

void Player::open()
{
    QFileDialog fileDialog(this);
    fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
    fileDialog.setWindowTitle(tr("Open Files"));
    QStringList supportedMimeTypes = player->supportedMimeTypes();
    if (!supportedMimeTypes.isEmpty()) {
        supportedMimeTypes.append("audio/x-m3u"); // MP3 playlists
        fileDialog.setMimeTypeFilters(supportedMimeTypes);
    }
    fileDialog.setDirectory(QStandardPaths::standardLocations(QStandardPaths::MoviesLocation).value(0, QDir::homePath()));
    if (fileDialog.exec() == QDialog::Accepted)
        addToPlaylist(fileDialog.selectedUrls());
}

static bool isPlaylist(const QUrl &url) // Check for ".m3u" playlists.
{
    if (!url.isLocalFile())
        return false;
    const QFileInfo fileInfo(url.toLocalFile());
    return fileInfo.exists() && !fileInfo.suffix().compare(QLatin1String("m3u"), Qt::CaseInsensitive);
}

void Player::addToPlaylist(const QList<QUrl> urls)
{
    foreach (const QUrl &url, urls) {
        if (isPlaylist(url))
            playlist->load(url);
        else
            playlist->addMedia(url);
    }
}

void Player::durationChanged(qint64 duration)
{
    this->duration = duration/1000;
    slider->setMaximum(duration / 1000);
}

void Player::positionChanged(qint64 progress)
{
    if (!slider->isSliderDown()) {
        slider->setValue(progress / 1000);
    }
    updateDurationInfo(progress / 1000);
}

void Player::metaDataChanged()
{
    if (player->isMetaDataAvailable()) {
        setTrackInfo(QString("%1 - %2")
                .arg(player->metaData(QMediaMetaData::AlbumArtist).toString())
                .arg(player->metaData(QMediaMetaData::Title).toString()));

        if (coverLabel) {
            QUrl url = player->metaData(QMediaMetaData::CoverArtUrlLarge).value<QUrl>();

            coverLabel->setPixmap(!url.isEmpty()
                    ? QPixmap(url.toString())
                    : QPixmap());
        }
    }
}

void Player::previousClicked()
{
    // Go to previous track if we are within the first 5 seconds of playback
    // Otherwise, seek to the beginning.
    if(player->position() <= 5000)
        playlist->previous();
    else
        player->setPosition(0);
}

void Player::jump(const QModelIndex &index)
{
    if (index.isValid()) {
        playlist->setCurrentIndex(index.row());
        player->play();
    }
}

void Player::playlistPositionChanged(int currentItem)
{
    playlistView->setCurrentIndex(playlistModel->index(currentItem, 0));
}

void Player::seek(int seconds)
{
    player->setPosition(seconds * 1000);
}

void Player::statusChanged(QMediaPlayer::MediaStatus status)
{
    handleCursor(status);

    // handle status message
    switch (status) {
    case QMediaPlayer::UnknownMediaStatus:
    case QMediaPlayer::NoMedia:
    case QMediaPlayer::LoadedMedia:
    case QMediaPlayer::BufferingMedia:
    case QMediaPlayer::BufferedMedia:
        setStatusInfo(QString());
        break;
    case QMediaPlayer::LoadingMedia:
        setStatusInfo(tr("Loading..."));
        break;
    case QMediaPlayer::StalledMedia:
        setStatusInfo(tr("Media Stalled"));
        break;
    case QMediaPlayer::EndOfMedia:
        QApplication::alert(this);
        break;
    case QMediaPlayer::InvalidMedia:
        displayErrorMessage();
        break;
    }
}

void Player::handleCursor(QMediaPlayer::MediaStatus status)
{
#ifndef QT_NO_CURSOR
    if (status == QMediaPlayer::LoadingMedia ||
        status == QMediaPlayer::BufferingMedia ||
        status == QMediaPlayer::StalledMedia)
        setCursor(QCursor(Qt::BusyCursor));
    else
        unsetCursor();
#endif
}

void Player::bufferingProgress(int progress)
{
    setStatusInfo(tr("Buffering %4%").arg(progress));
}

void Player::videoAvailableChanged(bool available)
{
    if (!available) {
        disconnect(fullScreenButton, SIGNAL(clicked(bool)),
                    videoWidget, SLOT(setFullScreen(bool)));
        disconnect(videoWidget, SIGNAL(fullScreenChanged(bool)),
                fullScreenButton, SLOT(setChecked(bool)));
        videoWidget->setFullScreen(false);
    } else {
        connect(fullScreenButton, SIGNAL(clicked(bool)),
                videoWidget, SLOT(setFullScreen(bool)));
        connect(videoWidget, SIGNAL(fullScreenChanged(bool)),
                fullScreenButton, SLOT(setChecked(bool)));

        if (fullScreenButton->isChecked())
            videoWidget->setFullScreen(true);
    }
    colorButton->setEnabled(available);
}

void Player::setTrackInfo(const QString &info)
{
    trackInfo = info;
    if (!statusInfo.isEmpty())
        setWindowTitle(QString("%1 | %2").arg(trackInfo).arg(statusInfo));
    else
        setWindowTitle(trackInfo);
}

void Player::setStatusInfo(const QString &info)
{
    statusInfo = info;
    if (!statusInfo.isEmpty())
        setWindowTitle(QString("%1 | %2").arg(trackInfo).arg(statusInfo));
    else
        setWindowTitle(trackInfo);
}

void Player::displayErrorMessage()
{
    setStatusInfo(player->errorString());
}

void Player::updateDurationInfo(qint64 currentInfo)
{
    QString tStr;
    if (currentInfo || duration) {
        QTime currentTime((currentInfo/3600)%60, (currentInfo/60)%60, currentInfo%60, (currentInfo*1000)%1000);
        QTime totalTime((duration/3600)%60, (duration/60)%60, duration%60, (duration*1000)%1000);
        QString format = "mm:ss";
        if (duration > 3600)
            format = "hh:mm:ss";
        tStr = currentTime.toString(format) + " / " + totalTime.toString(format);
    }
    labelDuration->setText(tStr);
}

void Player::showColorDialog()
{
    if (!colorDialog) {
        QSlider *brightnessSlider = new QSlider(Qt::Horizontal);
        brightnessSlider->setRange(-100, 100);
        brightnessSlider->setValue(videoWidget->brightness());
        connect(brightnessSlider, SIGNAL(sliderMoved(int)), videoWidget, SLOT(setBrightness(int)));
        connect(videoWidget, SIGNAL(brightnessChanged(int)), brightnessSlider, SLOT(setValue(int)));

        QSlider *contrastSlider = new QSlider(Qt::Horizontal);
        contrastSlider->setRange(-100, 100);
        contrastSlider->setValue(videoWidget->contrast());
        connect(contrastSlider, SIGNAL(sliderMoved(int)), videoWidget, SLOT(setContrast(int)));
        connect(videoWidget, SIGNAL(contrastChanged(int)), contrastSlider, SLOT(setValue(int)));

        QSlider *hueSlider = new QSlider(Qt::Horizontal);
        hueSlider->setRange(-100, 100);
        hueSlider->setValue(videoWidget->hue());
        connect(hueSlider, SIGNAL(sliderMoved(int)), videoWidget, SLOT(setHue(int)));
        connect(videoWidget, SIGNAL(hueChanged(int)), hueSlider, SLOT(setValue(int)));

        QSlider *saturationSlider = new QSlider(Qt::Horizontal);
        saturationSlider->setRange(-100, 100);
        saturationSlider->setValue(videoWidget->saturation());
        connect(saturationSlider, SIGNAL(sliderMoved(int)), videoWidget, SLOT(setSaturation(int)));
        connect(videoWidget, SIGNAL(saturationChanged(int)), saturationSlider, SLOT(setValue(int)));

        QFormLayout *layout = new QFormLayout;
        layout->addRow(tr("Brightness"), brightnessSlider);
        layout->addRow(tr("Contrast"), contrastSlider);
        layout->addRow(tr("Hue"), hueSlider);
        layout->addRow(tr("Saturation"), saturationSlider);

        QPushButton *button = new QPushButton(tr("Close"));
        layout->addRow(button);

        colorDialog = new QDialog(this);
        colorDialog->setWindowTitle(tr("Color Options"));
        colorDialog->setLayout(layout);

        connect(button, SIGNAL(clicked()), colorDialog, SLOT(close()));
    }
    colorDialog->show();
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

QImage Player::applyEffect(QImage frame, const QString method)
{
    if(method == "None") {
        return frame;
    }

    original = qimage_to_mat(frame);

    if(method == "Detail1") {
        applied = custom_1(original);
    } else if(method == "Detail 2") {
        //
    } else if(method == "Detail 3") {
        //
    }

    return mat_to_qimage(applied);
}
