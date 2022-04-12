#include <QtWidgets/QApplication>

#include "mainwindow.h"

#define kAppVersion "0.0.1"

int main(int argc, char *argv[]) {
    // Q_INIT_RESOURCE(application);
    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName(QStringLiteral("Tatsh"));
    QCoreApplication::setApplicationName(QStringLiteral("Device Manager"));
    QCoreApplication::setApplicationVersion(QStringLiteral(kAppVersion));
    MainWindow mainWin;
    mainWin.show();
    return app.exec();
}
