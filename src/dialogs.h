#pragma once
#include <QDialog>
#include <QLabel>
#include <QSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <QTextEdit>
#include <QImage>
#include <QTimer>

// Dialog to set JPG Options for saving
class JpegDialog : public QDialog
{
    Q_OBJECT
public:
    JpegDialog(QWidget *parent, QImage &img);
    QImage image;
    QSpinBox *qualitySpin;
    QLabel *sizeLabel;
    QTimer *timer;
    QSpinBox *dpiSpin;
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
    Q_OBJECT
public:
    QComboBox *combo;
    QSpinBox *widthSpin;
    QWidget *sidesFrame;
    QCheckBox *leftCheckBox, *rightCheckBox, *topCheckBox, *bottomCheckBox;
    ExpandBorderDialog(QWidget *parent, int border_w);
public slots:
    void toggleAllSides(bool checked);
};

// Preview Dialog for filter functions.
// This is Abstract and must be reimplemented
class PreviewDialog : public QDialog
{
    Q_OBJECT
public:
    QLabel *canvas;
    QImage image;
    float scale;
    QTimer *timer;
    // if the filter applied on scaled image looks same, then image from
    // canvas pixmap is passed, and scale is set 1.0
    // else, the original image is passed, and scale is set to canvas->scale
    PreviewDialog(QLabel *parent, QImage img, float scale);
    // implement getResult() in subclass. which apply filter on input image and returns output
    virtual QImage getResult(QImage img) = 0;
public slots:
    void triggerPreview();
    void preview();
};


class RotateDialog : public PreviewDialog
{
public:
    int angle;
    QSpinBox *angleSpin;

    RotateDialog(QLabel *parent, QImage img, float scale);
    QImage getResult(QImage img);
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
    QImage getResult(QImage img);
};


class ThresholdDialog : public PreviewDialog
{
public:
    int thresh;
    QSpinBox *thresholdSpin;

    ThresholdDialog(QLabel *parent, QImage img, float scale);
    QImage getResult(QImage img);
};


class GammaDialog : public PreviewDialog
{
public:
    float gamma = 1.2;
    QDoubleSpinBox *gammaSpin;

    GammaDialog(QLabel *parent, QImage img, float scale);
    QImage getResult(QImage img);
};


class LevelsWidget : public QLabel
{
    Q_OBJECT
public:
    int drag_slider;
    int left_val, right_val;
    QColor color;
    QPoint click_pos;

    LevelsWidget(QWidget *parent, int left_val, int right_val, QColor color);
    void redraw(int l_val, int r_val);
    void mousePressEvent(QMouseEvent *ev);
    void mouseReleaseEvent(QMouseEvent *ev);
    void mouseMoveEvent(QMouseEvent *ev);
signals:
    void valueChanged();
};


class LevelsDialog : public PreviewDialog
{
public:
    LevelsWidget *inputRSlider, *inputGSlider, *inputBSlider;
    LevelsWidget *outputRSlider, *outputGSlider, *outputBSlider;

    LevelsDialog(QLabel *parent, QImage img, float scale);
    QImage getResult(QImage img);
    void run();
};


class UpdateDialog : public QDialog
{
    Q_OBJECT
public:
    UpdateDialog(QWidget *parent);

    QLabel *currentVersionLabel;
    QLabel *latestVersionLabel;
    QTextEdit *textView;
    QPushButton *updateBtn;
    QPushButton *closeBtn;
    QWidget *buttonBox;

    QString latest_version;

    void download();

private slots:
    void checkForUpdate();
};
