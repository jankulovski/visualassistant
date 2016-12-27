TEMPLATE = app
TARGET = player

QT += network \
      xml \
      multimedia \
      multimediawidgets \
      widgets

HEADERS = \
    player.h \
    playercontrols.h \
    playlistmodel.h \
    videowidget.h \
    histogramwidget.h
SOURCES = main.cpp \
    player.cpp \
    playercontrols.cpp \
    playlistmodel.cpp \
    videowidget.cpp \
    histogramwidget.cpp

target.path = $$[QT_INSTALL_EXAMPLES]/multimediawidgets/player
INSTALLS += target

INCLUDEPATH += C:/opencv/release/install/include
LIBS += -LC:/opencv/release/install/x86/mingw/bin
LIBS += -lopencv_core320
LIBS += -lopencv_imgproc320
LIBS += -lopencv_highgui320
LIBS += -lopencv_ml320
LIBS += -lopencv_video320
LIBS += -lopencv_features2d320
LIBS += -lopencv_calib3d320
LIBS += -lopencv_objdetect320
LIBS += -lopencv_imgcodecs320
