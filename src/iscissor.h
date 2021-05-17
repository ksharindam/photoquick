#pragma once
/* Intelligent Scissor and Manual eraser for background removal */

#include "ui_iscissor_dialog.h"
#include "canvas.h"
#include <QPainter>
#include <QTimer>
#include <vector>

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
} ToolType;

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

class IScissorDialog : public QDialog, public Ui_IScissorDialog
{
    Q_OBJECT
public:
    QImage image; // original unchanged image
    QImage image_scaled;
    bool is_mask; // if the image is converted to mask
    float scale;
    QPainter painter;
    PaintCanvas *canvas;

    int bg_color_type = TRANSPERANT;
    QRgb bg_color = 0x80ccff; //light blue

    int tool_type = 0;

    void scaleImage();
    void redraw();

    // eraser related functions
    QImage image_tmp;
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

    IScissorDialog(QImage &img, QWidget *parent);
    void keyPressEvent(QKeyEvent *ev);
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
    void confirmAccept();
    void useAsMask();
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

