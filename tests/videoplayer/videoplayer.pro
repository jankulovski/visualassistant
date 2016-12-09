#-------------------------------------------------
#
# Project created by QtCreator 2016-12-01T15:30:22
#
#-------------------------------------------------

QT       += core gui \
            network \
            xml \
            multimedia \
            multimediawidgets \
            widgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = videoplayer
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    videosurface.cpp

HEADERS  += mainwindow.h \
    videosurface.h

FORMS    += mainwindow.ui

mac {
    INCLUDEPATH += /usr/local/include/opencv
    INCLUDEPATH += /usr/local/include/opencv2
    INCLUDEPATH += /usr/local/include

    LIBS += -L/usr/local/lib
    LIBS += -L/usr/local/include
    LIBS += -lopencv_core
    LIBS += -lopencv_imgproc
    LIBS += -lopencv_highgui
    LIBS += -lopencv_ml
    LIBS += -lopencv_video
    LIBS += -lopencv_features2d
    LIBS += -lopencv_calib3d
    LIBS += -lopencv_objdetect
    LIBS += -lopencv_contrib
    LIBS += -lopencv_legacy
    LIBS += -lopencv_flann
    LIBS += -lopencv_nonfree
    LIBS += -lopencv_legacy
} win32 {

}
