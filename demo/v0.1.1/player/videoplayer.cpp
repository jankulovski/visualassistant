#include "videoplayer.h"
#include "videosurface.h"

class InvalidMethodException : public QException
{
public:
    void raise() const { throw *this; }
    InvalidMethodException *clone() const { return new InvalidMethodException(*this); }
};

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

    //mediaPlayer.setVideoOutput(videoItem);

    connect(&mediaPlayer, SIGNAL(stateChanged(QMediaPlayer::State)),
            this, SLOT(mediaStateChanged(QMediaPlayer::State)));
    connect(&mediaPlayer, SIGNAL(positionChanged(qint64)), this, SLOT(positionChanged(qint64)));
    connect(&mediaPlayer, SIGNAL(durationChanged(qint64)), this, SLOT(durationChanged(qint64)));

    // additional

    QGroupBox *methodSettingsControlsBox_1 = new QGroupBox(this);
    QGroupBox *opticalSettingsControlsBox_2 = new QGroupBox(this);
    QVBoxLayout *methodSettingsVBox_1 = new QVBoxLayout();
    QVBoxLayout *opticalSettingsVBox_2 = new QVBoxLayout();

    methodsControlsCombo = new QComboBox(this);
    methodsControlsCombo->addItem("None");

    methodSettingsControlsBox_1->setMinimumSize(100, 200);
    opticalSettingsControlsBox_2->setMinimumSize(100, 200);

    methodSettingsVBox_1->addWidget(methodsControlsCombo);

    methodSettingsVBox_1->addStretch(1);
    methodSettingsControlsBox_1->setLayout(methodSettingsVBox_1);

    opticalSettingsVBox_2->addStretch(1);
    opticalSettingsControlsBox_2->setLayout(opticalSettingsVBox_2);

    controlLayout->addWidget(methodSettingsControlsBox_1,1);
    controlLayout->addWidget(opticalSettingsControlsBox_2,1);

    VideoSurface* surface = new VideoSurface(this);
    mediaPlayer.setVideoOutput(surface);
    connect(surface, SIGNAL(frameAvailable(QImage)), this, SLOT(processFrame(QImage)));
    connect(methodsControlsCombo, SIGNAL(currentTextChanged(QString)), this, SLOT(methodChanged(QString)));

    QList<QString> methodSets;
    methodSets << "Gamma" << "Color" << "Sharpness" << "Details" << "Contrast";

    for(int i=0; i < methodSets.length(); ++i) {
        QSlider * slider = new QSlider(Qt::Horizontal);
        methodSettingsUi[methodSets[i]] = slider;

        QLabel *sliderLabel = new QLabel(methodSets[i]);

        methodSettingsVBox_1->addWidget(sliderLabel);
        methodSettingsVBox_1->addWidget(slider);
    }

    loadSettings("/Users/kiko/Desktop/player/MethodSettings.json");
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

void VideoPlayer::processFrame(const QImage &frame)
{
    QPixmap image = QPixmap::fromImage(applyFrameEffect(frame, methodsControlsCombo->currentText()));
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

void VideoPlayer::loadMethodSettings(const QString &method)
{
    try {
        QJsonObject obj = getMethodSettings(method);
        QJsonArray params = obj["params"].toArray();
        foreach (const QJsonValue & param, params) {
            QJsonObject obj = param.toObject();
            adjustMethodSettingsSlider(obj["par_name"].toString(), obj["min"].toInt(), obj["max"].toInt(), obj["default"].toInt());
        }
    } catch(InvalidMethodException e) {

    }
}

void VideoPlayer::adjustMethodSettingsSlider(const QString &sname, const int &min, const int &max, const int &def)
{
    if(methodSettingsUi.contains(sname)) {
        (*methodSettingsUi[sname]).setRange(min, max);
        (*methodSettingsUi[sname]).setValue(def);
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

void VideoPlayer::methodChanged(const QString &method)
{
    loadMethodSettings(method);
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

QImage VideoPlayer::applyFrameEffect(QImage frame, const QString &method)
{
    if(method == "none") {
        return frame;
    }

    original = qimage_to_mat(frame);

    if(method == "Detail1") {
        applied = custom_1(original);
    }
    else {
        applied = original;
    }

    return mat_to_qimage(applied);
}
