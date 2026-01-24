#pragma once

#include <QFutureWatcher>
#include <QIcon>
#include <QStandardItemModel>
#include <QtWidgets/QDialog>

#include "deviceinfo.h"
#include "ui_propertiesdialog.h"

/**
 * @brief Dialog that displays detailed properties of a device.
 *
 * This dialog shows comprehensive information about a device across
 * multiple tabs: General, Driver, Details, Events, and Resources.
 * It provides access to device properties, driver information, and
 * system resource allocations.
 */
class PropertiesDialog : public QDialog, private Ui::PropertiesDialog {
    Q_OBJECT

public:
    /**
     * @brief Constructs a PropertiesDialog.
     * @param parent Optional parent widget.
     */
    explicit PropertiesDialog(QWidget *parent = nullptr);
    ~PropertiesDialog() override;

    /**
     * @brief Sets the device to display properties for.
     * @param syspath The system path of the device.
     */
    void setDeviceSyspath(const QString &syspath);

    /**
     * @brief Sets the category icon to display in the header.
     * @param icon The icon representing the device category.
     */
    void setCategoryIcon(const QIcon &icon);

private Q_SLOTS:
    void onPropertySelectionChanged(int index);
    void onEventSelectionChanged(const QModelIndex &current, const QModelIndex &previous);
    void onDriverDetailsClicked();
    void onViewAllEventsClicked();
    void onCopyDevicePath();
    void onEventsLoaded();

private:
    void populateGeneralTab();
    void populateDriverTab();
    void populateDetailsTab();
    void populateEventsTab();
    void createResourcesTab();
    QString getKernelVersion();
    QString getKernelBuildDate();
    QString lookupUsbVendor(const QString &vendorId);
    QString translateLocation(const QString &devpath);
    QString getDeviceCategory();
    QString getBlockDeviceManufacturer();
    QString getMountPoint();

    struct ResourceInfo {
        QString type;
        QString setting;
        QString iconName;
    };
    QList<ResourceInfo> getDeviceResources();

    QString syspath_;
    const DeviceInfo *deviceInfo_;
    QIcon categoryIcon_;
    QStandardItemModel *eventsModel_;
    QStringList allEvents_;
    QWidget *resourcesTab_ = nullptr;
    QFutureWatcher<QStringList> *eventsWatcher_ = nullptr;

    struct PropertyMapping {
        QString displayName;
        QString propertyKey;
        bool isMultiValue;
    };
    QList<PropertyMapping> propertyMappings_;
};
