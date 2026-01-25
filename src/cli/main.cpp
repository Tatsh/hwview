/**
 * @file main.cpp
 * @brief CLI-only device information exporter.
 */

#include <QtCore/QCommandLineParser>
#include <QtCore/QCoreApplication>
#include <QtCore/QTextStream>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif // _WIN32

#include "deviceexport.h"
#include "deviceinfo.h"
#include "systeminfo.h"

namespace {

/**
 * @brief Get the local hostname without requiring Qt6::Network.
 * @returns The local hostname.
 */
QString getHostname() {
#ifdef _WIN32
    wchar_t buffer[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD size = MAX_COMPUTERNAME_LENGTH + 1;
    if (GetComputerNameW(buffer, &size)) {
        return QString::fromWCharArray(buffer);
    }
    return QStringLiteral("unknown");
#else
    char buffer[256];
    if (gethostname(buffer, sizeof(buffer)) == 0) {
        return QString::fromLocal8Bit(buffer);
    }
    return QStringLiteral("unknown");
#endif // _WIN32
}

} // namespace

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    QCoreApplication::setOrganizationName(QStringLiteral("Tatsh"));
    QCoreApplication::setApplicationName(QStringLiteral("Device Manager Exporter"));
    QCoreApplication::setApplicationVersion(QStringLiteral("0.0.1"));

    QTextStream out(stdout);
    QTextStream err(stderr);

    QCommandLineParser parser;
    parser.setApplicationDescription(
        QCoreApplication::translate("main", "Export device hardware information to a file."));
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption outputOption(
        {QStringLiteral("o"), QStringLiteral("output")},
        QCoreApplication::translate("main", "Output file path (required)."),
        QStringLiteral("file"));
    parser.addOption(outputOption);

    QCommandLineOption quietOption(
        {QStringLiteral("q"), QStringLiteral("quiet")},
        QCoreApplication::translate("main", "Suppress progress output."));
    parser.addOption(quietOption);

    parser.process(app);

    QString outputPath = parser.value(outputOption);
    bool quiet = parser.isSet(quietOption);

    if (outputPath.isEmpty()) {
        err << QStringLiteral("Error: Output file path is required. Use -o or --output.")
            << Qt::endl;
        err << Qt::endl;
        err << parser.helpText();
        return 1;
    }

    // Ensure file has correct extension
    if (!outputPath.endsWith(QLatin1String(DeviceExport::FILE_EXTENSION), Qt::CaseInsensitive)) {
        outputPath += QLatin1String(DeviceExport::FILE_EXTENSION);
    }

    if (!quiet) {
        out << QStringLiteral("Enumerating devices...") << Qt::endl;
    }

    QList<DeviceInfo> devices = enumerateAllDevices();

    if (!quiet) {
        out << QStringLiteral("Found %1 devices.").arg(devices.size()) << Qt::endl;
    }

    QString hostname = getHostname();

    if (!quiet) {
        out << QStringLiteral("Exporting to: %1").arg(outputPath) << Qt::endl;
    }

    if (DeviceExport::exportToFile(outputPath, devices, hostname)) {
        if (!quiet) {
            out << QStringLiteral("Export successful.") << Qt::endl;
        }
        return 0;
    }

    err << QStringLiteral("Error: Failed to write export file.") << Qt::endl;
    return 1;
}
