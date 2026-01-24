#pragma once

#include "devicemonitor.h"

#include <windows.h>

/**
 * @brief Monitors Windows device events for device add/remove notifications.
 *
 * This class uses the Windows Device Notification API to receive device events. It creates a
 * hidden message-only window to receive @c WM_DEVICECHANGE messages and integrates with Qt's
 * event loop using a native event filter.
 *
 * Example usage:
 * @code
 * SetupApiMonitor monitor;
 * connect(&monitor, &SetupApiMonitor::deviceChanged, this, &MyClass::onDeviceChanged);
 * if (auto result = monitor.start(); !result) {
 *     // Handle error
 * }
 * @endcode
 */
class SetupApiMonitor : public DeviceMonitor {
    Q_OBJECT

public:
    /**
     * @brief Constructs a @c SetupApiMonitor.
     * @param parent Optional parent @c QObject for memory management.
     */
    explicit SetupApiMonitor(QObject *parent = nullptr);
    ~SetupApiMonitor() override;

    /**
     * @brief Starts monitoring for device events.
     *
     * Creates a hidden message-only window and registers for device interface change
     * notifications using @c RegisterDeviceNotification.
     *
     * @returns @c std::expected<void, DeviceMonitorError> with the error code on failure.
     */
    std::expected<void, DeviceMonitorError> start() override;

    /**
     * @brief Stops monitoring for device events.
     *
     * Unregisters device notifications and destroys the message window. Safe to call even if
     * monitoring is not running.
     */
    void stop() override;

    /**
     * @brief Checks if monitoring is currently active.
     * @returns @c true if monitoring is active, @c false otherwise.
     */
    bool isRunning() const override;

protected:
    bool nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result);

private:
    static LRESULT CALLBACK windowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    std::expected<void, DeviceMonitorError> registerForDeviceNotifications();

    HWND messageWindow_ = nullptr;
    HDEVNOTIFY deviceNotification_ = nullptr;
    bool running_ = false;

    static const wchar_t *kWindowClassName;
};
