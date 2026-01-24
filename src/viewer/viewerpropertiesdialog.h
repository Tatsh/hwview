#pragma once

#include <QDialog>
#include <QIcon>

#include "exporteddata.h"

QT_BEGIN_NAMESPACE
class QComboBox;
class QLabel;
class QListWidget;
class QPlainTextEdit;
class QPushButton;
class QTabWidget;
QT_END_NAMESPACE

/**
 * @brief Dialog that displays properties of a device from exported data.
 *
 * This is a read-only version of PropertiesDialog that works with
 * exported device data rather than live system queries.
 */
class ViewerPropertiesDialog : public QDialog {
    Q_OBJECT

public:
    explicit ViewerPropertiesDialog(QWidget *parent = nullptr);

    /**
     * @brief Sets the device to display properties for.
     * @param syspath The system path of the device in the export.
     */
    void setDeviceSyspath(const QString &syspath);

    /**
     * @brief Sets the category icon to display in the header.
     * @param icon The icon representing the device category.
     */
    void setCategoryIcon(const QIcon &icon);

private Q_SLOTS:
    void onPropertySelectionChanged(int index);
    void onDriverDetailsClicked();

private:
    void setupUi();
    void populateGeneralTab();
    void populateDriverTab();
    void populateDetailsTab();
    QString getCategoryName(int category);

    QString syspath_;
    const ExportedData::Device *device_ = nullptr;
    QIcon categoryIcon_;

    // UI elements
    QTabWidget *tabWidget_;

    // General tab
    QLabel *labelIconGeneral_;
    QLabel *labelDeviceNameGeneral_;
    QLabel *labelDeviceTypeValue_;
    QLabel *labelManufacturerValue_;
    QLabel *labelLocationValue_;
    QPlainTextEdit *textEditDeviceStatus_;

    // Driver tab
    QLabel *labelIconDriver_;
    QLabel *labelDeviceNameDriver_;
    QLabel *labelDriverProviderValue_;
    QLabel *labelDriverDateValue_;
    QLabel *labelDriverVersionValue_;
    QLabel *labelDigitalSignerValue_;

    // Details tab
    QLabel *labelIconDetails_;
    QLabel *labelDeviceNameDetails_;
    QComboBox *comboBoxDetailsProperty_;
    QListWidget *listWidgetDetailsPropertyValue_;

    // Driver details button
    QPushButton *buttonDriverDetails_;

    struct PropertyMapping {
        QString displayName;
        QString jsonKey;
    };
    QList<PropertyMapping> propertyMappings_;
};
