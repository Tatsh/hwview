#include <QtCore/QProcess>
#include <QtCore/QRegularExpression>

#include "driverinfo.h"

DriverSearchResult findDriverFiles(const QString &driverName) {
    DriverSearchResult result;

    QProcess modinfo;
    modinfo.start(QStringLiteral("modinfo"),
                  {QStringLiteral("-F"), QStringLiteral("filename"), driverName});
    if (modinfo.waitForFinished(3000)) {
        QString filename = QString::fromUtf8(modinfo.readAllStandardOutput()).trimmed();
        if (!filename.isEmpty() && filename != QStringLiteral("(builtin)")) {
            result.paths << filename;
        } else if (filename == QStringLiteral("(builtin)")) {
            result.isBuiltin = true;
        }
    }

    // Also check for related modules (dependencies)
    modinfo.start(QStringLiteral("modinfo"),
                  {QStringLiteral("-F"), QStringLiteral("depends"), driverName});
    if (modinfo.waitForFinished(3000)) {
        QString depends = QString::fromUtf8(modinfo.readAllStandardOutput()).trimmed();
        if (!depends.isEmpty()) {
            QStringList depList = depends.split(QStringLiteral(","), Qt::SkipEmptyParts);
            for (const QString &dep : depList) {
                QProcess depInfo;
                depInfo.start(QStringLiteral("modinfo"),
                              {QStringLiteral("-F"), QStringLiteral("filename"), dep.trimmed()});
                if (depInfo.waitForFinished(3000)) {
                    QString depFile = QString::fromUtf8(depInfo.readAllStandardOutput()).trimmed();
                    if (!depFile.isEmpty() && depFile != QStringLiteral("(builtin)") &&
                        !result.paths.contains(depFile)) {
                        result.paths << depFile;
                    }
                }
            }
        }
    }

    return result;
}

DriverInfo getDriverInfo(const QString &driverPath) {
    DriverInfo info;
    info.filename = driverPath;

    // Extract module name from path
    QString moduleName = driverPath;
    auto lastSlash = moduleName.lastIndexOf(QLatin1Char('/'));
    if (lastSlash >= 0) {
        moduleName = moduleName.mid(lastSlash + 1);
    }
    // Remove .ko, .ko.gz, .ko.xz, .ko.zst extensions
    moduleName.remove(QRegularExpression(QStringLiteral("\\.ko(\\.gz|\\.xz|\\.zst)?$")));

    QProcess modinfo;
    modinfo.start(QStringLiteral("modinfo"), {moduleName});
    if (!modinfo.waitForFinished(3000)) {
        return info;
    }

    QString output = QString::fromUtf8(modinfo.readAllStandardOutput());
    QStringList lines = output.split(QStringLiteral("\n"), Qt::SkipEmptyParts);

    for (const QString &line : lines) {
        auto colonIdx = line.indexOf(QLatin1Char(':'));
        if (colonIdx < 0) {
            continue;
        }

        QString key = line.left(colonIdx).trimmed();
        QString value = line.mid(colonIdx + 1).trimmed();

        if (key == QStringLiteral("filename")) {
            info.filename = value;
        } else if (key == QStringLiteral("version")) {
            info.version = value;
        } else if (key == QStringLiteral("author")) {
            if (info.author.isEmpty()) {
                info.author = value;
            } else {
                info.author += QStringLiteral(", ") + value;
            }
        } else if (key == QStringLiteral("description")) {
            info.description = value;
        } else if (key == QStringLiteral("license")) {
            info.license = value;
        } else if (key == QStringLiteral("srcversion")) {
            info.srcversion = value;
        } else if (key == QStringLiteral("depends")) {
            info.depends = value;
        } else if (key == QStringLiteral("signer")) {
            info.signer = value;
        } else if (key == QStringLiteral("sig_key")) {
            info.sigKey = value;
        }
    }

    return info;
}
