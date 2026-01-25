#pragma once

#include <QString>
#include <QStringList>

/**
 * @brief Information about a driver/kernel module.
 */
struct DriverInfo {
    QString filename;
    QString version;
    QString author;
    QString description;
    QString license;
    QString srcversion;
    QString alias;
    QString depends;
    QString signer;
    QString sigKey;
};

/**
 * @brief Result of searching for driver files.
 */
struct DriverSearchResult {
    QStringList paths;
    bool isBuiltin = false;
};

/**
 * @brief Find driver files for a given driver name.
 * @param driverName The driver/module name to search for.
 * @returns Search result containing paths and builtin status.
 */
DriverSearchResult findDriverFiles(const QString &driverName);

/**
 * @brief Get detailed information about a driver file.
 * @param driverPath Path to the driver file.
 * @returns DriverInfo structure with details.
 */
DriverInfo getDriverInfo(const QString &driverPath);
