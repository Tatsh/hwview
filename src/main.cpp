#include <QApplication>
#include <QCommandLineParser>
#include <QHostInfo>
#include <QIcon>
#include <QSize>
#include <QTextStream>

#ifdef DEVMGMT_USE_KDE
#include <KAboutData>
#include <KLocalizedString>
#endif

#include "deviceexport.h"
#include "deviceinfo.h"
#include "mainwindow.h"
#include "systeminfo.h"

namespace {

/**
 * @brief Perform headless export to file.
 * @param filePath Path to export file.
 * @returns 0 on success, 1 on failure.
 */
int performExport(const QString &filePath) {
    QTextStream out(stdout);
    QTextStream err(stderr);

    out << QStringLiteral("Enumerating devices...") << Qt::endl;
    QList<DeviceInfo> devices = enumerateAllDevices();
    out << QStringLiteral("Found %1 devices.").arg(devices.size()) << Qt::endl;

    QString hostname = QHostInfo::localHostName();
    out << QStringLiteral("Exporting to: %1").arg(filePath) << Qt::endl;

    if (DeviceExport::exportToFile(filePath, devices, hostname)) {
        out << QStringLiteral("Export successful.") << Qt::endl;
        return 0;
    }
    err << QStringLiteral("Error: Failed to write export file.") << Qt::endl;
    return 1;
}

} // namespace

int main(int argc, char *argv[]) {
    // Check for --export option before creating QApplication
    // This allows headless operation without display
    for (int i = 1; i < argc; ++i) {
        if (qstrcmp(argv[i], "--export") == 0 || qstrcmp(argv[i], "-e") == 0) {
            // Headless mode - use QCoreApplication
            QCoreApplication app(argc, argv);
            QCoreApplication::setOrganizationName(QStringLiteral("Tatsh"));
            QCoreApplication::setApplicationName(QStringLiteral("Device Manager"));
            QCoreApplication::setApplicationVersion(QStringLiteral("0.0.1"));

            QCommandLineParser parser;
            parser.setApplicationDescription(
                QCoreApplication::translate("main", "View and manage device hardware settings."));
            parser.addHelpOption();
            parser.addVersionOption();

            QCommandLineOption exportOption(
                {QStringLiteral("e"), QStringLiteral("export")},
                QCoreApplication::translate("main", "Export device data to <file> and exit."),
                QStringLiteral("file"));
            parser.addOption(exportOption);

            parser.process(app);

            QString exportPath = parser.value(exportOption);
            if (exportPath.isEmpty()) {
                QTextStream err(stderr);
                err << QStringLiteral("Error: --export requires a file path.") << Qt::endl;
                return 1;
            }

            // Ensure file has correct extension
            if (!exportPath.endsWith(QLatin1String(DeviceExport::FILE_EXTENSION),
                                     Qt::CaseInsensitive)) {
                exportPath += QLatin1String(DeviceExport::FILE_EXTENSION);
            }

            return performExport(exportPath);
        }
    }

    // GUI mode
    QApplication app(argc, argv);

#ifdef DEVMGMT_USE_KDE
    KLocalizedString::setApplicationDomain(QByteArrayLiteral("devmgmt"));
    KAboutData aboutData(QStringLiteral("devmgmt"),
                         i18n("Device Manager"),
                         QStringLiteral("0.0.1"),
                         i18n("View and manage device hardware settings and driver software."),
                         KAboutLicense::MIT,
                         i18n("(c) 2024-2026 Tatsh"));
    aboutData.addAuthor(i18n("Tatsh"),
                        i18n("Developer"),
                        QStringLiteral("tatsh@tatsh.net"),
                        QStringLiteral(DEVMGMT_WEBSITE_URL));
    aboutData.setHomepage(QStringLiteral(DEVMGMT_WEBSITE_URL));
    aboutData.setBugAddress(QByteArrayLiteral(DEVMGMT_WEBSITE_URL "/issues"));
    aboutData.setOrganizationDomain(QByteArrayLiteral("tat.sh"));
    KAboutData::setApplicationData(aboutData);
#else
    QCoreApplication::setOrganizationName(QStringLiteral("Tatsh"));
    QCoreApplication::setApplicationName(QStringLiteral("Device Manager"));
    QCoreApplication::setApplicationVersion(QStringLiteral("0.0.1"));
#endif

    // Parse command line arguments
    QCommandLineParser parser;
    parser.setApplicationDescription(
        QCoreApplication::translate("main", "View and manage device hardware settings."));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument(QStringLiteral("file"),
                                 QCoreApplication::translate("main", "Export file to open."),
                                 QStringLiteral("[file]"));
    parser.process(app);

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

    // If a file was passed as an argument, try to open it
    const QStringList args = parser.positionalArguments();
    if (!args.isEmpty()) {
        mainWin->loadExportFile(args.first());
    }

    return app.exec();
}
