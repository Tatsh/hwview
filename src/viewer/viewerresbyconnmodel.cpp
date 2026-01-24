#include "viewerresbyconnmodel.h"

#include "const_strings.h"
#include "exporteddata.h"
#include "models/node.h"

#include <QJsonArray>
#include <QRegularExpression>
#include <QStack>

namespace s = strings;

ViewerResourcesByConnectionModel::ViewerResourcesByConnectionModel(QObject *parent)
    : BaseTreeModel(parent), hostnameItem(nullptr), dmaItem(nullptr), ioItem(nullptr),
      irqItem(nullptr), memoryItem(nullptr) {
    auto *root = new Node({s::empty(), s::empty()});
    setRootItem(root);
    hostnameItem = new Node({ExportedData::instance().hostname(), s::empty()}, root);
    hostnameItem->setIcon(s::categoryIcons::computer());
    root->appendChild(hostnameItem);
    buildTree();
}

bool ViewerResourcesByConnectionModel::shouldShowIcons() const {
    return true;
}

void ViewerResourcesByConnectionModel::buildTree() {
    addDma();
    addIoPorts();
    addIrq();
    addMemory();
}

void ViewerResourcesByConnectionModel::parseHierarchicalResource(const QJsonArray &lines,
                                                                 Node *categoryNode,
                                                                 const QIcon &icon) {
    static const QRegularExpression resourceRe(
        QStringLiteral("^(\\s*)([0-9a-fA-F]+)-([0-9a-fA-F]+)\\s*:\\s*(.*)$"));

    QStack<QPair<int, Node *>> nodeStack;
    nodeStack.push({-1, categoryNode});

    for (const QJsonValue val : lines) {
        QString line = val.toString();
        if (line.trimmed().isEmpty()) {
            continue;
        }

        auto match = resourceRe.match(line);
        if (!match.hasMatch()) {
            continue;
        }

        int indentLevel = static_cast<int>(match.captured(1).length());
        QString rangeStart = match.captured(2).toUpper();
        QString rangeEnd = match.captured(3).toUpper();
        QString name = match.captured(4);

        if (name.isEmpty()) {
            continue;
        }

        // Pop stack until we find a parent with smaller indentation
        while (nodeStack.size() > 1 && nodeStack.top().first >= indentLevel) {
            nodeStack.pop();
        }

        Node *parentNode = nodeStack.top().second;

        QString displayText = QStringLiteral("[%1 - %2] %3").arg(rangeStart, rangeEnd, name);
        auto *node = new Node({displayText, s::empty()}, parentNode);
        node->setIcon(icon);
        parentNode->appendChild(node);

        nodeStack.push({indentLevel, node});
    }
}

void ViewerResourcesByConnectionModel::addDma() {
    auto &data = ExportedData::instance();
    QJsonArray dmaArray = data.systemResources()[QStringLiteral("dma")].toArray();

    if (dmaArray.isEmpty()) {
        return;
    }

    dmaItem = new Node({tr("Direct memory access (DMA)"), s::empty()}, hostnameItem);
    dmaItem->setIcon(s::categoryIcons::dma());

    static const QRegularExpression dmaRe(QStringLiteral("^(\\d+):\\s*(.*)$"));
    for (const QJsonValue val : dmaArray) {
        QString line = val.toString().trimmed();
        if (line.isEmpty()) {
            continue;
        }

        auto match = dmaRe.match(line);
        if (match.hasMatch()) {
            QString channel = match.captured(1);
            QString name = match.captured(2);
            QString displayText = QStringLiteral("[%1] %2").arg(channel, name);
            auto *node = new Node({displayText, s::empty()}, dmaItem);
            node->setIcon(s::categoryIcons::dma());
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

void ViewerResourcesByConnectionModel::addIoPorts() {
    auto &data = ExportedData::instance();
    QJsonArray ioportsArray = data.systemResources()[QStringLiteral("ioports")].toArray();

    if (ioportsArray.isEmpty()) {
        return;
    }

    ioItem = new Node({tr("Input/output (IO)"), s::empty()}, hostnameItem);
    ioItem->setIcon(s::categoryIcons::ioPorts());

    parseHierarchicalResource(ioportsArray, ioItem, s::categoryIcons::ioPorts());

    if (ioItem->childCount() > 0) {
        hostnameItem->appendChild(ioItem);
    } else {
        delete ioItem;
        ioItem = nullptr;
    }
}

void ViewerResourcesByConnectionModel::addIrq() {
    auto &data = ExportedData::instance();
    QJsonArray interruptsArray = data.systemResources()[QStringLiteral("interrupts")].toArray();

    if (interruptsArray.isEmpty()) {
        return;
    }

    irqItem = new Node({tr("Interrupt request (IRQ)"), s::empty()}, hostnameItem);
    irqItem->setIcon(s::categoryIcons::irq());

    static const QRegularExpression whitespaceRe(QStringLiteral("\\s+"));

    bool firstLine = true;
    for (const QJsonValue val : interruptsArray) {
        QString line = val.toString();

        if (firstLine) {
            firstLine = false;
            continue;
        }

        if (line.trimmed().isEmpty()) {
            continue;
        }

        auto parts = line.split(whitespaceRe, Qt::SkipEmptyParts);
        if (parts.size() < 2) {
            continue;
        }

        QString irqNum = parts[0];
        if (irqNum.endsWith(QLatin1Char(':'))) {
            irqNum.chop(1);
        }

        QString deviceName;
        QString irqType;

        for (int i = 1; i < parts.size(); ++i) {
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
        node->setIcon(s::categoryIcons::irq());
        irqItem->appendChild(node);
    }

    if (irqItem->childCount() > 0) {
        hostnameItem->appendChild(irqItem);
    } else {
        delete irqItem;
        irqItem = nullptr;
    }
}

void ViewerResourcesByConnectionModel::addMemory() {
    auto &data = ExportedData::instance();
    QJsonArray iomemArray = data.systemResources()[QStringLiteral("iomem")].toArray();

    if (iomemArray.isEmpty()) {
        return;
    }

    memoryItem = new Node({tr("Memory"), s::empty()}, hostnameItem);
    memoryItem->setIcon(s::categoryIcons::memory());

    parseHierarchicalResource(iomemArray, memoryItem, s::categoryIcons::memory());

    if (memoryItem->childCount() > 0) {
        hostnameItem->appendChild(memoryItem);
    } else {
        delete memoryItem;
        memoryItem = nullptr;
    }
}

int ViewerResourcesByConnectionModel::columnCount([[maybe_unused]] const QModelIndex &parent) const {
    return 1;
}

QVariant ViewerResourcesByConnectionModel::headerData(int section,
                                                      Qt::Orientation orientation,
                                                      int role) const {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        if (section == 0) {
            return tr("Resource");
        }
    }
    return {};
}
