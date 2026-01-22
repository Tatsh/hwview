#include <QApplication>
#include <QIcon>
#include <QSize>

#ifdef DEVMGMT_USE_KDE
#include <KAboutData>
#include <KLocalizedString>
#endif

#include "mainwindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

#ifdef DEVMGMT_USE_KDE
    KLocalizedString::setApplicationDomain(QByteArrayLiteral("devmgmt"));

    KAboutData aboutData(QStringLiteral("devmgmt"),
                         i18n("Device Manager"),
                         QStringLiteral("0.0.1"),
                         i18n("View and manage device hardware settings and driver software."),
                         KAboutLicense::GPL_V3,
                         i18n("(c) 2024-2025 Tatsh"));
    aboutData.addAuthor(i18n("Tatsh"),
                        i18n("Developer"),
                        QStringLiteral("tatsh@tatsh.net"),
                        QStringLiteral(DEVMGMT_WEBSITE_URL));
    aboutData.setHomepage(QStringLiteral(DEVMGMT_WEBSITE_URL));
    aboutData.setBugAddress(QByteArrayLiteral(DEVMGMT_WEBSITE_URL "/issues"));
    aboutData.setOrganizationDomain(QByteArrayLiteral("tatsh.net"));
    KAboutData::setApplicationData(aboutData);
#else
    QCoreApplication::setOrganizationName(QStringLiteral("Tatsh"));
    QCoreApplication::setApplicationName(QStringLiteral("Device Manager"));
    QCoreApplication::setApplicationVersion(QStringLiteral("0.0.1"));
#endif

    QIcon appIcon;
    appIcon.addFile(QStringLiteral(":/icons/icon_16.png"), QSize(16, 16));
    appIcon.addFile(QStringLiteral(":/icons/icon_32.png"), QSize(32, 32));
    appIcon.addFile(QStringLiteral(":/icons/icon_64.png"), QSize(64, 64));
    appIcon.addFile(QStringLiteral(":/icons/icon_128.png"), QSize(128, 128));
    appIcon.addFile(QStringLiteral(":/icons/icon_256.png"), QSize(256, 256));
    appIcon.addFile(QStringLiteral(":/icons/icon_512.png"), QSize(512, 512));
    QApplication::setWindowIcon(appIcon);

    auto *mainWin = new MainWindow;
    mainWin->setAttribute(Qt::WA_DeleteOnClose);
    mainWin->show();
    return app.exec();
}
