#pragma once

#include <QObject>

#ifdef Q_OS_LINUX
#include <libudev.h>

QT_BEGIN_NAMESPACE
class QSocketNotifier;
QT_END_NAMESPACE

// Monitors udev events for device add/remove notifications.
// Uses QSocketNotifier to integrate with Qt's event loop.
class UdevMonitor : public QObject {
    Q_OBJECT

public:
    explicit UdevMonitor(struct udev *ctx, QObject *parent = nullptr);
    ~UdevMonitor() override;

    // Start monitoring for device events
    bool start();

    // Stop monitoring
    void stop();

    // Check if monitoring is active
    bool isRunning() const;

Q_SIGNALS:
    // Emitted when a device is added or removed
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
// Stub class for non-Linux platforms
class UdevMonitor : public QObject {
    Q_OBJECT

public:
    explicit UdevMonitor(void *, QObject *parent = nullptr) : QObject(parent) {
    }
    ~UdevMonitor() override = default;

    bool start() {
        return false;
    }
    void stop() {
    }
    bool isRunning() const {
        return false;
    }

Q_SIGNALS:
    void deviceChanged();
};
#endif
