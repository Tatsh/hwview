#pragma once

#include <QObject>

#ifdef Q_OS_LINUX
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
 * @note This class is only functional on Linux. On other platforms, a stub implementation is
 *       provided that does nothing.
 *
 * Example usage:
 * @code
 * UdevMonitor monitor(udevContext);
 * connect(&monitor, &UdevMonitor::deviceChanged, this, &MyClass::onDeviceChanged);
 * monitor.start();
 * @endcode
 */
class UdevMonitor : public QObject {
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
    bool start();

    /**
     * @brief Stops monitoring for device events.
     *
     * Disables the socket notifier and releases the udev monitor resources. Safe to call even if
     * monitoring is not running.
     */
    void stop();

    /**
     * @brief Checks if monitoring is currently active.
     * @returns @c true if monitoring is active, @c false otherwise.
     */
    bool isRunning() const;

Q_SIGNALS:
    /**
     * @brief Emitted when a device is added or removed.
     *
     * This signal is emitted for "add" and "remove" udev actions only. Other actions (e.g.,
     * "change", "move") are ignored.
     */
    void deviceChanged();

private Q_SLOTS:
    void onUdevEvent();

private:
    struct udev *ctx_;
    struct udev_monitor *monitor_ = nullptr;
    QSocketNotifier *notifier_ = nullptr;
    bool running_ = false;
};
#else
/**
 * @brief Stub class for UdevMonitor on non-Linux platforms.
 *
 * This class provides the same interface as the Linux implementation but does nothing. All methods
 * are no-ops and start() always returns false.
 */
class UdevMonitor : public QObject {
    Q_OBJECT

public:
    /**
     * @brief Constructs a stub UdevMonitor.
     * @param ctx Ignored on non-Linux platforms.
     * @param parent Optional parent QObject for memory management.
     */
    explicit UdevMonitor(void *ctx, QObject *parent = nullptr) : QObject(parent) {
        Q_UNUSED(ctx);
    }
    ~UdevMonitor() override = default;

    /**
     * @brief Stub implementation that always fails.
     * @returns Always returns @c false on non-Linux platforms.
     */
    bool start() {
        return false;
    }

    /**
     * @brief Stub implementation that does nothing.
     */
    void stop() {
    }

    /**
     * @brief Stub implementation.
     * @returns Always returns @c false on non-Linux platforms.
     */
    bool isRunning() const {
        return false;
    }

Q_SIGNALS:
    /**
     * @brief Signal declared for API compatibility but never emitted on non-Linux platforms.
     */
    void deviceChanged();
};
#endif
