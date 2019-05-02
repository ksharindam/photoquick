#ifndef DIALOGS_H
#define DIALOGS_H

#include "ui_resize_dialog.h"
#include <QPixmap>
#include <QTimer>

// Dialog to set JPG image quality for saving
class QualityDialog : public QDialog
{
    Q_OBJECT
public:
    QualityDialog(QWidget *parent, QPixmap &pm);
    QPixmap pixmap;
    QSpinBox *qualitySpin;
    QLabel *sizeLabel;
    QTimer *timer;
public slots:
    void checkFileSize();
    void toggleCheckSize(bool checked);
};

// ResizeDialog object to get required image size
class ResizeDialog : public QDialog, public Ui_ResizeDialog
{
    Q_OBJECT
public:
    ResizeDialog(QWidget *parent, int img_width, int img_height);
public slots:
    void toggleAdvanced(bool checked);
    void onValueChange(int value);
    void onValueChange(double value) {onValueChange(int(value));}
};

#endif
