#pragma once
/* This file is a part of qmageview program, which is GPLv3 licensed */

#include "ui_mainwindow.h"
#include "canvas.h"
#include <QTimer>


class Window : public QMainWindow, Ui_MainWindow
{
    Q_OBJECT
public:
    Window();
    void openImage(QString filename);
    void saveImage(QString filename);
    void adjustWindowSize(bool animation=false);
    //Variables declaration
    Canvas *canvas;
    int screen_width, screen_height, offset_x, offset_y, btnboxwidth;
    QTimer *timer;      // Slideshow timer
    QString filename;
private:
    void connectSignals();
    float fitToScreenScale(QImage img);
    float fitToWindowScale(QImage img);
    void disableButtons(bool disable);
    void closeEvent(QCloseEvent *ev);
public slots:
    void openFile();
    void overwrite();
    void saveAs();
    void saveACopy();
    void resizeImage();
    void cropImage();
    void addBorder();
    void createPhotoGrid();
    void createPhotoCollage();
    // filters
    void toGrayScale();
    void toBlacknWhite();
    void adaptiveThresh();
    void blur();
    void sharpenImage();
    void reduceSpeckleNoise();
    void removeDust();
    void sigmoidContrast(); // Enhance low light images
    void whiteBalance();
    // file and view options
    void openPrevImage();
    void openNextImage();
    void zoomInImage();
    void zoomOutImage();
    void origSizeImage();
    void rotateLeft();
    void rotateRight();
    void mirror();
    void perspectiveTransform();
    void playSlideShow(bool checked);
    void updateStatus();
};

void waitFor(int millisec);
// rounds up a number to given decimal point
float roundOff(float num, int dec);

