#include "exporteddata.h"

#include <QFile>
#include <QJsonDocument>

ExportedData &ExportedData::instance() {
    static ExportedData data;
    return data;
}

ExportedData::ExportedData() : QObject(nullptr) {
}

void ExportedData::clear() {
    loaded_ = false;
    filePath_.clear();
    hostname_.clear();
    exportDate_.clear();
    sourceAppName_.clear();
    sourceAppVersion_.clear();
    devices_.clear();
    syspathIndex_.clear();
    systemInfo_ = QJsonObject();
    systemResources_ = QJsonObject();
}

bool ExportedData::loadFromFile(const QString &filePath) {
    clear();

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);
    file.close();

    if (error.error != QJsonParseError::NoError) {
        return false;
    }

    if (!doc.isObject()) {
        return false;
    }

    QJsonObject root = doc.object();

    // Parse metadata
    exportDate_ = root[QStringLiteral("exportDate")].toString();
    sourceAppName_ = root[QStringLiteral("applicationName")].toString();
    sourceAppVersion_ = root[QStringLiteral("applicationVersion")].toString();

    // Parse system info
    systemInfo_ = root[QStringLiteral("system")].toObject();
    hostname_ = systemInfo_[QStringLiteral("hostname")].toString();

    // Parse system resources
    systemResources_ = root[QStringLiteral("systemResources")].toObject();

    // Parse devices
    QJsonArray devicesArray = root[QStringLiteral("devices")].toArray();
    devices_.reserve(devicesArray.size());

    for (const QJsonValue val : devicesArray) {
        if (val.isObject()) {
            devices_.append(parseDevice(val.toObject()));
            syspathIndex_.insert(devices_.last().syspath, static_cast<int>(devices_.size() - 1));
        }
    }

    filePath_ = filePath;
    loaded_ = true;

    Q_EMIT dataLoaded();
    return true;
}

ExportedData::Device ExportedData::parseDevice(const QJsonObject &obj) {
    Device dev;

    dev.syspath = obj[QStringLiteral("syspath")].toString();
    dev.name = obj[QStringLiteral("name")].toString();
    dev.driver = obj[QStringLiteral("driver")].toString();
    dev.subsystem = obj[QStringLiteral("subsystem")].toString();
    dev.devnode = obj[QStringLiteral("devnode")].toString();
    dev.parentSyspath = obj[QStringLiteral("parentSyspath")].toString();
    dev.devPath = obj[QStringLiteral("devPath")].toString();
    dev.category = obj[QStringLiteral("category")].toInt();
    dev.categoryName = obj[QStringLiteral("categoryName")].toString();
    dev.isHidden = obj[QStringLiteral("isHidden")].toBool();
    dev.isValidForDisplay = obj[QStringLiteral("isValidForDisplay")].toBool();

    // PCI info
    QJsonObject pci = obj[QStringLiteral("pci")].toObject();
    dev.pciClass = pci[QStringLiteral("class")].toString();
    dev.pciSubclass = pci[QStringLiteral("subclass")].toString();
    dev.pciInterface = pci[QStringLiteral("interface")].toString();

    // ID properties
    QJsonObject ids = obj[QStringLiteral("ids")].toObject();
    dev.idCdrom = ids[QStringLiteral("cdrom")].toString();
    dev.idDevType = ids[QStringLiteral("devType")].toString();
    dev.idInputKeyboard = ids[QStringLiteral("inputKeyboard")].toString();
    dev.idInputMouse = ids[QStringLiteral("inputMouse")].toString();
    dev.idType = ids[QStringLiteral("type")].toString();
    dev.idModelFromDatabase = ids[QStringLiteral("modelFromDatabase")].toString();

    // Additional data
    dev.properties = obj[QStringLiteral("properties")].toObject();
    dev.driverInfo = obj[QStringLiteral("driverInfo")].toObject();
    dev.resources = obj[QStringLiteral("resources")].toArray();

    return dev;
}

bool ExportedData::isLoaded() const {
    return loaded_;
}

const QString &ExportedData::hostname() const {
    return hostname_;
}

const QList<ExportedData::Device> &ExportedData::allDevices() const {
    return devices_;
}

const ExportedData::Device *ExportedData::deviceBySyspath(const QString &syspath) const {
    auto it = syspathIndex_.find(syspath);
    if (it != syspathIndex_.end()) {
        return &devices_.at(it.value());
    }
    return nullptr;
}

const QJsonObject &ExportedData::systemInfo() const {
    return systemInfo_;
}

const QJsonObject &ExportedData::systemResources() const {
    return systemResources_;
}

const QString &ExportedData::exportDate() const {
    return exportDate_;
}

const QString &ExportedData::sourceApplicationName() const {
    return sourceAppName_;
}

const QString &ExportedData::sourceApplicationVersion() const {
    return sourceAppVersion_;
}

const QString &ExportedData::currentFilePath() const {
    return filePath_;
}

bool ExportedData::showHiddenDevices() const {
    return showHiddenDevices_;
}

void ExportedData::setShowHiddenDevices(bool show) {
    showHiddenDevices_ = show;
}
