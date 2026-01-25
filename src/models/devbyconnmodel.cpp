#include <QtCore/QHash>

#include "const_strings.h"
#include "devicecache.h"
#include "models/devbyconnmodel.h"

namespace s = strings;

DevicesByConnectionModel::DevicesByConnectionModel(QObject *parent)
    : BaseTreeModel(parent), hostnameItem(nullptr) {
    auto *root = new Node({s::empty()});
    setRootItem(root);
    hostnameItem = new Node({DeviceCache::hostname()}, root);
    hostnameItem->setIcon(s::categoryIcons::computer());
    root->appendChild(hostnameItem);
    buildTree();
}

QString DevicesByConnectionModel::getNodeName(const DeviceInfo &info, QString *rawName) const {
    QString name = info.name();
    QString subsystem = info.subsystem();
    bool addSubsystemPrefix = false;

    if (name.isEmpty()) {
        // Fall back to syspath basename
        QString syspath = info.syspath();
        auto lastSlash = syspath.lastIndexOf(QLatin1Char('/'));
        if (lastSlash >= 0) {
            name = syspath.mid(lastSlash + 1);
        } else {
            name = syspath;
        }
        // Only add prefix for subsystems without dedicated display name handling
        addSubsystemPrefix =
            !subsystem.isEmpty() && subsystem != QStringLiteral("acpi") &&
            subsystem != QStringLiteral("scsi_host") && subsystem != QStringLiteral("scsi") &&
            subsystem != QStringLiteral("i2c");
    }

    // Store raw name before transformation
    if (rawName) {
        *rawName = addSubsystemPrefix ? QStringLiteral("[%1] %2").arg(subsystem, name) : name;
    }

    // Apply display name transformations
    if (subsystem == QStringLiteral("acpi")) {
        name = s::acpiDeviceDisplayName(info.devPath(), name);
    } else {
        name = s::softwareDeviceDisplayName(name);
    }

    // Add subsystem prefix if name was empty and subsystem lacks display name handling
    if (addSubsystemPrefix) {
        name = QStringLiteral("[%1] %2").arg(subsystem, name);
    }
    return name;
}

void DevicesByConnectionModel::collectAncestorSyspaths(const QSet<QString> &deviceSyspaths,
                                                       QSet<QString> &allSyspaths) const {
    // For each device syspath, add all ancestor syspaths to maintain the hierarchy
    for (const QString &syspath : deviceSyspaths) {
        allSyspaths.insert(syspath);

        // Walk up the syspath to add ancestors
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
            allSyspaths.insert(current);
        }
    }
}

void DevicesByConnectionModel::buildTree() {
    // Store device list locally to avoid dangling pointers
    // (allDevices() returns a copy, so we need stable references)
    QList<DeviceInfo> allDevices = DeviceCache::instance().allDevices();

    // Use cached devices - filter in memory
    QSet<QString> validSyspaths;
    bool showHidden = DeviceCache::instance().showHiddenDevices();

    // Build filter set
    for (const DeviceInfo &info : allDevices) {
        // Skip hidden devices unless show hidden is enabled
        if (info.isHidden() && !showHidden) {
            continue;
        }
        if (info.isValidForDisplay()) {
            validSyspaths.insert(info.syspath());
        }
    }

    // Expand valid set to include ancestors
    QSet<QString> expandedSyspaths;
    collectAncestorSyspaths(validSyspaths, expandedSyspaths);

    // Collect devices to display (store indices to avoid pointer issues)
    QVector<int> deviceIndicesToDisplay;
    for (int i = 0; i < allDevices.size(); ++i) {
        if (expandedSyspaths.contains(allDevices.at(i).syspath())) {
            deviceIndicesToDisplay.append(i);
        }
    }

    // Sort by syspath length to ensure parents are processed before children
    std::sort(
        deviceIndicesToDisplay.begin(), deviceIndicesToDisplay.end(), [&allDevices](int a, int b) {
            return allDevices.at(a).syspath().length() < allDevices.at(b).syspath().length();
        });

    // Hash map to quickly look up nodes by syspath
    QHash<QString, Node *> nodeMap;

    // Build the tree
    for (int idx : deviceIndicesToDisplay) {
        const DeviceInfo &info = allDevices.at(idx);
        const QString &syspath = info.syspath();
        if (syspath.isEmpty()) {
            continue;
        }

        // Find parent node
        Node *parentNode = hostnameItem;
        const QString &parentSyspath = info.parentSyspath();

        if (!parentSyspath.isEmpty() && nodeMap.contains(parentSyspath)) {
            parentNode = nodeMap.value(parentSyspath);
        }

        // Create node for this device
        QString rawName;
        QString name = getNodeName(info, &rawName);
        auto *node = new Node({name, info.driver()}, parentNode, NodeType::Device);
        node->setSyspath(info.syspath());
        node->setIsHidden(info.isHidden());
        node->setRawName(rawName);

        // Set appropriate icon based on subsystem
        node->setIcon(s::categoryIcons::forSubsystem(info.subsystem()));

        parentNode->appendChild(node);
        nodeMap.insert(syspath, node);
    }
}

QVariant
DevicesByConnectionModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        if (section == 0) {
            return tr("Name");
        }
    }
    return {};
}
