#pragma once

#include <QFutureWatcher>
#include <QIcon>
#include <QStandardItemModel>
#include <QtWidgets/QDialog>

#include "deviceinfo.h"
#include "ui_propertiesdialog.h"

class PropertiesDialog : public QDialog, private Ui::PropertiesDialog {
    Q_OBJECT

public:
    explicit PropertiesDialog(QWidget *parent = nullptr);
    ~PropertiesDialog() override;
    void setDeviceSyspath(const QString &syspath);
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

    // Resource information structure
    struct ResourceInfo {
        QString type;     // IRQ, Memory Range, I/O Range, DMA
        QString setting;  // The actual value/range
        QString iconName; // Icon for the resource type
    };
    QList<ResourceInfo> getDeviceResources();

    QString syspath_;
    const DeviceInfo *deviceInfo_;
    QIcon categoryIcon_;
    QStandardItemModel *eventsModel_;
    QStringList allEvents_;           // Store all events for "View All Events" dialog
    QWidget *resourcesTab_ = nullptr; // Track resources tab for removal/recreation
    QFutureWatcher<QStringList> *eventsWatcher_ = nullptr; // Async events loading

    // Property name to udev property key mapping
    struct PropertyMapping {
        QString displayName;
        QString propertyKey;
        bool isMultiValue;
    };
    QList<PropertyMapping> propertyMappings_;
};
