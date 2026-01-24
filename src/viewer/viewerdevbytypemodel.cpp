#include "viewerdevbytypemodel.h"

#include "const_strings.h"
#include "exporteddata.h"
#include "models/node.h"

namespace s = strings;

ViewerDevicesByTypeModel::ViewerDevicesByTypeModel(QObject *parent)
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
    hostnameItem = new Node({ExportedData::instance().hostname(), s::empty()}, root);
    hostnameItem->setIcon(s::categoryIcons::computer());
    root->appendChild(hostnameItem);
    buildTree();
}

QVariant ViewerDevicesByTypeModel::decorationData(Node *item, int column) const {
    if (column != 0) {
        return {};
    }
    if (item->isHidden()) {
        return item->disabledPixmap(16);
    }
    return item->icon();
}

void ViewerDevicesByTypeModel::finalizeCategory(Node *&category) {
    if (category && category->childCount() > 0) {
        category->sortChildren();
        hostnameItem->appendChild(category);
    } else if (category) {
        delete category;
        category = nullptr;
    }
}

void ViewerDevicesByTypeModel::buildTree() {
    // Create all category nodes
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

    // Category enum values from deviceinfo.h
    enum Category {
        Unknown = 0,
        AudioInputsAndOutputs = 1,
        Batteries = 2,
        Computer = 3,
        DiskDrives = 4,
        DisplayAdapters = 5,
        DvdCdromDrives = 6,
        HumanInterfaceDevices = 7,
        Keyboards = 8,
        MiceAndOtherPointingDevices = 9,
        NetworkAdapters = 10,
        SoftwareDevices = 11,
        SoundVideoAndGameControllers = 12,
        StorageControllers = 13,
        StorageVolumes = 14,
        SystemDevices = 15,
        UniversalSerialBusControllers = 16,
    };

    // Iterate through all exported devices
    bool showHidden = ExportedData::instance().showHiddenDevices();
    for (const ExportedData::Device &dev : ExportedData::instance().allDevices()) {
        // Skip hidden devices if not showing them
        if (dev.isHidden && !showHidden) {
            continue;
        }

        Node *parentNode = nullptr;
        QString displayName = dev.name;

        switch (dev.category) {
        case AudioInputsAndOutputs:
            parentNode = audioInputsAndOutputsItem;
            break;
        case Batteries:
            displayName = s::acpiDeviceNiceName(dev.devPath, dev.name);
            parentNode = batteriesItem;
            break;
        case Computer:
            // Skip - Computer entry is handled separately
            continue;
        case DiskDrives:
            parentNode = diskDrivesItem;
            break;
        case DisplayAdapters:
            parentNode = displayAdaptersItem;
            break;
        case DvdCdromDrives:
            parentNode = dvdCdromDrivesItem;
            break;
        case HumanInterfaceDevices:
            displayName = s::softwareDeviceNiceName(dev.name);
            parentNode = humanInterfaceDevicesItem;
            break;
        case Keyboards:
            displayName = s::softwareDeviceNiceName(dev.name);
            parentNode = keyboardsItem;
            break;
        case MiceAndOtherPointingDevices:
            displayName = s::softwareDeviceNiceName(dev.name);
            parentNode = miceAndOtherPointingDevicesItem;
            break;
        case NetworkAdapters:
            parentNode = networkAdaptersItem;
            break;
        case SoftwareDevices:
            displayName = s::softwareDeviceNiceName(dev.name);
            parentNode = softwareDevicesItem;
            break;
        case SoundVideoAndGameControllers:
            parentNode = soundVideoAndGameControllersItem;
            break;
        case StorageControllers:
            parentNode = storageControllersItem;
            break;
        case StorageVolumes:
            parentNode = storageVolumesItem;
            break;
        case SystemDevices:
            parentNode = systemDevicesItem;
            break;
        case UniversalSerialBusControllers:
            parentNode = universalSerialBusControllersItem;
            break;
        case Unknown:
        default:
            continue;
        }

        if (parentNode) {
            auto *node = new Node({displayName, dev.driver}, parentNode, NodeType::Device);
            node->setSyspath(dev.syspath);
            node->setIsHidden(dev.isHidden);
            node->setRawName(dev.name);
            node->setIcon(parentNode->icon());
            parentNode->appendChild(node);
        }
    }

    // Add Computer entry from system info
    auto &data = ExportedData::instance();
    QString arch = data.systemInfo()[QStringLiteral("cpuArchitecture")].toString();
    QString computerName;
    if (arch == QStringLiteral("x86_64")) {
        computerName = tr("ACPI x64-based PC");
    } else if (arch == QStringLiteral("i386") || arch == QStringLiteral("i686")) {
        computerName = tr("ACPI x86-based PC");
    } else if (arch == QStringLiteral("arm64") || arch == QStringLiteral("aarch64")) {
        computerName = tr("ACPI ARM64-based PC");
    } else {
        computerName = tr("ACPI-based PC");
    }
    auto *acpiNode = new Node({computerName, s::empty()}, computerItem, NodeType::Device);
    acpiNode->setIcon(computerItem->icon());
    computerItem->appendChild(acpiNode);

    // Finalize categories
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

    hostnameItem->sortChildren();
}

QVariant ViewerDevicesByTypeModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        if (section == 0) {
            return tr("Name");
        } else if (section == 1) {
            return tr("Driver");
        }
    }
    return {};
}
