#ifndef DEVBYTYPEMODEL_H
#define DEVBYTYPEMODEL_H

#include <QtCore/QAbstractItemModel>

#include "node.h"
#include "udev/udevmanager.h"

class DevicesByTypeModel : public QAbstractItemModel {
    Q_OBJECT

public:
    explicit DevicesByTypeModel(QObject *parent = nullptr);
    ~DevicesByTypeModel() override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex
    index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &child) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

private:
    void addAudio();
    void addBatteries();
    void addComputer();
    void addDiskDrives();
    void addDisplayAdapters();
    void addOptical();
    void addHid();
    void addIdeAtapi();
    void addKeyboards();
    void addMice();
    void addMonitors();
    void addNetwork();
    void addPrintQueues();
    void addProcessors();
    void addSoftwareComponents();
    void addSoftwareDevices();
    void addSoundVideoAndGameControllers();
    void addStorageControllers();
    void addStorageVolumes();
    void addSystemDevices();
    void addUsbControllers();

    UdevManager manager;
    Node *rootItem;
    Node *hostnameItem;
    // Categories
    Node *audioInputsAndOutputsItem;
    Node *batteriesItem;
    Node *computerItem;
    Node *diskDrivesItem;
    Node *displayAdaptersItem;
    Node *dvdCdromDrivesItem;
    Node *humanInterfaceDevicesItem;
    Node *ideAtaAtapiControllersItem;
    Node *keyboardsItem;
    Node *miceAndOtherPointingDevicesItem;
    Node *monitorsItem;
    Node *networkAdaptersItem;
    Node *printQueuesItem;
    Node *processorsItem;
    Node *softwareComponentsItem;
    Node *softwareDevicesItem;
    Node *soundVideoAndGameControllersItem;
    Node *storageControllersItem;
    Node *storageVolumesItem;
    Node *systemDevicesItem;
    Node *universalSerialBusControllersItem;
};

#endif // DEVBYTYPEMODEL_H
