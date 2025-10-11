#include <QApplication>
#include <QIcon>
#include "mainwindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setWindowIcon(QIcon(":/icons/appicon.png"));  // from qrc
    MainWindow w;
    w.show();
    return app.exec();
}
