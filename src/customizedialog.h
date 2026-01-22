#pragma once

#include <QDialog>

QT_BEGIN_NAMESPACE
class QCheckBox;
class QDialogButtonBox;
QT_END_NAMESPACE

class CustomizeDialog : public QDialog {
    Q_OBJECT

public:
    explicit CustomizeDialog(QWidget *parent = nullptr);

    bool showDeviceIcons() const;
    void setShowDeviceIcons(bool show);

    bool expandAllOnLoad() const;
    void setExpandAllOnLoad(bool expand);

    bool showDriverColumn() const;
    void setShowDriverColumn(bool show);

private:
    QCheckBox *showIconsCheckBox;
    QCheckBox *expandAllCheckBox;
    QCheckBox *showDriverColumnCheckBox;
    QDialogButtonBox *buttonBox;
};
