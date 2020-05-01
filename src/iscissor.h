#pragma once
//*************** Intelligent Scissor GUI *******************
#include "ui_iscissor_dialog.h"
#include <QImage>
#include <QPainter>
#include <QMouseEvent>
#include <vector>

typedef unsigned int uint;

class IntBuffer
{
public:
    uint *data;
    int width;
    int height;
    IntBuffer(int width, int height);
    ~IntBuffer();
};

class GradMap
{
public:
    int width;
    int height;
    uchar *grad_mag;
    GradMap(QImage image);
    int linkCost(int x, int y, int link);
    ~GradMap();
};

typedef enum {
    NO_SEED,
    SEED_PLACED,
    PATH_CLOSED
} SeedMode;

typedef enum {
    TRANSPERANT,
    COLOR_WHITE,
    COLOR_OTHER
} BgColorType;

class IScissorCanvas : public QLabel
{
    Q_OBJECT
public:
    QImage image; // original unchanged image
    QImage image_scaled;
    float scale = 1.0;
    QPainter painter;

    GradMap *grad_map = 0;
    bool smooth_mask = true;
    bool masked_image_ready = false;
    int bg_color_type = TRANSPERANT;
    QRgb bg_color = 0x80ccff; //light blue

    SeedMode seed_mode = NO_SEED;
    std::vector<QPoint> seeds;
    std::vector<QPoint> shortPath;
    std::vector<std::vector<QPoint>> fullPath;
    std::vector<QPoint> redoStack;

    IScissorCanvas(QImage &img, QWidget *parent);
    void scaleBy(float factor);
    void scaleImage();
    void redraw();
    void mousePressEvent(QMouseEvent *ev);
    void mouseMoveEvent(QMouseEvent *ev);
    void undo();
    void redo();
    void drawSeedToCursorPath();
    void drawSeedToSeedPath();
    void drawFullPath();
    void calcShortPath(QPoint from, QPoint to);
    void checkPathClosed();
    void getMaskedImage(QPoint click_pos);
    QImage getResultImage();
signals:
    void undoAvailable(bool);
    void redoAvailable(bool);
    void maskedImageReady();
    void messageUpdated(const QString&);
};


class IScissorDialog : public QDialog, public Ui_IScissorDialog
{
    Q_OBJECT
public:
    IScissorCanvas *canvas;
    IScissorDialog(QImage &img, QWidget *parent);
    void keyPressEvent(QKeyEvent *ev);
    void done(int);
public slots:
    void undo();
    void redo();
    void zoomIn();
    void zoomOut();
    void setBgType(int type);
    void toggleSmoothMask(bool checked);
    void onMaskedImageReady();
};

void findOptimalPath(GradMap *grad_map, IntBuffer &dp_buff,
                    int x1, int y1, int xs, int ys);
std::vector<QPoint> plotShortPath(IntBuffer *dp_buff, int x1, int y1,
                    int target_x, int target_y);

void floodfill(QImage &img, QPoint pos, QRgb oldColor, QRgb newColor);
