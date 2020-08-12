#pragma once
#include <QDialog>
#include <QLabel>
#include <QSpinBox>
#include <QComboBox>
#include <QCheckBox>
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

// dialog to choose paper size
class PaperSizeDialog : public QDialog
{
public:
    QComboBox *combo;
    QCheckBox *landscape;
    PaperSizeDialog(QWidget *parent, bool landscapeMode);
};

// dialog to choose border width and size
class ExpandBorderDialog : public QDialog
{
public:
    QComboBox *combo;
    QSpinBox *widthSpin;
    ExpandBorderDialog(QWidget *parent, int border_w);
};
