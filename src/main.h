#pragma once
#include "ui_mainwindow.h"
#include "canvas.h"
#include <QTimer>

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
    QAction *overwrite_action, *savecopy_action;
    // functions
    Window();
    void openStartupImage();
    void openImage(QString filename);
    void saveImage(QString filename);
    void connectSignals();
    void adjustWindowSize(bool animation=false);
    float fitToScreenScale(QImage img);
    float fitToWindowScale(QImage img);
    void disableButtons(ButtonType type, bool disable);
    void closeEvent(QCloseEvent *ev);
    void addMaskWidget();
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
    void setAspectRatio();
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
    void blur();
    void sharpenImage();
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
