#pragma once

#include "deviceinfo_p.h"

#include <QtCore/QJsonObject>

/**
 * @brief Implementation for devices loaded from JSON export files.
 */
class ImportedDeviceInfoPrivate : public DeviceInfoPrivate {
public:
    /**
     * @brief Construct from exported JSON data.
     * @param json The JSON object containing device data.
     */
    explicit ImportedDeviceInfoPrivate(const QJsonObject &json);

    ~ImportedDeviceInfoPrivate() override = default;

    QString propertyValue(const char *key) const override;
    DeviceInfoPrivate *clone(DeviceInfo *q) const override;
    void dump() const override;

    // Additional imported data
    QJsonObject properties_;
    QJsonObject driverInfo_;
    QJsonArray resources_;

protected:
    // Copy constructor for clone()
    ImportedDeviceInfoPrivate(const ImportedDeviceInfoPrivate &other, DeviceInfo *q);
};
