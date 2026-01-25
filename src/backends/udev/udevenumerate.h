#pragma once

#include <QString>
#include <libudev.h>

class UdevManager;

/**
 * @brief RAII wrapper for udev device enumeration.
 */
class UdevEnumerate {
public:
    /**
     * @brief Constructs a UdevEnumerate from a UdevManager.
     * @param manager The UdevManager providing the udev context.
     */
    UdevEnumerate(UdevManager &manager);
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
