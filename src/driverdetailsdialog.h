// SPDX-License-Identifier: MIT
/** @file */
#pragma once

#include <QtCore/QFutureWatcher>
#include <QtCore/QString>
#include <QtGui/QIcon>
#include <QtWidgets/QDialog>
#include <QtWidgets/QLabel>
#include <QtWidgets/QListWidget>

struct DriverSearchResult;

/**
 * @brief Dialog that displays detailed information about a kernel driver.
 */
class DriverDetailsDialog : public QDialog {
    Q_OBJECT

public:
    explicit DriverDetailsDialog(QWidget *parent = nullptr);

    /**
     * @brief Sets the driver name to display details for.
     * @param driverName The kernel module/driver name.
     */
    void setDriverName(const QString &driverName);

    /**
     * @brief Sets the category icon to display in the header.
     * @param icon The icon representing the device category.
     */
    void setCategoryIcon(const QIcon &icon);

private Q_SLOTS:
    void onFileSelectionChanged();
    void onDriverFilesFound();

private:
    void populateDriverFiles();
    void updateFileDetails(const QString &driverPath);

    QString driverName_;
    QIcon categoryIcon_;
    QFutureWatcher<DriverSearchResult> *fileSearchWatcher_ = nullptr;

    QLabel *labelIcon_;
    QLabel *labelDeviceName_;
    QListWidget *listDriverFiles_;
    QLabel *labelProvider_;
    QLabel *labelProviderValue_;
    QLabel *labelFileVersion_;
    QLabel *labelFileVersionValue_;
    QLabel *labelCopyright_;
    QLabel *labelCopyrightValue_;
    QLabel *labelDigitalSigner_;
    QLabel *labelDigitalSignerValue_;
};
