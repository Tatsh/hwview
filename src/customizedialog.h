#pragma once

#include <QtWidgets/QDialog>

QT_BEGIN_NAMESPACE
class QCheckBox;
class QDialogButtonBox;
QT_END_NAMESPACE

/**
 * @brief Dialog for customizing view settings.
 *
 * This dialog allows users to configure display options such as
 * whether to show device icons, expand all nodes on load, and
 * show the driver column in the device tree.
 */
class CustomizeDialog : public QDialog {
    Q_OBJECT

public:
    /**
     * @brief Constructs a CustomizeDialog.
     * @param parent Optional parent widget.
     */
    explicit CustomizeDialog(QWidget *parent = nullptr);

    /**
     * @brief Returns whether device icons should be shown.
     * @returns @c true if icons should be displayed, @c false otherwise.
     */
    bool showDeviceIcons() const;

    /**
     * @brief Sets whether device icons should be shown.
     * @param show @c true to show icons, @c false to hide them.
     */
    void setShowDeviceIcons(bool show);

    /**
     * @brief Returns whether all nodes should expand on load.
     * @returns @c true if all nodes expand, @c false otherwise.
     */
    bool expandAllOnLoad() const;

    /**
     * @brief Sets whether all nodes should expand on load.
     * @param expand @c true to expand all, @c false for default.
     */
    void setExpandAllOnLoad(bool expand);

    /**
     * @brief Returns whether the driver column should be shown.
     * @returns @c true if the driver column is visible, @c false otherwise.
     */
    bool showDriverColumn() const;

    /**
     * @brief Sets whether the driver column should be shown.
     * @param show @c true to show the column, @c false to hide it.
     */
    void setShowDriverColumn(bool show);

private:
    QCheckBox *showIconsCheckBox;
    QCheckBox *expandAllCheckBox;
    QCheckBox *showDriverColumnCheckBox;
    QDialogButtonBox *buttonBox;
};
