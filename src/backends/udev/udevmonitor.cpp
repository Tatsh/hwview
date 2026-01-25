#include <QtCore/QSocketNotifier>

#include "udevmonitor.h"

UdevMonitor::UdevMonitor(struct udev *ctx, QObject *parent) : DeviceMonitor(parent), ctx_(ctx) {
}

UdevMonitor::~UdevMonitor() {
    stop();
}

std::expected<void, DeviceMonitorError> UdevMonitor::start() {
    if (running_) {
        return {};
    }

    // Create a udev monitor for kernel events
    monitor_ = udev_monitor_new_from_netlink(ctx_, "udev");
    if (!monitor_) {
        return std::unexpected(DeviceMonitorError::MonitorCreationFailed);
    }

    // Enable receiving of events
    if (udev_monitor_enable_receiving(monitor_) < 0) {
        udev_monitor_unref(monitor_);
        monitor_ = nullptr;
        return std::unexpected(DeviceMonitorError::MonitorEnableFailed);
    }

    // Get the file descriptor for the monitor
    int fd = udev_monitor_get_fd(monitor_);
    if (fd < 0) {
        udev_monitor_unref(monitor_);
        monitor_ = nullptr;
        return std::unexpected(DeviceMonitorError::FileDescriptorFailed);
    }

    // Create a socket notifier to watch the file descriptor
    notifier_ = new QSocketNotifier(fd, QSocketNotifier::Read, this);
    connect(notifier_, &QSocketNotifier::activated, this, &UdevMonitor::onUdevEvent);

    running_ = true;
    return {};
}

void UdevMonitor::stop() {
    if (!running_) {
        return;
    }

    running_ = false;

    if (notifier_) {
        notifier_->setEnabled(false);
        delete notifier_;
        notifier_ = nullptr;
    }

    if (monitor_) {
        udev_monitor_unref(monitor_);
        monitor_ = nullptr;
    }
}

bool UdevMonitor::isRunning() const {
    return running_;
}

void UdevMonitor::onUdevEvent() {
    if (!monitor_) {
        return;
    }

    // Receive the device event
    struct udev_device *dev = udev_monitor_receive_device(monitor_);
    if (!dev) {
        return;
    }

    // Check if this is an add or remove action
    const char *action = udev_device_get_action(dev);
    if (action) {
        QString actionStr = QString::fromLatin1(action);
        if (actionStr == QLatin1String("add") || actionStr == QLatin1String("remove")) {
            Q_EMIT deviceChanged();
        }
    }

    udev_device_unref(dev);
}
