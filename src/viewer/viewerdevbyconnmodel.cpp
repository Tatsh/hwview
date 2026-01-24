#include "viewerdevbyconnmodel.h"

#include "const_strings.h"
#include "exporteddata.h"
#include "models/node.h"

#include <QSet>

namespace s = strings;

ViewerDevicesByConnectionModel::ViewerDevicesByConnectionModel(QObject *parent)
    : BaseTreeModel(parent), hostnameItem(nullptr) {
    auto *root = new Node({s::empty()});
    setRootItem(root);
    hostnameItem = new Node({ExportedData::instance().hostname()}, root);
    hostnameItem->setIcon(s::categoryIcons::computer());
    root->appendChild(hostnameItem);
    buildTree();
}

QVariant ViewerDevicesByConnectionModel::decorationData(Node *item, int column) const {
    if (column != 0) {
        return {};
    }
    if (item->isHidden()) {
        return item->disabledPixmap(16);
    }
    return item->icon();
}

void ViewerDevicesByConnectionModel::buildTree() {
    auto &data = ExportedData::instance();
    const auto &allDevices = data.allDevices();
    bool showHidden = data.showHiddenDevices();

    // Build filter set of valid syspaths
    QSet<QString> validSyspaths;
    for (const ExportedData::Device &dev : allDevices) {
        // Skip hidden devices if not showing them
        if (dev.isHidden && !showHidden) {
            continue;
        }
        if (dev.isValidForDisplay) {
            validSyspaths.insert(dev.syspath);
        }
    }

    // Expand to include ancestors
    QSet<QString> expandedSyspaths;
    for (const QString &syspath : validSyspaths) {
        expandedSyspaths.insert(syspath);

        QString current = syspath;
        while (true) {
            auto lastSlash = current.lastIndexOf(QLatin1Char('/'));
            if (lastSlash <= 0) {
                break;
            }
            current = current.left(lastSlash);
            if (current == QStringLiteral("/sys/devices") || current == QStringLiteral("/sys")) {
                break;
            }
            expandedSyspaths.insert(current);
        }
    }

    // Collect devices to display
    QVector<int> deviceIndicesToDisplay;
    for (int i = 0; i < allDevices.size(); ++i) {
        if (expandedSyspaths.contains(allDevices.at(i).syspath)) {
            deviceIndicesToDisplay.append(i);
        }
    }

    // Sort by syspath length to process parents before children
    std::sort(
        deviceIndicesToDisplay.begin(), deviceIndicesToDisplay.end(), [&allDevices](int a, int b) {
            return allDevices.at(a).syspath.length() < allDevices.at(b).syspath.length();
        });

    // Build the tree
    for (int idx : deviceIndicesToDisplay) {
        const ExportedData::Device &dev = allDevices.at(idx);
        if (dev.syspath.isEmpty()) {
            continue;
        }

        // Find parent node
        Node *parentNode = hostnameItem;
        if (!dev.parentSyspath.isEmpty() && nodesBySyspath_.contains(dev.parentSyspath)) {
            parentNode = nodesBySyspath_.value(dev.parentSyspath);
        }

        // Get display name
        QString name = dev.name;
        QString rawName = name;
        bool addSubsystemPrefix = false;

        if (name.isEmpty()) {
            auto lastSlash = dev.syspath.lastIndexOf(QLatin1Char('/'));
            if (lastSlash >= 0) {
                name = dev.syspath.mid(lastSlash + 1);
            } else {
                name = dev.syspath;
            }
            addSubsystemPrefix =
                !dev.subsystem.isEmpty() && dev.subsystem != QStringLiteral("acpi") &&
                dev.subsystem != QStringLiteral("scsi_host") &&
                dev.subsystem != QStringLiteral("scsi") && dev.subsystem != QStringLiteral("i2c");
        }

        if (addSubsystemPrefix) {
            rawName = QStringLiteral("[%1] %2").arg(dev.subsystem, name);
        } else {
            rawName = name;
        }

        // Apply nice name transformations
        if (dev.subsystem == QStringLiteral("acpi")) {
            name = s::acpiDeviceNiceName(dev.devPath, name);
        } else {
            name = s::softwareDeviceNiceName(name);
        }

        if (addSubsystemPrefix) {
            name = QStringLiteral("[%1] %2").arg(dev.subsystem, name);
        }

        // Create node
        auto *node = new Node({name}, parentNode, NodeType::Device);
        node->setSyspath(dev.syspath);
        node->setIsHidden(dev.isHidden);
        node->setRawName(rawName);
        node->setIcon(s::categoryIcons::forSubsystem(dev.subsystem));

        parentNode->appendChild(node);
        nodesBySyspath_.insert(dev.syspath, node);
    }
}

int ViewerDevicesByConnectionModel::columnCount([[maybe_unused]] const QModelIndex &parent) const {
    return 1;
}

QVariant ViewerDevicesByConnectionModel::headerData(int section,
                                                    Qt::Orientation orientation,
                                                    int role) const {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        if (section == 0) {
            return tr("Name");
        }
    }
    return {};
}
