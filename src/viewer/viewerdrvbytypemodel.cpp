#include "viewerdrvbytypemodel.h"

#include "const_strings.h"
#include "exporteddata.h"
#include "models/node.h"

#include <QMap>
#include <QSet>

namespace s = strings;

ViewerDriversByTypeModel::ViewerDriversByTypeModel(QObject *parent)
    : BaseTreeModel(parent), hostnameItem(nullptr) {
    auto *root = new Node({s::empty()});
    setRootItem(root);
    hostnameItem = new Node({ExportedData::instance().hostname()}, root);
    hostnameItem->setIcon(s::categoryIcons::computer());
    root->appendChild(hostnameItem);
    buildTree();
}

void ViewerDriversByTypeModel::finalizeCategory(Node *&category) {
    if (category && category->childCount() > 0) {
        hostnameItem->appendChild(category);
    } else if (category) {
        delete category;
        category = nullptr;
    }
}

void ViewerDriversByTypeModel::buildTree() {
    auto &data = ExportedData::instance();
    bool showHidden = data.showHiddenDevices();

    // Collect unique drivers by category
    QMap<QString, QSet<QString>> driversByCategory;

    // Category enum values
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

    for (const ExportedData::Device &dev : data.allDevices()) {
        // Skip hidden devices if not showing them
        if (dev.isHidden && !showHidden) {
            continue;
        }
        if (dev.driver.isEmpty()) {
            continue;
        }

        QString category;
        switch (dev.category) {
        case AudioInputsAndOutputs:
        case SoundVideoAndGameControllers:
            category = tr("Sound, video and game controllers");
            break;
        case DisplayAdapters:
            category = tr("Display adapters");
            break;
        case NetworkAdapters:
            category = tr("Network adapters");
            break;
        case StorageControllers:
            category = tr("Storage controllers");
            break;
        case UniversalSerialBusControllers:
            category = tr("Universal Serial Bus controllers");
            break;
        case DiskDrives:
        case DvdCdromDrives:
        case StorageVolumes:
            category = tr("Disk drives");
            break;
        case HumanInterfaceDevices:
            category = tr("Human Interface Devices");
            break;
        case Keyboards:
            category = tr("Keyboards");
            break;
        case MiceAndOtherPointingDevices:
            category = tr("Mice and other pointing devices");
            break;
        case Batteries:
            category = tr("Batteries");
            break;
        case SystemDevices:
            category = tr("System devices");
            break;
        case SoftwareDevices:
            category = tr("Software devices");
            break;
        default:
            category = tr("Other devices");
            break;
        }

        if (!category.isEmpty()) {
            driversByCategory[category].insert(dev.driver);
        }
    }

    // Create nodes
    for (auto it = driversByCategory.constBegin(); it != driversByCategory.constEnd(); ++it) {
        const QString &categoryName = it.key();
        const QSet<QString> &drivers = it.value();

        auto *categoryNode = new Node({categoryName}, hostnameItem);
        categoryNode->setIcon(s::categoryIcons::forCategory(categoryName));

        QStringList sortedDrivers = drivers.values();
        std::sort(sortedDrivers.begin(), sortedDrivers.end());

        for (const QString &driver : sortedDrivers) {
            auto *driverNode = new Node({driver}, categoryNode, NodeType::Label);
            driverNode->setIcon(s::categoryIcons::forDriver(driver));
            categoryNode->appendChild(driverNode);
        }

        finalizeCategory(categoryNode);
    }
}

QVariant ViewerDriversByTypeModel::headerData(int section,
                                              Qt::Orientation orientation,
                                              int role) const {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        if (section == 0) {
            return tr("Name");
        }
    }
    return {};
}
