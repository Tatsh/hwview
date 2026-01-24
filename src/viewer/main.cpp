#include <QApplication>
#include <QIcon>
#include <QSize>

#include "viewermainwindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QCoreApplication::setOrganizationName(QStringLiteral("Tatsh"));
    QCoreApplication::setApplicationName(QStringLiteral("Device Manager Viewer"));
    QCoreApplication::setApplicationVersion(QStringLiteral("0.0.1"));

    // Use the same icon as the main app
    QIcon appIcon;
    appIcon.addFile(QStringLiteral(":/icons/icon_16.png"), QSize(16, 16));
    appIcon.addFile(QStringLiteral(":/icons/icon_32.png"), QSize(32, 32));
    appIcon.addFile(QStringLiteral(":/icons/icon_64.png"), QSize(64, 64));
    appIcon.addFile(QStringLiteral(":/icons/icon_128.png"), QSize(128, 128));
    appIcon.addFile(QStringLiteral(":/icons/icon_256.png"), QSize(256, 256));
    appIcon.addFile(QStringLiteral(":/icons/icon_512.png"), QSize(512, 512));
    QApplication::setWindowIcon(appIcon);

    auto *mainWin = new ViewerMainWindow;
    mainWin->setAttribute(Qt::WA_DeleteOnClose);
    mainWin->show();

    // Immediately prompt to open a file
    if (!mainWin->openFile()) {
        // User cancelled - exit the application
        return 0;
    }

    return app.exec();
}
