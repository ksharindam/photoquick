#pragma once
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
    QMap<QString, QMenu*> menu_dict;
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
    void autoResizeAndSave();
    void exportToPdf();
    void deleteFile();
    void reloadImage();
    void resizeImage();
    void cropImage();
    void mirror();
    void perspectiveTransform();
    void addBorder();
    void createPhotoGrid();
    void createPhotoCollage();
    // tools
    void magicEraser();
    void iScissor();
    // filters
    void toGrayScale();
    void toBlacknWhite();
    void adaptiveThresh();
    void blur();
    void sharpenImage();
    void reduceSpeckleNoise();
    void removeDust();
    void sigmoidContrast(); // Enhance low light images
    void enhanceLight();
    void whiteBalance();
    void enhanceColors();
    void grayWorldFilter();
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
};

QString getNextFileName(QString current);
QString getNewFileName(QString filename);
