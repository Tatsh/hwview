#include <QRegularExpression>
#include <QStack>

#include "const_strings.h"
#include "devicecache.h"
#include "models/resbyconnmodel.h"
#include "procutils.h"
#include "viewsettings.h"

namespace s = strings;

#ifdef Q_OS_LINUX
using procutils::readProcFile;
#endif

ResourcesByConnectionModel::ResourcesByConnectionModel(QObject *parent)
    : BaseTreeModel(parent), hostnameItem(nullptr), dmaItem(nullptr), ioItem(nullptr),
      irqItem(nullptr), memoryItem(nullptr) {
    auto *root = new Node({s::empty(), s::empty()});
    setRootItem(root);
    hostnameItem = new Node({DeviceCache::hostname(), s::empty()}, root);
    hostnameItem->setIcon(s::categoryIcons::computer());
    root->appendChild(hostnameItem);
    buildTree();
}

bool ResourcesByConnectionModel::shouldShowIcons() const {
    return ViewSettings::instance().showDeviceIcons();
}

void ResourcesByConnectionModel::buildTree() {
#ifdef Q_OS_LINUX
    addDma();
    addIoPorts();
    addIrq();
    addMemory();
#endif
}

#ifdef Q_OS_LINUX
void ResourcesByConnectionModel::addDma() {
    dmaItem = new Node({tr("Direct memory access (DMA)"), s::empty()}, hostnameItem);
    dmaItem->setIcon(s::categoryIcons::other());

    static const QRegularExpression dmaRe(QStringLiteral("^(\\d+):\\s*(.*)$"));
    for (const auto &line : readProcFile(QStringLiteral("/proc/dma"))) {
        if (line.trimmed().isEmpty()) {
            continue;
        }
        auto match = dmaRe.match(line.trimmed());
        if (match.hasMatch()) {
            auto channel = match.captured(1);
            auto name = match.captured(2);
            auto displayText = QStringLiteral("[%1] %2").arg(channel, name);
            auto *node = new Node({displayText, s::empty()}, dmaItem);
            node->setIcon(s::categoryIcons::other());
            dmaItem->appendChild(node);
        }
    }

    if (dmaItem->childCount() > 0) {
        hostnameItem->appendChild(dmaItem);
    } else {
        delete dmaItem;
        dmaItem = nullptr;
    }
}

void ResourcesByConnectionModel::parseHierarchicalResource(const QString &filePath,
                                                           Node *categoryNode) {
    static const QRegularExpression resourceRe(
        QStringLiteral("^(\\s*)([0-9a-fA-F]+)-([0-9a-fA-F]+)\\s*:\\s*(.*)$"));

    // Stack to track parent nodes at each indentation level
    // Each entry is (indentLevel, Node*)
    QStack<QPair<int, Node *>> nodeStack;
    nodeStack.push({-1, categoryNode});

    for (const auto &line : readProcFile(filePath)) {
        if (line.trimmed().isEmpty()) {
            continue;
        }

        auto match = resourceRe.match(line);
        if (!match.hasMatch()) {
            continue;
        }

        auto indentLevel = static_cast<int>(match.captured(1).length());
        auto rangeStart = match.captured(2).toUpper();
        auto rangeEnd = match.captured(3).toUpper();
        auto name = match.captured(4);

        if (name.isEmpty()) {
            continue;
        }

        // Pop stack until we find a parent with smaller indentation
        while (nodeStack.size() > 1 && nodeStack.top().first >= indentLevel) {
            nodeStack.pop();
        }

        auto *parentNode = nodeStack.top().second;

        auto displayText = QStringLiteral("[%1 - %2] %3").arg(rangeStart, rangeEnd, name);
        auto *node = new Node({displayText, s::empty()}, parentNode);
        node->setIcon(s::categoryIcons::other());
        parentNode->appendChild(node);

        // Push this node as potential parent for more indented entries
        nodeStack.push({indentLevel, node});
    }
}

void ResourcesByConnectionModel::addIoPorts() {
    ioItem = new Node({tr("Input/output (IO)"), s::empty()}, hostnameItem);
    ioItem->setIcon(s::categoryIcons::other());

    parseHierarchicalResource(QStringLiteral("/proc/ioports"), ioItem);

    if (ioItem->childCount() > 0) {
        hostnameItem->appendChild(ioItem);
    } else {
        delete ioItem;
        ioItem = nullptr;
    }
}

void ResourcesByConnectionModel::addIrq() {
    irqItem = new Node({tr("Interrupt request (IRQ)"), s::empty()}, hostnameItem);
    irqItem->setIcon(s::categoryIcons::other());

    auto lines = readProcFile(QStringLiteral("/proc/interrupts"));
    if (!lines.isEmpty()) {
        lines.removeFirst(); // Skip header
    }

    static const QRegularExpression whitespaceRe(QStringLiteral("\\s+"));
    for (const auto &line : lines) {
        if (line.trimmed().isEmpty()) {
            continue;
        }

        auto parts = line.split(whitespaceRe, Qt::SkipEmptyParts);
        if (parts.size() < 2) {
            continue;
        }

        auto irqNum = parts[0];
        if (irqNum.endsWith(QLatin1Char(':'))) {
            irqNum.chop(1);
        }

        QString deviceName;
        QString irqType;

        for (auto i = 1; i < parts.size(); ++i) {
            if (parts[i].contains(QStringLiteral("APIC")) ||
                parts[i].contains(QStringLiteral("PCI")) ||
                parts[i].contains(QStringLiteral("MSI")) ||
                parts[i].contains(QStringLiteral("DMAR")) ||
                parts[i].contains(QStringLiteral("edge")) ||
                parts[i].contains(QStringLiteral("level")) ||
                parts[i].contains(QStringLiteral("fasteoi"))) {
                if (irqType.isEmpty()) {
                    irqType = parts[i];
                } else {
                    irqType += QLatin1Char(' ') + parts[i];
                }
            } else if (i > 1 && !parts[i].isEmpty() && !parts[i].at(0).isDigit()) {
                if (deviceName.isEmpty()) {
                    deviceName = parts[i];
                } else {
                    deviceName += QLatin1Char(' ') + parts[i];
                }
            }
        }

        if (deviceName.isEmpty()) {
            continue;
        }

        QString displayText;
        if (!irqType.isEmpty()) {
            displayText = QStringLiteral("(%1) %2 %3").arg(irqType, irqNum, deviceName);
        } else {
            displayText = QStringLiteral("%1 %2").arg(irqNum, deviceName);
        }
        auto *node = new Node({displayText, s::empty()}, irqItem);
        node->setIcon(s::categoryIcons::other());
        irqItem->appendChild(node);
    }

    if (irqItem->childCount() > 0) {
        hostnameItem->appendChild(irqItem);
    } else {
        delete irqItem;
        irqItem = nullptr;
    }
}

void ResourcesByConnectionModel::addMemory() {
    memoryItem = new Node({tr("Memory"), s::empty()}, hostnameItem);
    memoryItem->setIcon(s::categoryIcons::other());

    parseHierarchicalResource(QStringLiteral("/proc/iomem"), memoryItem);

    if (memoryItem->childCount() > 0) {
        hostnameItem->appendChild(memoryItem);
    } else {
        delete memoryItem;
        memoryItem = nullptr;
    }
}
#endif

int ResourcesByConnectionModel::columnCount([[maybe_unused]] const QModelIndex &parent) const {
    return ViewSettings::instance().showDriverColumn() ? 2 : 1;
}

QVariant
ResourcesByConnectionModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
        case 0:
            return tr("Resource");
        case 1:
            return tr("Details");
        default:
            return {};
        }
    }
    return {};
}
