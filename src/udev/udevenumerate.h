#pragma once

#include <QtGlobal>

#ifdef Q_OS_LINUX
#include <QString>
#include <libudev.h>

class UdevManager;

/**
 * @brief RAII wrapper for udev device enumeration.
 *
 * This class provides a convenient C++ interface for udev device enumeration,
 * automatically managing the lifecycle of the udev_enumerate object.
 */
class UdevEnumerate {
public:
    /**
     * @brief Constructs a UdevEnumerate from a UdevManager.
     * @param manager The UdevManager providing the udev context.
     */
    UdevEnumerate(UdevManager &manager);

    /**
     * @brief Destroys the UdevEnumerate and releases resources.
     */
    ~UdevEnumerate();

    /**
     * @brief Adds a property match filter (property must exist).
     * @param property The property name to match.
     */
    void addMatchProperty(const char *property);

    /**
     * @brief Adds a property match filter with a specific value.
     * @param property The property name to match.
     * @param value The required property value.
     */
    void addMatchProperty(const char *property, const QString &value);

    /**
     * @brief Returns the underlying udev_enumerate object.
     * @returns Pointer to the udev_enumerate structure.
     */
    struct udev_enumerate *enumerator() const;

private:
    struct udev_enumerate *enumerator_;
};
#endif
