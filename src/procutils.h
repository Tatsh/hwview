#pragma once

#include <QFile>
#include <QStringList>

#ifdef Q_OS_LINUX
namespace procutils {
    // Read a /proc file and return its contents as a list of lines.
    // Uses readAll() because proc files report size 0, which causes
    // QTextStream::atEnd() to return true immediately.
    inline QStringList readProcFile(const QString &path) {
        QFile file(path);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            return QString::fromLocal8Bit(file.readAll()).split(QLatin1Char('\n'));
        }
        return {};
    }
} // namespace procutils
#endif
