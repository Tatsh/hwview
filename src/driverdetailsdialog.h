#pragma once

#include <QDialog>
#include <QIcon>
#include <QLabel>
#include <QListWidget>
#include <QString>
#include <QStringList>

/**
 * @brief Dialog that displays detailed information about a kernel driver.
 *
 * This dialog shows driver file information including version, author,
 * license, dependencies, and digital signature status.
 */
class DriverDetailsDialog : public QDialog {
    Q_OBJECT

public:
    /**
     * @brief Constructs a DriverDetailsDialog.
     * @param parent Optional parent widget.
     */
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

private:
    void populateDriverFiles();
    void updateFileDetails(const QString &modulePath);

    struct ModuleInfo {
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

    ModuleInfo getModuleInfo(const QString &moduleName);

    QString driverName_;
    QIcon categoryIcon_;

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
