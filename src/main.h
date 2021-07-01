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
    Window();
    void openImage(QString filename);
    void saveImage(QString filename);
    void adjustWindowSize(bool animation=false);
    //Variables declaration
    Canvas *canvas;
    ImageData data;
    int screen_width, screen_height;
    int btnboxes_w, statusbar_h, windowdecor_w, windowdecor_h;
    QTimer *timer;      // Slideshow timer
    QMap<QString, QMenu*> menu_dict;
private:
    void connectSignals();
    float fitToScreenScale(QImage img);
    float fitToWindowScale(QImage img);
    void disableButtons(ButtonType type, bool disable);
    void closeEvent(QCloseEvent *ev);
    void addMaskWidget();
public slots:
    void openFile();
    void overwrite();
    void saveAs();
    void saveACopy();
    void autoResizeAndSave();
    void exportToPdf();
    void deleteFile();
    void reloadImage();
    void resizeImage();
    void cropImage();
    void mirror();
    void perspectiveTransform();
    void addBorder();
    void expandImageBorder();
    void createPhotoGrid();
    void createPhotoCollage();
    // tools
    void maskTool();
    void magicEraser();
    void iScissor();
    void removeMaskWidget();
    // filters
    void toGrayScale();
    void applyThreshold();
    void adaptiveThresh();
    void blur();
    void sharpenImage();
    void reduceSpeckleNoise();
    void removeDust();
    void sigmoidContrast(); // Enhance low light images
    void enhanceLight();
    void gammaCorrection();
    void whiteBalance();
    void enhanceColors();
    void vignetteFilter();
    void pencilSketchFilter();
    void grayWorldFilter();
    void lensDistort();
    // info menu
    void imageInfo();
    void showAbout();
    // file and view options
    void openPrevImage();
    void openNextImage();
    void zoomInImage();
    void zoomOutImage();
    void origSizeImage();
    void rotateLeft();
    void rotateRight();
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
