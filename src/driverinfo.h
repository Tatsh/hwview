// SPDX-License-Identifier: MIT
/** @file */
#pragma once

#include <QtCore/QString>
#include <QtCore/QStringList>

/**
 * @brief Information about a driver/kernel module.
 */
struct DriverInfo {
    QString filename;    ///< Path to the driver file.
    QString version;     ///< Driver version string.
    QString author;      ///< Driver author or maintainer.
    QString description; ///< Human-readable driver description.
    QString license;     ///< License type (e.g., "GPL").
    QString srcversion;  ///< Source version hash.
    QString alias;       ///< Module alias patterns.
    QString depends;     ///< Comma-separated list of dependencies.
    QString signer;      ///< Module signature signer.
    QString sigKey;      ///< Module signature key identifier.
};

/**
 * @brief Result of searching for driver files.
 */
struct DriverSearchResult {
    QStringList paths;      ///< List of paths to driver files.
    bool isBuiltin = false; ///< Whether the driver is built into the kernel.
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
