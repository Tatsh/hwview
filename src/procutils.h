#pragma once

#include <QFile>
#include <QStringList>

#ifdef Q_OS_LINUX
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
 * @param path The path to the /proc file.
 * @returns List of lines from the file, or empty list on error.
 */
    inline QStringList readProcFile(const QString &path) {
        QFile file(path);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            return QString::fromLocal8Bit(file.readAll()).split(QLatin1Char('\n'));
        }
        return {};
    }

} // namespace procutils
#endif
