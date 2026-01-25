#pragma once

#include <QJsonArray>
#include <QJsonObject>
#include <QString>

enum class DeviceCategory;
class DeviceInfo;

/**
 * @brief Private implementation base class for DeviceInfo.
 *
 * This class provides the platform-agnostic interface for device information.
 * Platform-specific backends derive from this class and implement the virtual methods.
 *
 * Uses Qt's d-pointer convention.
 */
class DeviceInfoPrivate {
    Q_DISABLE_COPY(DeviceInfoPrivate)
public:
    /**
     * @brief Construct with optional q-pointer.
     * @param q The public instance (can be nullptr, will be set by DeviceInfo constructor).
     */
    explicit DeviceInfoPrivate(DeviceInfo *q = nullptr);
    virtual ~DeviceInfoPrivate();

    /**
     * @brief Get a property value by key.
     * @param key The property key name.
     * @returns The property value, or empty string if not found.
     */
    virtual QString propertyValue(const char *key) const = 0;

    /**
     * @brief Clone this private implementation for a new public instance.
     * @param q The new public instance that will own the clone.
     * @returns A new copy of this implementation.
     */
    virtual DeviceInfoPrivate *clone(DeviceInfo *q) const = 0;

    /**
     * @brief Dump device information for debugging.
     */
    virtual void dump() const = 0;

    // Common data members populated by platform-specific constructors
    QString devPath_;
    QString driver_;
    QString hidID_;
    QString hidName_;
    QString hidPhysicalMac_;
    QString hidUniq_;
    QString modAlias_;
    QString name_;
    QString subsystem_;
    QString syspath_;
    QString parentSyspath_;
    QString devnode_;
    QString idVendorFromDatabase_;
    QString pciClass_;
    QString pciSubclass_;
    QString pciInterface_;
    QString idCdrom_;
    QString devType_;
    QString idInputKeyboard_;
    QString idInputMouse_;
    QString idType_;
    QString idModelFromDatabase_;
    bool isHidden_ = false;
    DeviceCategory category_;

    // Platform-specific class name storage (used for category calculation)
    QString platformClassName_;

    // q-pointer back to public class
    DeviceInfo *q_ptr;

protected:
    // Protected copy for use by clone()
    DeviceInfoPrivate(const DeviceInfoPrivate &other, DeviceInfo *q);
};

/**
 * @brief Factory function to create a DeviceInfoPrivate from JSON export data.
 * @param json The JSON data.
 * @returns New DeviceInfoPrivate instance (q_ptr will be set by DeviceInfo).
 */
DeviceInfoPrivate *createDeviceInfoFromJson(const QJsonObject &json);
