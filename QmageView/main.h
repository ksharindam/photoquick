/*
This file is a part of qmageview program, which is GPLv3 licensed

This holds the declaration of top level objects
*/
#ifndef MAIN_H
#define MAIN_H

#include "ui_mainwindow.h"
#include "ui_resize_dialog.h"
#include "image.h"
#include "photogrid.h"
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
    Image *image;
    QSettings settings;
    int screen_width, screen_height, offset_x, offset_y, btnboxwidth;
    QTimer *timer;
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

void waitFor(int millisec);
float roundOff(float num, int dec);

QT_END_NAMESPACE

#endif // MAIN_H
