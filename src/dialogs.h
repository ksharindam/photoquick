#pragma once
#include <QDialog>
#include <QLabel>
#include <QSpinBox>
#include <QImage>
#include <QTimer>

// Dialog to set JPG image quality for saving
class QualityDialog : public QDialog
{
    Q_OBJECT
public:
    QualityDialog(QWidget *parent, QImage &img);
    QImage image;
    QSpinBox *qualitySpin;
    QLabel *sizeLabel;
    QTimer *timer;
public slots:
    void checkFileSize();
    void toggleCheckSize(bool checked);
};
