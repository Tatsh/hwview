#pragma once

#include <QDialog>
#include <QIcon>
#include <QJsonObject>

QT_BEGIN_NAMESPACE
class QLabel;
class QListWidget;
QT_END_NAMESPACE

/**
 * @brief Dialog that displays driver file details from exported data.
 *
 * This is a read-only version that displays driver information
 * stored in the export file.
 */
class ViewerDriverDetailsDialog : public QDialog {
    Q_OBJECT

public:
    explicit ViewerDriverDetailsDialog(QWidget *parent = nullptr);

    /**
     * @brief Sets the driver name to display.
     * @param driverName The kernel module/driver name.
     */
    void setDriverName(const QString &driverName);

    /**
     * @brief Sets the driver info from exported data.
     * @param driverInfo The JSON object containing driver details.
     */
    void setDriverInfo(const QJsonObject &driverInfo);

    /**
     * @brief Sets the category icon to display in the header.
     * @param icon The icon representing the device category.
     */
    void setCategoryIcon(const QIcon &icon);

private:
    void populateDetails();

    QString driverName_;
    QJsonObject driverInfo_;
    QIcon categoryIcon_;

    QLabel *labelIcon_;
    QLabel *labelDeviceName_;
    QListWidget *listDriverFiles_;
    QLabel *labelProviderValue_;
    QLabel *labelFileVersionValue_;
    QLabel *labelCopyrightValue_;
    QLabel *labelDigitalSignerValue_;
};
