#include <QtCore/QRegularExpression>

#include "const_strings.h"
#include "devicecache.h"
#include "models/devbytypemodel.h"
#include "systeminfo.h"

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

    // Add computer info using platform-specific backend
    auto computerName = getComputerDisplayName();
    auto computerSyspath = getComputerSyspath();

    auto *acpiNode = new Node({computerName, s::empty()}, computerItem, NodeType::Device);
    if (!computerSyspath.isEmpty()) {
        acpiNode->setSyspath(computerSyspath);
    }
    acpiNode->setIcon(computerItem->icon());
    computerItem->appendChild(acpiNode);

    static const QRegularExpression kBeginningWithSlashDevRe(QStringLiteral("^/dev/"));

    auto showHidden = DeviceCache::instance().showHiddenDevices();

    // Single pass through all cached devices - use pre-computed category for fast classification
    for (const DeviceInfo &info : DeviceCache::instance().allDevices()) {
        // Skip hidden devices unless show hidden is enabled
        if (info.isHidden() && !showHidden) {
            continue;
        }

        Node *parentNode = nullptr;
        auto rawName = info.name();
        auto displayName = rawName;

        // Use pre-computed category for O(1) classification
        switch (info.category()) {
        case DeviceCategory::AudioInputsAndOutputs:
            parentNode = audioInputsAndOutputsItem;
            break;

        case DeviceCategory::Batteries:
            displayName = s::acpiDeviceDisplayName(info.devPath(), rawName);
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
            // Try to get a display name: partition label, filesystem label, or device name
            auto volumeName = info.propertyValue(us::propertyNames::ID_PART_ENTRY_NAME);
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
            displayName = s::softwareDeviceDisplayName(rawName);
            parentNode = humanInterfaceDevicesItem;
            break;

        case DeviceCategory::Keyboards:
            displayName = s::softwareDeviceDisplayName(rawName);
            parentNode = keyboardsItem;
            break;

        case DeviceCategory::MiceAndOtherPointingDevices:
            displayName = s::softwareDeviceDisplayName(rawName);
            parentNode = miceAndOtherPointingDevicesItem;
            break;

        case DeviceCategory::SoundVideoAndGameControllers:
            parentNode = soundVideoAndGameControllersItem;
            break;

        case DeviceCategory::SoftwareDevices: {
            rawName = info.name();
            rawName.replace(kBeginningWithSlashDevRe, s::empty());
            displayName = s::softwareDeviceDisplayName(rawName);
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
            node->setRawName(rawName);
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
