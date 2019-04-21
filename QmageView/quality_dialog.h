#ifndef QUALITY_DIALOG_H
#define QUALITY_DIALOG_H

#include <QDialog>
#include <QWidget>
#include <QPixmap>
#include <QSpinBox>
#include <QLabel>
#include <QTimer>


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

#endif
