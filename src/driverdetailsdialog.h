#pragma once

#include <QDialog>
#include <QIcon>
#include <QLabel>
#include <QListWidget>
#include <QString>
#include <QStringList>

class DriverDetailsDialog : public QDialog {
    Q_OBJECT

public:
    explicit DriverDetailsDialog(QWidget *parent = nullptr);
    void setDriverName(const QString &driverName);
    void setCategoryIcon(const QIcon &icon);

private Q_SLOTS:
    void onFileSelectionChanged();

private:
    void populateDriverFiles();
    void updateFileDetails(const QString &modulePath);

    struct ModuleInfo {
        QString filename;
        QString version;
        QString author;
        QString description;
        QString license;
        QString srcversion;
        QString alias;
        QString depends;
        QString signer;
        QString sigKey;
    };

    ModuleInfo getModuleInfo(const QString &moduleName);

    QString driverName_;
    QIcon categoryIcon_;

    // UI elements
    QLabel *labelIcon_;
    QLabel *labelDeviceName_;
    QListWidget *listDriverFiles_;
    QLabel *labelProvider_;
    QLabel *labelProviderValue_;
    QLabel *labelFileVersion_;
    QLabel *labelFileVersionValue_;
    QLabel *labelCopyright_;
    QLabel *labelCopyrightValue_;
    QLabel *labelDigitalSigner_;
    QLabel *labelDigitalSignerValue_;
};
