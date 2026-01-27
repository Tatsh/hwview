// SPDX-License-Identifier: MIT
#include "iokitmonitor.h"

IOKitMonitor::IOKitMonitor(QObject *parent) : DeviceMonitor(parent) {
}

IOKitMonitor::~IOKitMonitor() {
    stop();
}

std::expected<void, DeviceMonitorError> IOKitMonitor::start() {
    if (running_) {
        return {};
    }

    // Create notification port
    notificationPort_ = IONotificationPortCreate(kIOMainPortDefault);
    if (!notificationPort_) {
        return std::unexpected(DeviceMonitorError::NotificationPortFailed);
    }

    // Get run loop source and add to current run loop
    runLoopSource_ = IONotificationPortGetRunLoopSource(notificationPort_);
    if (!runLoopSource_) {
        IONotificationPortDestroy(notificationPort_);
        notificationPort_ = nullptr;
        return std::unexpected(DeviceMonitorError::RunLoopSourceFailed);
    }

    CFRunLoopAddSource(CFRunLoopGetCurrent(), runLoopSource_, kCFRunLoopDefaultMode);

    // Create matching dictionary for all IOService objects
    // We need to copy it because each registration consumes the dictionary
    CFMutableDictionaryRef matchingDict = IOServiceMatching(kIOServiceClass);
    if (!matchingDict) {
        stop();
        return std::unexpected(DeviceMonitorError::MatchingDictionaryFailed);
    }

    // Retain the dictionary so we can use it twice (once for add, once for remove)
    CFRetain(matchingDict);

    // Register for device added notifications
    kern_return_t kr = IOServiceAddMatchingNotification(notificationPort_,
                                                        kIOFirstMatchNotification,
                                                        matchingDict,
                                                        deviceAddedCallback,
                                                        this,
                                                        &addedIterator_);
    if (kr != KERN_SUCCESS) {
        CFRelease(matchingDict);
        stop();
        return std::unexpected(DeviceMonitorError::AddNotificationFailed);
    }

    // Drain the iterator to arm the notification (required by IOKit)
    drainIterator(addedIterator_);

    // Register for device removed notifications
    kr = IOServiceAddMatchingNotification(notificationPort_,
                                          kIOTerminatedNotification,
                                          matchingDict,
                                          deviceRemovedCallback,
                                          this,
                                          &removedIterator_);
    if (kr != KERN_SUCCESS) {
        stop();
        return std::unexpected(DeviceMonitorError::RemoveNotificationFailed);
    }

    // Drain the iterator to arm the notification
    drainIterator(removedIterator_);

    running_ = true;
    return {};
}

void IOKitMonitor::stop() {
    running_ = false;

    if (addedIterator_) {
        IOObjectRelease(addedIterator_);
        addedIterator_ = 0;
    }

    if (removedIterator_) {
        IOObjectRelease(removedIterator_);
        removedIterator_ = 0;
    }

    if (runLoopSource_) {
        CFRunLoopRemoveSource(CFRunLoopGetCurrent(), runLoopSource_, kCFRunLoopDefaultMode);
        runLoopSource_ = nullptr;
    }

    if (notificationPort_) {
        IONotificationPortDestroy(notificationPort_);
        notificationPort_ = nullptr;
    }
}

bool IOKitMonitor::isRunning() const {
    return running_;
}

void IOKitMonitor::deviceAddedCallback(void *refcon, io_iterator_t iterator) {
    auto *monitor = static_cast<IOKitMonitor *>(refcon);

    // Drain the iterator to re-arm the notification and count devices
    auto hasDevices = false;
    io_service_t service;
    while ((service = IOIteratorNext(iterator)) != 0) {
        hasDevices = true;
        IOObjectRelease(service);
    }

    if (hasDevices) {
        Q_EMIT monitor->deviceChanged();
    }
}

void IOKitMonitor::deviceRemovedCallback(void *refcon, io_iterator_t iterator) {
    auto *monitor = static_cast<IOKitMonitor *>(refcon);

    // Drain the iterator to re-arm the notification and count devices
    auto hasDevices = false;
    io_service_t service;
    while ((service = IOIteratorNext(iterator)) != 0) {
        hasDevices = true;
        IOObjectRelease(service);
    }

    if (hasDevices) {
        Q_EMIT monitor->deviceChanged();
    }
}

void IOKitMonitor::drainIterator(io_iterator_t iterator) {
    io_service_t service;
    while ((service = IOIteratorNext(iterator)) != 0) {
        IOObjectRelease(service);
    }
}
