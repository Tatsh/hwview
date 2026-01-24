#pragma once

#include "devicemonitor.h"

#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>

/**
 * @brief Monitors IOKit events for device add/remove notifications on macOS.
 *
 * This class uses IOKit's notification system to receive device events from the kernel. It
 * integrates with Qt's event loop using a @c CFRunLoopSource for non-blocking operation.
 *
 * Example usage:
 * @code
 * IOKitMonitor monitor;
 * connect(&monitor, &IOKitMonitor::deviceChanged, this, &MyClass::onDeviceChanged);
 * if (auto result = monitor.start(); !result) {
 *     // Handle error
 * }
 * @endcode
 */
class IOKitMonitor : public DeviceMonitor {
    Q_OBJECT

public:
    /**
     * @brief Constructs an @c IOKitMonitor.
     * @param parent Optional parent @c QObject for memory management.
     */
    explicit IOKitMonitor(QObject *parent = nullptr);
    ~IOKitMonitor() override;

    /**
     * @brief Starts monitoring for device events.
     *
     * Creates an IOKit notification port, registers for device add/remove notifications, and
     * integrates the notification source with the current run loop.
     *
     * @returns @c std::expected<void, DeviceMonitorError> with the error code on failure.
     */
    std::expected<void, DeviceMonitorError> start() override;

    /**
     * @brief Stops monitoring for device events.
     *
     * Removes the notification source from the run loop and releases IOKit resources. Safe to call
     * even if monitoring is not running.
     */
    void stop() override;

    /**
     * @brief Checks if monitoring is currently active.
     * @returns @c true if monitoring is active, @c false otherwise.
     */
    bool isRunning() const override;

private:
    static void deviceAddedCallback(void *refcon, io_iterator_t iterator);
    static void deviceRemovedCallback(void *refcon, io_iterator_t iterator);
    void drainIterator(io_iterator_t iterator);

    IONotificationPortRef notificationPort_ = nullptr;
    CFRunLoopSourceRef runLoopSource_ = nullptr;
    io_iterator_t addedIterator_ = 0;
    io_iterator_t removedIterator_ = 0;
    bool running_ = false;
};
