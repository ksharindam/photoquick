#pragma once
/* Inpainting Algorithm to Heal damaged photo or erase object */

#include "canvas.h"
#include <QList>

class MaskedImage
{
public:
    uchar **mask;
    QImage image;
    int width, height;
    // member functions
    MaskedImage(QImage image);
    MaskedImage(int width, int height);
    void copyMaskFrom(uchar **mask);
    void copyMaskFrom(QImage mask);
    int getSample(int x, int y, int band);
    void setSample(int x, int y, int band, int value);
    int isMasked(int x, int y);
    void setMask(int x, int y, int value);
    //bool hasMasked();
    int containsMasked(int x, int y, int S);
    MaskedImage* copy();
    MaskedImage* downsample();
    MaskedImage* upscale(int newW,int newH);
    ~MaskedImage();
};

int distanceMaskedImage(MaskedImage *source,int xs,int ys, MaskedImage *target,int xt,int yt, int S);


class NNF
{
public:
    // image
    MaskedImage *input, *output;
    //  patch radius
    int S;
    // Nearest-Neighbor Field 1 pixel = { target_x, target_y, distance_scaled }
    int ***field;
    int fieldW, fieldH;
    // functions
    NNF(MaskedImage *input, MaskedImage *output, int patchsize);
    ~NNF();
    void randomize();
    void initializeNNF(NNF *nnf);
    void initializeNNF();
    void minimizeNNF(int pass);
    void minimizeLinkNNF(int x, int y, int dir);
    int distance(int x,int y, int xp,int yp);
};


class Inpaint
{
public:
    // patch radius
    int radius;
    // Nearest-Neighbor Fields
    NNF *nnf_TargetToSource;
    NNF *nnf_SourceToTarget;
    // Pyramid of downsampled initial images
    QList<MaskedImage*> pyramid;

    // functions
    Inpaint();
    QImage inpaint(QImage input, QImage mask, int radius);
    MaskedImage* ExpectationMaximization(int level);
    void ExpectationStep(NNF* nnf, int sourceToTarget, double** vote, MaskedImage* source, int upscale);
};

void weightedCopy(MaskedImage* src, int xs, int ys, double** vote, int xd,int yd, double w);
void MaximizationStep(MaskedImage* target, double** vote);


//*************** Inpainting GUI *******************

#include "ui_inpaint_dialog.h"
#include <QPainter>
#include <QMouseEvent>


class InpaintDialog : public QDialog, public Ui_InpaintDialog
{
    Q_OBJECT
public:
    QImage image; // original unchanged image
    QImage image_scaled;
    QImage mask;  // mask of same size as pixmap
    QPixmap main_pixmap; // scaled pixmap to draw over
    bool mouse_pressed = false;
    float scale = 1.0;
    int min_x, min_y, max_x, max_y;
    QPoint start_pos;
    bool draw_mask = true; // true = draw mask, false = erase mask
    QPen eraser_pen;
    QPen brush_pen;
    QPainter painter;
    PaintCanvas *canvas;
    QList<HistoryItem> undoStack; // maximum 10 steps undo
    QList<HistoryItem> redoStack;

    void setBrushSize(int size);
    void scaleBy(float factor);
    void drawMask(QPoint start, QPoint end);
    void eraseMask(QPoint start, QPoint end);
    void updateMaskedArea(QPoint start, QPoint end);
    void updateImageArea(int x, int y, QImage part);
    void initMask();

    InpaintDialog(QImage &img, QWidget *parent);
    void keyPressEvent(QKeyEvent *ev);
public slots:
    void changeDrawMode(bool checked);
    void changeBrushSize(int size);
    void undo();
    void redo();
    void zoomIn();
    void zoomOut();
    void inpaint();
    void onMousePress(QPoint pos);
    void onMouseRelease(QPoint pos);
    void onMouseMove(QPoint pos);
};
