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

// Preview Dialog for filter functions.
// This is Abstract and must be reimplemented
class PreviewDialog : public QDialog
{
    Q_OBJECT
public:
    QImage image;
    float scale;
    QTimer *timer;
    // if the filter applied on scaled image looks same, then image from
    // canvas pixmap is passed, and scale is set 1.0
    // else, the original image is passed, and scale is set to canvas->scale
    PreviewDialog(QLabel *parent, QImage img, float scale);
    void preview(QImage);
public slots:
    void onValueChange();
    // implement run() in subclass. apply filter, then call preview()
    virtual void run() = 0;
signals:
    void previewRequested(const QPixmap&);
};


class LensDialog : public PreviewDialog
{
public:
    float main=20.0;
    float edge=0;
    float zoom=10.0;
    QDoubleSpinBox *mainSpin;
    QDoubleSpinBox *edgeSpin;
    QDoubleSpinBox *zoomSpin;

    LensDialog(QLabel *parent, QImage img, float scale);
    void run();
};


class ThresholdDialog : public PreviewDialog
{
public:
    int thresh;
    QSpinBox *thresholdSpin;

    ThresholdDialog(QLabel *parent, QImage img, float scale);
    void run();
};


class GammaDialog : public PreviewDialog
{
public:
    float gamma = 1.6;
    QDoubleSpinBox *gammaSpin;

    GammaDialog(QLabel *parent, QImage img, float scale);
    void run();
};

