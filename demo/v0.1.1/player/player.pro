TEMPLATE = app
TARGET = player

QT += multimedia multimediawidgets

HEADERS   += videoplayer.h \
    videosurface.h

SOURCES   += main.cpp \
             videoplayer.cpp \
    videosurface.cpp

INSTALLS += target

QT+=widgets

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
