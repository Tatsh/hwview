#include "setupapimonitor.h"

#include <dbt.h>

const wchar_t *SetupApiMonitor::kWindowClassName = L"HwViewDeviceNotifyWindow";

SetupApiMonitor::SetupApiMonitor(QObject *parent) : DeviceMonitor(parent) {
}

SetupApiMonitor::~SetupApiMonitor() {
    stop();
}

std::expected<void, DeviceMonitorError> SetupApiMonitor::start() {
    if (running_) {
        return {};
    }

    // Register window class
    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.lpfnWndProc = windowProc;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.lpszClassName = kWindowClassName;

    if (!RegisterClassExW(&wc)) {
        auto error = GetLastError();
        if (error != ERROR_CLASS_ALREADY_EXISTS) {
            return std::unexpected(DeviceMonitorError::WindowClassRegisterFailed);
        }
    }

    // Create message-only window
    messageWindow_ = CreateWindowExW(0,
                                     kWindowClassName,
                                     L"HwView Device Monitor",
                                     0,
                                     0,
                                     0,
                                     0,
                                     0,
                                     HWND_MESSAGE, // Message-only window
                                     nullptr,
                                     GetModuleHandle(nullptr),
                                     this); // Pass this pointer for later retrieval

    if (!messageWindow_) {
        return std::unexpected(DeviceMonitorError::WindowCreationFailed);
    }

    // Store this pointer in window user data
    SetWindowLongPtrW(messageWindow_, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

    // Register for device notifications
    if (auto result = registerForDeviceNotifications(); !result) {
        DestroyWindow(messageWindow_);
        messageWindow_ = nullptr;
        return result;
    }

    running_ = true;
    return {};
}

void SetupApiMonitor::stop() {
    running_ = false;

    if (deviceNotification_) {
        UnregisterDeviceNotification(deviceNotification_);
        deviceNotification_ = nullptr;
    }

    if (messageWindow_) {
        DestroyWindow(messageWindow_);
        messageWindow_ = nullptr;
    }
}

bool SetupApiMonitor::isRunning() const {
    return running_;
}

std::expected<void, DeviceMonitorError> SetupApiMonitor::registerForDeviceNotifications() {
    // Register for all device interface changes
    DEV_BROADCAST_DEVICEINTERFACE filter = {};
    filter.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
    filter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
    // Leave dbcc_classguid as zero to receive notifications for all device classes

    deviceNotification_ = RegisterDeviceNotificationW(
        messageWindow_, &filter, DEVICE_NOTIFY_WINDOW_HANDLE | DEVICE_NOTIFY_ALL_INTERFACE_CLASSES);

    if (!deviceNotification_) {
        return std::unexpected(DeviceMonitorError::DeviceNotificationFailed);
    }

    return {};
}

LRESULT CALLBACK SetupApiMonitor::windowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_DEVICECHANGE) {
        auto *monitor = reinterpret_cast<SetupApiMonitor *>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));

        if (monitor && monitor->running_) {
            switch (wParam) {
            case DBT_DEVICEARRIVAL:
            case DBT_DEVICEREMOVECOMPLETE:
                // Device was added or removed
                Q_EMIT monitor->deviceChanged();
                break;
            default:
                break;
            }
        }
        return TRUE;
    }

    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

bool SetupApiMonitor::nativeEventFilter(const QByteArray &eventType,
                                        void *message,
                                        qintptr *result) {
    Q_UNUSED(eventType);
    Q_UNUSED(message);
    Q_UNUSED(result);
    // The message-only window handles events via windowProc, not through Qt's event system
    return false;
}
