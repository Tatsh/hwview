#include <QtCore/QFileInfo>
#include <QtCore/QRegularExpression>

#include "const_strings.h"
#include "devicecache.h"
#include "models/devbytypemodel.h"

namespace s = strings;
namespace us = strings::udev;

DevicesByTypeModel::DevicesByTypeModel(QObject *parent)
    : BaseTreeModel(parent), hostnameItem(nullptr), audioInputsAndOutputsItem(nullptr),
      batteriesItem(nullptr), computerItem(nullptr), diskDrivesItem(nullptr),
      displayAdaptersItem(nullptr), dvdCdromDrivesItem(nullptr), humanInterfaceDevicesItem(nullptr),
      keyboardsItem(nullptr), miceAndOtherPointingDevicesItem(nullptr),
      networkAdaptersItem(nullptr), softwareDevicesItem(nullptr),
      soundVideoAndGameControllersItem(nullptr), storageControllersItem(nullptr),
      storageVolumesItem(nullptr), systemDevicesItem(nullptr),
      universalSerialBusControllersItem(nullptr) {
    auto *root = new Node({s::empty(), s::empty()});
    setRootItem(root);
    hostnameItem = new Node({DeviceCache::hostname(), s::empty()}, root);
    hostnameItem->setIcon(s::categoryIcons::computer());
    root->appendChild(hostnameItem);
    buildTree();
}

QVariant DevicesByTypeModel::decorationData(Node *item, int column) const {
    // Only show icons in the first column (Name)
    if (column != 0) {
        return {};
    }
    if (item->isHidden()) {
        // Return cached greyed-out pixmap for hidden devices
        return item->disabledPixmap(16);
    }
    return item->icon();
}

void DevicesByTypeModel::finalizeCategory(Node *&category) {
    if (category && category->childCount() > 0) {
        category->sortChildren();
        hostnameItem->appendChild(category);
    } else if (category) {
        delete category;
        category = nullptr;
    }
}

void DevicesByTypeModel::buildTree() {
    // Create all category nodes upfront (with two columns for Name and Driver)
    audioInputsAndOutputsItem =
        new Node({tr("Audio inputs and outputs"), s::empty()}, hostnameItem);
    audioInputsAndOutputsItem->setIcon(s::categoryIcons::audioInputs());

    batteriesItem = new Node({tr("Batteries"), s::empty()}, hostnameItem);
    batteriesItem->setIcon(s::categoryIcons::batteries());

    computerItem = new Node({tr("Computer"), s::empty()}, hostnameItem);
    computerItem->setIcon(s::categoryIcons::computer());

    diskDrivesItem = new Node({tr("Disk drives"), s::empty()}, hostnameItem);
    diskDrivesItem->setIcon(s::categoryIcons::diskDrives());

    displayAdaptersItem = new Node({tr("Display adapters"), s::empty()}, hostnameItem);
    displayAdaptersItem->setIcon(s::categoryIcons::displayAdapters());

    dvdCdromDrivesItem = new Node({tr("DVD/CD-ROM drives"), s::empty()}, hostnameItem);
    dvdCdromDrivesItem->setIcon(s::categoryIcons::dvdCdromDrives());

    humanInterfaceDevicesItem = new Node({tr("Human Interface Devices"), s::empty()}, hostnameItem);
    humanInterfaceDevicesItem->setIcon(s::categoryIcons::hid());

    keyboardsItem = new Node({tr("Keyboards"), s::empty()}, hostnameItem);
    keyboardsItem->setIcon(s::categoryIcons::keyboards());

    miceAndOtherPointingDevicesItem =
        new Node({tr("Mice and other pointing devices"), s::empty()}, hostnameItem);
    miceAndOtherPointingDevicesItem->setIcon(s::categoryIcons::mice());

    networkAdaptersItem = new Node({tr("Network adapters"), s::empty()}, hostnameItem);
    networkAdaptersItem->setIcon(s::categoryIcons::networkAdapters());

    softwareDevicesItem = new Node({tr("Software devices"), s::empty()}, hostnameItem);
    softwareDevicesItem->setIcon(s::categoryIcons::other());

    soundVideoAndGameControllersItem =
        new Node({tr("Sound, video and game controllers"), s::empty()}, hostnameItem);
    soundVideoAndGameControllersItem->setIcon(s::categoryIcons::soundVideoGameControllers());

    storageControllersItem = new Node({tr("Storage controllers"), s::empty()}, hostnameItem);
    storageControllersItem->setIcon(s::categoryIcons::storageControllers());

    storageVolumesItem = new Node({tr("Storage volumes"), s::empty()}, hostnameItem);
    storageVolumesItem->setIcon(s::categoryIcons::storageVolumes());

    systemDevicesItem = new Node({tr("System devices"), s::empty()}, hostnameItem);
    systemDevicesItem->setIcon(s::categoryIcons::systemDevices());

    universalSerialBusControllersItem =
        new Node({tr("Universal Serial Bus controllers"), s::empty()}, hostnameItem);
    universalSerialBusControllersItem->setIcon(s::categoryIcons::usbControllers());

    // Add computer info - determine system type based on firmware
    QString computerName;
#ifdef Q_OS_LINUX
    QFileInfo acpiInfo(QStringLiteral("/sys/firmware/acpi"));
    QFileInfo dtInfo(QStringLiteral("/sys/firmware/devicetree"));

    if (acpiInfo.exists() && acpiInfo.isDir()) {
#if defined(Q_PROCESSOR_X86_64)
        computerName = tr("ACPI x64-based PC");
#elif defined(Q_PROCESSOR_X86_32)
        computerName = tr("ACPI x86-based PC");
#elif defined(Q_PROCESSOR_ARM_64)
        computerName = tr("ACPI ARM64-based PC");
#elif defined(Q_PROCESSOR_ARM)
        computerName = tr("ACPI ARM-based PC");
#else
        computerName = tr("ACPI-based PC");
#endif
    } else if (dtInfo.exists() && dtInfo.isDir()) {
        computerName = tr("Device Tree-based System");
    } else {
        computerName = tr("Standard PC");
    }
#else
    computerName = tr("Standard PC");
#endif

    auto *acpiNode = new Node({computerName, s::empty()}, computerItem, NodeType::Device);
#ifdef Q_OS_LINUX
    acpiNode->setSyspath(QStringLiteral("/sys/devices/virtual/dmi/id"));
#endif
    acpiNode->setIcon(computerItem->icon());
    computerItem->appendChild(acpiNode);

    static const QRegularExpression kBeginningWithSlashDevRe(QStringLiteral("^/dev/"));

    bool showHidden = DeviceCache::instance().showHiddenDevices();

    // Single pass through all cached devices - use pre-computed category for fast classification
    for (const DeviceInfo &info : DeviceCache::instance().allDevices()) {
        // Skip hidden devices unless show hidden is enabled
        if (info.isHidden() && !showHidden) {
            continue;
        }

        Node *parentNode = nullptr;
        QString displayName = info.name();

        // Use pre-computed category for O(1) classification
        switch (info.category()) {
        case DeviceCategory::AudioInputsAndOutputs:
            parentNode = audioInputsAndOutputsItem;
            break;

        case DeviceCategory::Batteries:
            parentNode = batteriesItem;
            break;

        case DeviceCategory::DisplayAdapters:
            parentNode = displayAdaptersItem;
            break;

        case DeviceCategory::UniversalSerialBusControllers:
            parentNode = universalSerialBusControllersItem;
            break;

        case DeviceCategory::StorageControllers:
            parentNode = storageControllersItem;
            break;

        case DeviceCategory::NetworkAdapters:
            parentNode = networkAdaptersItem;
            break;

        case DeviceCategory::DvdCdromDrives:
            parentNode = dvdCdromDrivesItem;
            break;

        case DeviceCategory::StorageVolumes: {
            // Try to get a nice name: partition label, filesystem label, or device name
            QString volumeName = info.propertyValue(us::propertyNames::ID_PART_ENTRY_NAME);
            if (volumeName.isEmpty()) {
                volumeName = info.propertyValue(us::propertyNames::ID_FS_LABEL);
            }
            if (!volumeName.isEmpty()) {
                displayName = volumeName;
            }
            parentNode = storageVolumesItem;
            break;
        }

        case DeviceCategory::DiskDrives:
            parentNode = diskDrivesItem;
            break;

        case DeviceCategory::HumanInterfaceDevices:
            displayName = s::softwareDeviceNiceName(info.name());
            parentNode = humanInterfaceDevicesItem;
            break;

        case DeviceCategory::Keyboards:
            displayName = s::softwareDeviceNiceName(info.name());
            parentNode = keyboardsItem;
            break;

        case DeviceCategory::MiceAndOtherPointingDevices:
            displayName = s::softwareDeviceNiceName(info.name());
            parentNode = miceAndOtherPointingDevicesItem;
            break;

        case DeviceCategory::SoundVideoAndGameControllers:
            parentNode = soundVideoAndGameControllersItem;
            break;

        case DeviceCategory::SoftwareDevices: {
            QString name = info.name();
            name.replace(kBeginningWithSlashDevRe, s::empty());
            displayName = s::softwareDeviceNiceName(name);
            parentNode = softwareDevicesItem;
            break;
        }

        case DeviceCategory::SystemDevices:
            parentNode = systemDevicesItem;
            break;

        case DeviceCategory::Computer:
        case DeviceCategory::Unknown:
        default:
            // Skip unknown devices
            continue;
        }

        if (parentNode) {
            auto *node = new Node({displayName, info.driver()}, parentNode, NodeType::Device);
            node->setSyspath(info.syspath());
            node->setIsHidden(info.isHidden());
            node->setIcon(parentNode->icon());
            parentNode->appendChild(node);
        }
    }

    // Finalize categories - only add non-empty ones
    finalizeCategory(audioInputsAndOutputsItem);
    finalizeCategory(batteriesItem);
    finalizeCategory(computerItem);
    finalizeCategory(diskDrivesItem);
    finalizeCategory(displayAdaptersItem);
    finalizeCategory(dvdCdromDrivesItem);
    finalizeCategory(humanInterfaceDevicesItem);
    finalizeCategory(keyboardsItem);
    finalizeCategory(miceAndOtherPointingDevicesItem);
    finalizeCategory(networkAdaptersItem);
    finalizeCategory(softwareDevicesItem);
    finalizeCategory(soundVideoAndGameControllersItem);
    finalizeCategory(storageControllersItem);
    finalizeCategory(storageVolumesItem);
    finalizeCategory(systemDevicesItem);
    finalizeCategory(universalSerialBusControllersItem);

    // Sort categories alphabetically
    hostnameItem->sortChildren();
}

QVariant DevicesByTypeModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        if (section == 0) {
            return tr("Name");
        } else if (section == 1) {
            return tr("Driver");
        }
    }
    return {};
}
