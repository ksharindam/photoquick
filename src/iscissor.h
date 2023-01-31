#pragma once
/* Intelligent Scissor and Manual eraser for background removal */

#include "canvas.h"
#include <QPainter>
#include <QTimer>
#include <vector>
#include <QColorDialog>
#include <QButtonGroup>
#include <QComboBox>
#include <QDialogButtonBox>
#include <cmath>
#include "common.h"
#include "filters.h"
#include "ui_iscissor_dialog.h"
/*
How drawing works in Scissor :
 first it scales image to create image_scaled,
 when placing seeds, draws seed to seed permanent path on image_scaled and
 when showing temporary seed to cursor path, it copies image_scaled and draws over it
 finally it creates a temporary mask, floodfill it and make unmasked areas
 in image transperant

Mask mode :
 when mouse is clicked inside loop, mask is generated, and image_scaled is
 updated and shown to canvas.

How drawing works in Eraser :
 First brush is created, and that is scaled to make brush_scaled.
 Image is scaled to make image_scaled (ARGB_Premultiplied).
 When mouse is dragged, brush is drawn on image, and brush_scaled is drawn on image_scaled.
 On mouse release, image_scaled is regenerated, and shown on canvas.

Mask mode :
 The brush is solid white brush which is drawn on mask.
 Brush_scaled is a semi-transperant green brush, drawn on image_scaled
 On mouse release image_scaled is generated from mask and image.
*/

#ifndef __PHOTOQUICK_ISCISSOR
#define __PHOTOQUICK_ISCISSOR

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
    TOOL_ISCISSOR=1,
    TOOL_ERASER=2
} ToolType;// matches the button id

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

// IScissor dialog mode
enum {
    ERASER_MODE,
    MASK_MODE
};

class IScissorDialog : public QDialog, public Ui_IScissorDialog
{
    Q_OBJECT
public:
    IScissorDialog(QImage &img, int mode, QWidget *parent);

    int mode;

    QImage image; // original unchanged image
    QImage mask;
    QImage image_scaled;
    float scale;
    QPainter painter;
    PaintCanvas *canvas;

    int bg_color_type = TRANSPERANT;
    QRgb bg_color = 0x80ccff; //light blue

    int tool_type = 0;

    void scaleImage();
    void redraw();

    // eraser related variables and functions
    QImage backup_img;// a region of this is pushed to undoStack
    bool mouse_pressed = false;
    QPoint mouse_pos;
    int min_x, min_y, max_x, max_y;
    QImage brush, brush_scaled;
    QTimer *timer;
    void onMousePress_Eraser(QPoint pos);
    void onMouseRelease_Eraser(QPoint pos);
    void onMouseMove_Eraser(QPoint pos);
    void eraseAt(int x, int y);
    void undo_Eraser();
    void redo_Eraser();
    QList<HistoryItem> undoStack_Eraser; // maximum 10 steps undo
    QList<HistoryItem> redoStack_Eraser;

    // Scissor related variables and functions
    GradMap *grad_map = 0;

    SeedMode seed_mode = NO_SEED;
    std::vector<QPoint> seeds;
    std::vector<QPoint> shortPath;
    std::vector<std::vector<QPoint>> fullPath;
    std::vector<QPoint> redoStack;

    void undo_iScissor();
    void redo_iScissor();
    void onMousePress_iScissor(QPoint pos);
    void onMouseMove_iScissor(QPoint pos);
    void drawSeedToCursorPath();
    void drawSeedToSeedPath();
    void drawFullPath();
    void calcShortPath(QPoint from, QPoint to);
    void checkPathClosed();
    void getMaskedImage(QPoint click_pos);

    void keyPressEvent(QKeyEvent *ev);
    void accept();
    void done(int);
public slots:
    void undo();
    void redo();
    void zoomIn();
    void zoomOut();
    void onToolClick(int);
    void setBgColor(int type, QRgb clr);
    void setEraserSize(int val);
    void updateEraserSize();
    void onMousePress(QPoint);
    void onMouseRelease(QPoint);
    void onMouseMove(QPoint);
};

class BgColorDialog : public QDialog
{
    Q_OBJECT
public:
    BgColorDialog(QWidget *parent);
    int bg_type = TRANSPERANT;
    QRgb bg_color = 0x80ccff;
public slots:
    void setBgType(int val);
signals:
    void bgColorSelected(int, QRgb);
};

#endif /* __PHOTOQUICK_ISCISSOR */
