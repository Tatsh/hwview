#include <QtCore/QDir>
#include <QtCore/QDirIterator>
#include <QtCore/QFile>
#include <QtCore/QProcess>
#include <QtCore/QSettings>

#include "driverinfo.h"

namespace {

QString readBundleIdentifier(const QString &kextPath) {
    auto plistPath = QString(kextPath + QStringLiteral("/Contents/Info.plist"));
    if (!QFile::exists(plistPath)) {
        plistPath = QString(kextPath + QStringLiteral("/Info.plist"));
    }
    if (QFile::exists(plistPath)) {
        QSettings plist(plistPath, QSettings::NativeFormat);
        return plist.value(QStringLiteral("CFBundleIdentifier")).toString();
    }
    return {};
}

} // namespace

DriverSearchResult findDriverFiles(const QString &driverName) {
    DriverSearchResult result;

    static const QStringList searchDirs = {
        QStringLiteral("/System/Library/Extensions"),
        QStringLiteral("/Library/Extensions"),
        QStringLiteral("/System/Library/DriverExtensions"),
        QStringLiteral("/Library/DriverExtensions"),
    };

    // Check if it looks like a bundle identifier
    if (driverName.contains(QLatin1Char('.'))) {
        // First try quick match: derive kext name from bundle ID
        auto kextName = driverName.section(QLatin1Char('.'), -1) + QStringLiteral(".kext");
        for (const auto &searchDir : searchDirs) {
            auto directPath = searchDir + QLatin1Char('/') + kextName;
            if (QDir(directPath).exists()) {
                auto foundId = readBundleIdentifier(directPath);
                if (foundId == driverName) {
                    result.paths << directPath;
                }
            }
        }

        // If quick match failed, do full search
        if (result.paths.isEmpty()) {
            for (const auto &searchDir : searchDirs) {
                QDir dir(searchDir);
                if (!dir.exists()) {
                    continue;
                }
                QDirIterator it(searchDir,
                                {QStringLiteral("*.kext"), QStringLiteral("*.dext")},
                                QDir::Dirs | QDir::NoDotAndDotDot);
                while (it.hasNext()) {
                    auto kextPath = it.next();
                    auto foundId = readBundleIdentifier(kextPath);
                    if (foundId == driverName && !result.paths.contains(kextPath)) {
                        result.paths << kextPath;
                    }
                }
            }
        }
    } else {
        // Driver name might be a kext name directly
        auto kextName = driverName;
        if (!kextName.endsWith(QStringLiteral(".kext"))) {
            kextName += QStringLiteral(".kext");
        }
        for (const auto &searchDir : searchDirs) {
            auto path = searchDir + QLatin1Char('/') + kextName;
            if (QDir(path).exists()) {
                result.paths << path;
            }
        }
    }

    // If no paths found, it might be a built-in driver
    if (result.paths.isEmpty()) {
        result.isBuiltin = true;
    }

    return result;
}

DriverInfo getDriverInfo(const QString &kextPath) {
    DriverInfo info;
    info.filename = kextPath;

    // Read Info.plist from the kext bundle
    auto plistPath = QString(kextPath + QStringLiteral("/Contents/Info.plist"));
    if (!QFile::exists(plistPath)) {
        plistPath = QString(kextPath + QStringLiteral("/Info.plist"));
    }

    if (QFile::exists(plistPath)) {
        QSettings plist(plistPath, QSettings::NativeFormat);

        info.version = plist.value(QStringLiteral("CFBundleShortVersionString")).toString();
        if (info.version.isEmpty()) {
            info.version = plist.value(QStringLiteral("CFBundleVersion")).toString();
        }

        info.description = plist.value(QStringLiteral("CFBundleName")).toString();
        info.license = plist.value(QStringLiteral("NSHumanReadableCopyright")).toString();

        // Try to get author from bundle identifier
        auto bundleId = plist.value(QStringLiteral("CFBundleIdentifier")).toString();
        if (bundleId.startsWith(QStringLiteral("com.apple."))) {
            info.author = QStringLiteral("Apple Inc.");
        } else if (bundleId.startsWith(QStringLiteral("com.nvidia."))) {
            info.author = QStringLiteral("NVIDIA Corporation");
        } else if (bundleId.startsWith(QStringLiteral("com.amd."))) {
            info.author = QStringLiteral("AMD, Inc.");
        } else if (bundleId.startsWith(QStringLiteral("com.intel."))) {
            info.author = QStringLiteral("Intel Corporation");
        } else {
            // Extract organization from bundle ID (second component)
            auto parts = bundleId.split(QLatin1Char('.'));
            if (parts.size() >= 2) {
                auto org = parts[1];
                if (!org.isEmpty()) {
                    org[0] = org[0].toUpper();
                    info.author = org;
                }
            }
        }
    }

    // Get code signing info using codesign
    QProcess codesign;
    codesign.start(QStringLiteral("codesign"),
                   {QStringLiteral("-dv"), QStringLiteral("--verbose=2"), kextPath});
    if (codesign.waitForFinished(3000)) {
        auto output = QString::fromUtf8(codesign.readAllStandardError());
        auto lines = output.split(QLatin1Char('\n'), Qt::SkipEmptyParts);

        for (const auto &line : lines) {
            if (line.startsWith(QStringLiteral("Authority="))) {
                if (info.signer.isEmpty()) {
                    info.signer = line.mid(10);
                }
            } else if (line.startsWith(QStringLiteral("TeamIdentifier="))) {
                info.sigKey = line.mid(15);
            }
        }
    }

    return info;
}
