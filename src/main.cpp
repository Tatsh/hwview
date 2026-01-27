// SPDX-License-Identifier: MIT
#include <QtCore/QCommandLineParser>
#include <QtCore/QTextStream>
#include <QtNetwork/QHostInfo>

#ifndef HWVIEW_HEADLESS
#include <QtCore/QSize>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#ifdef HWVIEW_USE_KDE
#include <KAboutData>
#include <KLocalizedString>
#endif // HWVIEW_USE_KDE
#endif // HWVIEW_HEADLESS

#include "deviceexport.h"
#include "deviceinfo.h"
#ifndef HWVIEW_HEADLESS
#include "mainwindow.h"
#endif // HWVIEW_HEADLESS
#include "systeminfo.h"

namespace {

/**
 * @brief Set application metadata.
 */
void setAppMetadata() {
    QCoreApplication::setOrganizationName(QStringLiteral("Tatsh"));
    QCoreApplication::setApplicationName(QStringLiteral("Hardware Viewer"));
    QCoreApplication::setApplicationVersion(QStringLiteral("0.0.1"));
}

/**
 * @brief Set up common command line parser options.
 * @param parser Command line parser to configure.
 */
void setupParser(QCommandLineParser &parser) {
    parser.setApplicationDescription(
        QCoreApplication::translate("main", "View device information."));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addOption({{QStringLiteral("e"), QStringLiteral("export")},
                      QCoreApplication::translate("main", "Export device data to <file> and exit."),
                      QStringLiteral("file")});
}

/**
 * @brief Perform headless export to file.
 * @param filePath Path to export file.
 * @returns 0 on success, 1 on failure.
 */
int performExport(const QString &filePath) {
    QTextStream out(stdout);
    QTextStream err(stderr);

    out << QStringLiteral("Enumerating devices...") << Qt::endl;
    auto devices = enumerateAllDevices();
    out << QStringLiteral("Found %1 devices.").arg(devices.size()) << Qt::endl;

    auto hostname = QHostInfo::localHostName();
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
    QCommandLineParser parser;

#ifdef HWVIEW_HEADLESS
    QCoreApplication app(argc, argv);
    setAppMetadata();
    setupParser(parser);
    parser.process(app);

    auto exportPath = parser.value(QStringLiteral("export"));
    if (exportPath.isEmpty()) {
        QTextStream err(stderr);
        err << QStringLiteral("Error: --export requires a file path.") << Qt::endl;
        return 1;
    }

    // Ensure file has correct extension.
    if (!exportPath.endsWith(QLatin1String(DeviceExport::FILE_EXTENSION), Qt::CaseInsensitive)) {
        exportPath += QLatin1String(DeviceExport::FILE_EXTENSION);
    }

    return performExport(exportPath);
#else
    QApplication app(argc, argv);
    setAppMetadata();

#ifdef HWVIEW_USE_KDE
    KLocalizedString::setApplicationDomain(QByteArrayLiteral("hwview"));
    KAboutData aboutData(QStringLiteral("hwview"),
                         i18n("Hardware Viewer"),
                         QStringLiteral("0.0.1"),
                         i18n("View device information and driver software."),
                         KAboutLicense::MIT,
                         i18n("(c) 2024-2026 Tatsh"));
    aboutData.addAuthor(i18n("Tatsh"),
                        i18n("Developer"),
                        QStringLiteral("audvare@gmail.com"),
                        QStringLiteral(HWVIEW_WEBSITE_URL));
    aboutData.setHomepage(QStringLiteral(HWVIEW_WEBSITE_URL));
    aboutData.setBugAddress(QByteArrayLiteral(HWVIEW_WEBSITE_URL "/issues"));
    aboutData.setOrganizationDomain(QByteArrayLiteral("tat.sh"));
    KAboutData::setApplicationData(aboutData);
#endif // HWVIEW_USE_KDE

    setupParser(parser);
    parser.addPositionalArgument(QStringLiteral("file"),
                                 QCoreApplication::translate("main", "Export file to open."),
                                 QStringLiteral("[file]"));
    parser.process(app);

    // Handle --export option.
    if (parser.isSet(QStringLiteral("export"))) {
        auto exportPath = parser.value(QStringLiteral("export"));
        if (exportPath.isEmpty()) {
            QTextStream err(stderr);
            err << QStringLiteral("Error: --export requires a file path.") << Qt::endl;
            return 1;
        }
        if (!exportPath.endsWith(QLatin1String(DeviceExport::FILE_EXTENSION),
                                 Qt::CaseInsensitive)) {
            exportPath += QLatin1String(DeviceExport::FILE_EXTENSION);
        }
        return performExport(exportPath);
    }

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

    // If a file was passed as an argument, try to open it.
    const auto args = parser.positionalArguments();
    if (!args.isEmpty()) {
        mainWin->loadExportFile(args.first());
    }

    return app.exec();
#endif // HWVIEW_HEADLESS
}
