#pragma once
#include <QTimer>
#include <QFileDialog>
#include <QInputDialog>
#include <QColorDialog>
#include <QMessageBox>
#include <QFileInfo>
#include <QPainter>
#include <QPrinter>
#include <QPrintDialog>
#include <QDesktopWidget>
#include <QSettings>
#include <QClipboard>
#include <QMenu>
#include <QRegExp>
#include <QBuffer>
#include <cmath>
#include <QImageWriter>
#include <QDesktopServices>
#include <QUrl>
#include "common.h"
#include "canvas.h"
#include "exif.h"
#include "plugin.h"
#include "dialogs.h"
#include "transform.h"
#include "photogrid.h"
#include "photo_collage.h"
#include "inpaint.h"
#include "iscissor.h"
#include "filters.h"
#include "pdfwriter.h"
#include "ui_mainwindow.h"

#ifndef __PHOTOQUICK_MAIN
#define __PHOTOQUICK_MAIN

typedef enum
{
    FILE_BUTTON,
    VIEW_BUTTON,
    EDIT_BUTTON
} ButtonType;


class Window : public QMainWindow, Ui_MainWindow
{
    Q_OBJECT
public:
    //Variables declaration
    Canvas *canvas;
    ImageData data;
    int screen_width, screen_height;
    int btnboxes_w, statusbar_h, windowdecor_w, windowdecor_h;
    QTimer *timer;      // Slideshow timer
    QMap<QString, QMenu*> menu_dict;
    // functions
    Window();
    void openStartupImage();
    void openImage(QString filename);
    void saveImage(QString filename);
    void connectSignals();
    void adjustWindowSize(bool animation=false);
    QImage resizeImageBicub (QImage, unsigned, unsigned);
    QImage resizeImageBiakima (QImage, unsigned, unsigned);
    float fitToScreenScale(QImage img);
    float fitToWindowScale(QImage img);
    void disableButtons(ButtonType type, bool disable);
    void closeEvent(QCloseEvent *ev);
    void addMaskWidget();
    void blurorbox(int method);
public slots:
    void openFile();
    void openFromClipboard();
    void copyToClipboard();
    void overwrite();
    void saveAs();
    void saveACopy();
    void autoResizeAndSave();
    void exportToPdf();
    void printImage();
    void deleteFile();
    void reloadImage();
    void resizeImage();
    void cropImage();
    void mirror();
    void perspectiveTransform();
    void deWarping();
    void deOblique();
    void addBorder();
    void expandImageBorder();
    void createPhotoGrid();
    void createPhotoCollage();
    // tools
    void maskTool();
    void magicEraser();
    void iScissor();
    void removeMaskWidget();
    // color filters
    void toGrayScale();
    void adjustColorLevels();
    void grayWorldFilter();
    void whiteBalance();
    void enhanceColors();
    // threshold filters
    void applyThreshold();
    void adaptiveThresh();
    // brightness filters
    void adjustGamma();
    void stretchImageContrast();
    void sigmoidContrast();
    // denoise filters
    void reduceSpeckleNoise();
    void removeDust();
    // other filters
    void box();
    void blur();
    void sharpenImage();
    void edgeImage();
    void vignetteFilter();
    void pencilSketchFilter();
    void lensDistort();
    // info menu
    void imageInfo();
    void getPlugins();
    void checkForUpdate();
    void showAbout();
    // file and view options
    void openPrevImage();
    void openNextImage();
    void zoomInImage();
    void zoomOutImage();
    void origSizeImage();
    void rotateLeft();
    void rotateRight();
    void rotateAny();
    void playPause();
    // others
    void loadPlugins();
    void resizeToOptimum();
    void showNotification(QString title, QString message);
    void onEditingFinished();
    void updateStatus();
    void onEscPress();
};

QString getNextFileName(QString current);
QString getNewFileName(QString filename);

#endif /* __PHOTOQUICK_MAIN */
