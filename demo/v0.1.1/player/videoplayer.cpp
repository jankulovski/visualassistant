#include "videoplayer.h"
#include "videosurface.h"
#include "sharpcontrast.h"
#include "neonedge.h"

class InvalidMethodException : public QException
{
public:
    void raise() const { throw *this; }
    InvalidMethodException *clone() const { return new InvalidMethodException(*this); }
};

class InvalidSettingException : public QException
{
public:
    void raise() const { throw *this; }
    InvalidSettingException *clone() const { return new InvalidSettingException(*this); }
};

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

VideoPlayer::VideoPlayer(QWidget *parent)
    : QWidget(parent)
    , mediaPlayer(0, QMediaPlayer::VideoSurface)
    , videoItem(0)
    , playButton(0)
    , positionSlider(0)
{
    videoItem = new QGraphicsVideoItem;
    videoItem->setSize(QSizeF(640, 480));

    scene = new QGraphicsScene(this);
    graphicsView = new QGraphicsView(scene);

    graphicsView->setOptimizationFlag(QGraphicsView::DontClipPainter);
    graphicsView->setOptimizationFlags(QGraphicsView::DontSavePainterState);
    graphicsView->setOptimizationFlags(QGraphicsView::DontAdjustForAntialiasing);
    graphicsView->setCacheMode(QGraphicsView::CacheBackground);

    scene->addItem(videoItem);

    QAbstractButton *openButton = new QPushButton(tr("Open..."));
    connect(openButton, SIGNAL(clicked()), this, SLOT(openFile()));

    playButton = new QPushButton;
    playButton->setEnabled(false);
    playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));

    connect(playButton, SIGNAL(clicked()),
            this, SLOT(play()));

    positionSlider = new QSlider(Qt::Horizontal);
    positionSlider->setRange(0, 0);

    connect(positionSlider, SIGNAL(sliderMoved(int)),
            this, SLOT(setPosition(int)));

    QBoxLayout *controlLayout = new QHBoxLayout;
    controlLayout->setMargin(0);
    controlLayout->addWidget(openButton);
    controlLayout->addWidget(playButton);
    controlLayout->addWidget(positionSlider);

    QBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(graphicsView);
    layout->addLayout(controlLayout);

    setLayout(layout);

    connect(&mediaPlayer, SIGNAL(stateChanged(QMediaPlayer::State)),
            this, SLOT(mediaStateChanged(QMediaPlayer::State)));
    connect(&mediaPlayer, SIGNAL(positionChanged(qint64)), this, SLOT(positionChanged(qint64)));
    connect(&mediaPlayer, SIGNAL(durationChanged(qint64)), this, SLOT(durationChanged(qint64)));

    QGroupBox *methodSettingsControlsBox_1 = new QGroupBox(this);
    QGroupBox *opticalSettingsControlsBox_2 = new QGroupBox(this);
    methodSettingsVBox_1 = new QVBoxLayout();
    opticalSettingsVBox_1 = new QVBoxLayout();

    methodsControlsCombo = new QComboBox(this);
    methodsControlsCombo->addItem("None");

    opticsControlsCombo = new QComboBox(this);
    opticsControlsCombo->addItem("None");

    methodSettingsControlsBox_1->setMinimumSize(100, 200);
    opticalSettingsControlsBox_2->setMinimumSize(100, 200);

    controlLayout->addWidget(opticsControlsCombo);
    controlLayout->addWidget(methodsControlsCombo);

    methodSettingsVBox_1->addStretch(1);
    methodSettingsControlsBox_1->setLayout(methodSettingsVBox_1);

    opticalSettingsVBox_1->addStretch(1);
    opticalSettingsControlsBox_2->setLayout(opticalSettingsVBox_1);

    controlLayout->addWidget(opticalSettingsControlsBox_2,1);
    controlLayout->addWidget(methodSettingsControlsBox_1,1);

    VideoSurface* surface = new VideoSurface(this);
    mediaPlayer.setVideoOutput(surface);
    connect(surface, SIGNAL(frameAvailable(QImage)), this, SLOT(processFrame(QImage)));

    connect(methodsControlsCombo, SIGNAL(currentTextChanged(QString)), this, SLOT(methodChanged(QString)));
    connect(opticsControlsCombo, SIGNAL(currentTextChanged(QString)), this, SLOT(opticsChanged(QString)));

    loadSettings("/Users/kiko/Desktop/visualassistant/demo/v0.1.1/player/MethodSettings.json");
    loadSettingsOptics("/Users/kiko/Desktop/visualassistant/demo/v0.1.1/player/OpticalSettings.json");
}

VideoPlayer::~VideoPlayer()
{
}


void VideoPlayer::openFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Movie"),QDir::homePath());

    if (!fileName.isEmpty()) {
        mediaPlayer.setMedia(QUrl::fromLocalFile(fileName));
        playButton->setEnabled(true);
    }
}

void VideoPlayer::play()
{
    switch(mediaPlayer.state()) {
    case QMediaPlayer::PlayingState:
        mediaPlayer.pause();
        break;
    default:
        mediaPlayer.play();
        break;
    }
}

void VideoPlayer::mediaStateChanged(QMediaPlayer::State state)
{
    switch(state) {
    case QMediaPlayer::PlayingState:
        playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
        break;
    default:
        playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
        break;
    }
}

void VideoPlayer::positionChanged(qint64 position)
{
    positionSlider->setValue(position);
}

void VideoPlayer::durationChanged(qint64 duration)
{
    positionSlider->setRange(0, duration);
}

void VideoPlayer::setPosition(int position)
{
    mediaPlayer.setPosition(position);
}

void VideoPlayer::processFrame(QImage frame)
{
    applied = qimage_to_mat(frame);

    if(isPreProcessNeeded && opticsControlsCombo->currentText() != "None") {
        applied = preProcessFrame(applied, opticsControlsCombo->currentText());
    }

    if(methodsControlsCombo->currentText() != "None") {
        applied = postProcessFrame(applied, methodsControlsCombo->currentText());
    }

    QPixmap image = QPixmap::fromImage(mat_to_qimage(applied));

    graphicsView->scene()->clear();
    scene->addPixmap(image);
    scene->setSceneRect(image.rect());
    graphicsView->setScene(scene);
    graphicsView->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
}

void VideoPlayer::loadSettings(const QString &filename)
{
    QFile jsonFile(filename);
    jsonFile.open(QFile::ReadOnly);
    QJsonDocument jsonResponse = QJsonDocument::fromJson(jsonFile.readAll());
    QJsonObject jsonObject = jsonResponse.object();

    settings = jsonObject["methods"].toArray();

    foreach (const QJsonValue & value, settings) {
        QJsonObject obj = value.toObject();
        methodsControlsCombo->addItem(obj["name"].toString());
    }
    methodsControlsCombo->setCurrentIndex(0);
}

void VideoPlayer::loadSettingsOptics(const QString &filename)
{
    QFile jsonFile(filename);
    jsonFile.open(QFile::ReadOnly);
    QJsonDocument jsonResponse = QJsonDocument::fromJson(jsonFile.readAll());
    QJsonObject jsonObject = jsonResponse.object();

    settingsOptics = jsonObject["methods"].toArray();

    foreach (const QJsonValue & value, settingsOptics) {
        QJsonObject obj = value.toObject();
        opticsControlsCombo->addItem(obj["name"].toString());
    }
    opticsControlsCombo->setCurrentIndex(0);
}

void VideoPlayer::cleanSettingsLayout(QLayout* layout)
{
    QLayoutItem* child;
    while(layout->count()!=0)
    {
        child = layout->takeAt(0);
        if(child->layout() != 0)
        {
            cleanSettingsLayout(child->layout());
        }
        else if(child->widget() != 0)
        {
            delete child->widget();
        }

        delete child;
    }
}

void VideoPlayer::loadMethodSettings(const QString &method)
{
    if(method == "None") {
        cleanSettingsLayout(methodSettingsVBox_1);
        return;
    }

    try {
        QJsonObject obj = getMethodSettings(method);
        QJsonArray params = obj["params"].toArray();

        cleanSettingsLayout(methodSettingsVBox_1);

        foreach (const QJsonValue & param, params) {
            QJsonObject obj = param.toObject();

            QSlider * slider = new QSlider(Qt::Horizontal);
            slider->setObjectName(obj["par_name"].toString());
            methodSettingsUi[obj["par_name"].toString()] = slider;

            QLabel *sliderLabel = new QLabel(obj["par_name"].toString());

            methodSettingsVBox_1->addWidget(sliderLabel);
            methodSettingsVBox_1->addWidget(slider);

            adjustMethodSettingsSlider(obj["par_name"].toString(), obj["min"].toInt(), obj["max"].toInt(), obj["default"].toInt());
        }
    } catch(InvalidMethodException e) {}
}

int VideoPlayer::getMSetting(const QString &name)
{
    if(methodSettingsUi.contains(name)) {
        return (*methodSettingsUi[name]).value();
    }
    return 0;
}

int VideoPlayer::getOSetting(const QString &name)
{
    if(opticalSettingsUi.contains(name)) {
        return (*opticalSettingsUi[name]).value();
    }
    return 0;
}

void VideoPlayer::updatePreProcessNeeded()
{
    for(auto e : opticalSettingsUi.keys())
    {
        if( (*opticalSettingsUi.value(e)).value() != (*opticalSettingsUi.value(e)).minimum() ) {
            isPreProcessNeeded = true;
            return;
        }
    }
    isPreProcessNeeded = false;
}

void VideoPlayer::loadOpticSettings(const QString &method)
{
    if(method == "None") {
        cleanSettingsLayout(opticalSettingsVBox_1);
        updatePreProcessNeeded();
        return;
    }

    try {
        QJsonObject obj = getOpticsSettings(method);
        QJsonArray params = obj["params"].toArray();

        cleanSettingsLayout(opticalSettingsVBox_1);

        foreach (const QJsonValue & param, params) {
            QJsonObject obj = param.toObject();

            QSlider * slider = new QSlider(Qt::Horizontal);
            slider->setObjectName(obj["par_name"].toString());
            opticalSettingsUi[obj["par_name"].toString()] = slider;

            connect(slider, SIGNAL(valueChanged(int)), this, SLOT(updatePreProcessNeeded()));

            QLabel *sliderLabel = new QLabel(obj["par_name"].toString());

            opticalSettingsVBox_1->addWidget(sliderLabel);
            opticalSettingsVBox_1->addWidget(slider);

            adjustOpticalSettingsSlider(obj["par_name"].toString(), obj["min"].toInt(), obj["max"].toInt(), obj["default"].toInt());
        }

        updatePreProcessNeeded();

    } catch(InvalidMethodException e) {}
}

void VideoPlayer::adjustMethodSettingsSlider(const QString &sname, const int &min, const int &max, const int &def)
{
    if(methodSettingsUi.contains(sname)) {
        (*methodSettingsUi[sname]).setRange(min, max);
        (*methodSettingsUi[sname]).setValue(def);
    }
}

void VideoPlayer::adjustOpticalSettingsSlider(const QString &sname, const int &min, const int &max, const int &def)
{
    if(opticalSettingsUi.contains(sname)) {
        (*opticalSettingsUi[sname]).setRange(min, max);
        (*opticalSettingsUi[sname]).setValue(def);
    }
}

const QJsonObject VideoPlayer::getMethodSettings(const QString &method)
{
    foreach (const QJsonValue & setting, settings) {
        QJsonObject obj = setting.toObject();
        if(obj["name"].toString() == method) {
            return obj;
        }
    }
    throw InvalidMethodException();
}

const QJsonObject VideoPlayer::getOpticsSettings(const QString &method)
{
    foreach (const QJsonValue & setting, settingsOptics) {
        QJsonObject obj = setting.toObject();
        if(obj["name"].toString() == method) {
            return obj;
        }
    }
    throw InvalidMethodException();
}

void VideoPlayer::methodChanged(const QString &method)
{
    loadMethodSettings(method);
}

void VideoPlayer::opticsChanged(const QString &optic)
{
    loadOpticSettings(optic);
}

cv::Mat VideoPlayer::preProcessFrame(cv::Mat frame, const QString &method)
{
    if(method == "SharpContrast") {
        return SharpContrast(frame, getOSetting("DarkLight"), getOSetting("Intensity"),
                             getOSetting("Vibrance"), getOSetting("Sharpness"), getOSetting("Contrast"));
    }
    else {
        return frame;
    }
}

cv::Mat VideoPlayer::postProcessFrame(cv::Mat frame, const QString &method)
{
    if(method == "NeonEdge") {
        return NeonEdge(frame, getMSetting("intensity"), getMSetting("kernel"), getMSetting("weight"),
                        getMSetting("scale"), getMSetting("cut"), getMSetting("hue"));
    }
    else {
        return frame;
    }
}

