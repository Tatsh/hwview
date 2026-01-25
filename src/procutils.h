#pragma once

#include <QtCore/QFile>
#include <QtCore/QStringList>

#include "devicecache.h"

/**
 * @brief Utility functions for reading /proc filesystem files.
 */
namespace procutils {

/**
 * @brief Reads a /proc file and returns its contents as a list of lines.
 *
 * This function uses readAll() because proc files report size 0,
 * which causes QTextStream::atEnd() to return true immediately.
 *
 * When in viewer mode, this returns the cached data from the export file instead.
 *
 * @param path The path to the /proc file.
 * @returns List of lines from the file, or empty list on error.
 */
inline QStringList readProcFile(const QString &path) {
    // In viewer mode, return cached data from the export
    if (DeviceCache::instance().isViewerMode()) {
        const QJsonObject &resources = DeviceCache::instance().systemResources();
        // Map /proc paths to JSON keys
        QString key;
        if (path == QStringLiteral("/proc/dma")) {
            key = QStringLiteral("dma");
        } else if (path == QStringLiteral("/proc/ioports")) {
            key = QStringLiteral("ioports");
        } else if (path == QStringLiteral("/proc/interrupts")) {
            key = QStringLiteral("interrupts");
        } else if (path == QStringLiteral("/proc/iomem")) {
            key = QStringLiteral("iomem");
        }
        if (!key.isEmpty() && resources.contains(key)) {
            return resources[key].toString().split(QLatin1Char('\n'));
        }
        return {};
    }

    QFile file(path);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return QString::fromLocal8Bit(file.readAll()).split(QLatin1Char('\n'));
    }
    return {};
}

} // namespace procutils
