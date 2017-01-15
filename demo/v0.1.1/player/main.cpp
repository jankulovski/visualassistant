#include "videoplayer.h"

#include <QApplication>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    VideoPlayer player;
    player.show();

    return app.exec();
}

