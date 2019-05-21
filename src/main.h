/*
This file is a part of qmageview program, which is GPLv3 licensed

This holds the declaration of top level objects
*/
#ifndef MAIN_H
#define MAIN_H

#include "ui_mainwindow.h"
#include "image.h"
#include <QSettings>
#include <QTimer>

QT_BEGIN_NAMESPACE
class Window : public QMainWindow, Ui_MainWindow
{
    Q_OBJECT
public:
    Window();
    //~Window();
    void openImage(QString filename);
    void adjustWindowSize(bool animation=false);
    //Variables declaration
    Image *image;       // Canvas Widget
    QSettings settings;
    int screen_width, screen_height, offset_x, offset_y, btnboxwidth;
    QTimer *timer;      // Slideshow timer
    QString filepath;
private:
    void connectSignals();
    float getOptimumScale(QPixmap pixmap);
    void disableButtons(bool disable);
    void closeEvent(QCloseEvent *ev);
    QList<QWidget *> crop_widgets;
public slots:
    void openFile();
    void saveFile();
    void resizeImage();
    void cropImage();
    void cancelCropping();
    void addBorder();
    void createPhotoGrid();
    void toGrayScale();
    void toBlacknWhite();
    void adaptiveThresh();
    void blur();
    void openPrevImage();
    void openNextImage();
    void zoomInImage();
    void zoomOutImage();
    void origSizeImage();
    void rotateLeft();
    void rotateRight();
    void playSlideShow(bool checked);
    void updateStatus();

};

void waitFor(int millisec);
float roundOff(float num, int dec);

QT_END_NAMESPACE

#endif // MAIN_H
