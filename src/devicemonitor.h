#pragma once

#include <QObject>

#include <expected>

/**
 * @brief Error codes for device monitor operations.
 */
enum class DeviceMonitorError {
    None,                      ///< No error
    NotificationPortFailed,    ///< Failed to create notification port (macOS)
    RunLoopSourceFailed,       ///< Failed to get run loop source (macOS)
    MatchingDictionaryFailed,  ///< Failed to create matching dictionary (macOS)
    AddNotificationFailed,     ///< Failed to register for add notifications (macOS)
    RemoveNotificationFailed,  ///< Failed to register for remove notifications (macOS)
    MonitorCreationFailed,     ///< Failed to create udev monitor (Linux)
    MonitorEnableFailed,       ///< Failed to enable udev monitor receiving (Linux)
    FileDescriptorFailed,      ///< Failed to get file descriptor (Linux)
    WindowClassRegisterFailed, ///< Failed to register window class (Windows)
    WindowCreationFailed,      ///< Failed to create message window (Windows)
    DeviceNotificationFailed,  ///< Failed to register device notification (Windows)
};

/**
 * @brief Abstract interface for platform-specific device monitoring.
 *
 * This class defines the interface for monitoring device add/remove events. Platform-specific
 * implementations (e.g., @c UdevMonitor on Linux) inherit from this class.
 *
 * On platforms without device monitoring support, no implementation is provided and @c DeviceCache
 * will not create a monitor.
 */
class DeviceMonitor : public QObject {
    Q_OBJECT

public:
    /**
     * @brief Constructs a @c DeviceMonitor.
     * @param parent Optional parent @c QObject for memory management.
     */
    explicit DeviceMonitor(QObject *parent = nullptr) : QObject(parent) {
    }

    ~DeviceMonitor() override = default;

    /**
     * @brief Starts monitoring for device events.
     * @returns @c std::expected<void, DeviceMonitorError> with the error code on failure.
     */
    virtual std::expected<void, DeviceMonitorError> start() = 0;

    /**
     * @brief Stops monitoring for device events.
     *
     * Safe to call even if monitoring is not running.
     */
    virtual void stop() = 0;

    /**
     * @brief Checks if monitoring is currently active.
     * @returns @c true if monitoring is active, @c false otherwise.
     */
    virtual bool isRunning() const = 0;

Q_SIGNALS:
    /**
     * @brief Emitted when a device is added or removed.
     */
    void deviceChanged();
};
