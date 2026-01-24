#pragma once

#include "devicemonitor.h"

#include <libudev.h>

QT_BEGIN_NAMESPACE
class QSocketNotifier;
QT_END_NAMESPACE

/**
 * @brief Monitors udev events for device add/remove notifications.
 *
 * This class uses the udev netlink interface to receive device events from the kernel. It
 * integrates with Qt's event loop using QSocketNotifier for non-blocking operation.
 *
 * Example usage:
 * @code
 * UdevMonitor monitor(udevContext);
 * connect(&monitor, &UdevMonitor::deviceChanged, this, &MyClass::onDeviceChanged);
 * monitor.start();
 * @endcode
 */
class UdevMonitor : public DeviceMonitor {
    Q_OBJECT

public:
    /**
     * @brief Constructs a UdevMonitor.
     * @param ctx The udev context to use for monitoring. Must remain valid for the lifetime of
     *            this object.
     * @param parent Optional parent QObject for memory management.
     */
    explicit UdevMonitor(struct udev *ctx, QObject *parent = nullptr);
    ~UdevMonitor() override;

    /**
     * @brief Starts monitoring for device events.
     *
     * Creates a udev monitor, enables receiving, and sets up a QSocketNotifier to watch the
     * monitor's file descriptor.
     *
     * @returns @c true if monitoring was started successfully, @c false otherwise.
     */
    bool start() override;

    /**
     * @brief Stops monitoring for device events.
     *
     * Disables the socket notifier and releases the udev monitor resources. Safe to call even if
     * monitoring is not running.
     */
    void stop() override;

    /**
     * @brief Checks if monitoring is currently active.
     * @returns @c true if monitoring is active, @c false otherwise.
     */
    bool isRunning() const override;

private Q_SLOTS:
    void onUdevEvent();

private:
    struct udev *ctx_;
    struct udev_monitor *monitor_ = nullptr;
    QSocketNotifier *notifier_ = nullptr;
    bool running_ = false;
};
