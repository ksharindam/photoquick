#pragma once
/*
Image Label Object to display the image.
*/

#include <QLabel>
#include <QStatusBar>
#include <QMovie>
#include <QMouseEvent>
#include <QScrollArea>
#include <QScrollBar>


//This is the image widget responsible for displaying image
class Canvas : public QLabel
{
    Q_OBJECT
public:
    Canvas(QWidget *parent, QScrollArea *scrollArea);
    void setAnimation(QMovie *anim);
    void setImage(QImage img);
    void showScaled();
    void rotate(int degree, Qt::Axis axis=Qt::ZAxis);
    // Variables
    QImage image;
    bool animation;
    float scale;
    bool drag_to_scroll;    // if click and drag moves image
private:
    void mousePressEvent(QMouseEvent *ev);
    void mouseReleaseEvent(QMouseEvent *ev);
    void mouseMoveEvent(QMouseEvent *ev);
    // Variables
    bool mouse_pressed;
    int v_scrollbar_pos, h_scrollbar_pos;
    QPoint clk_global;
    QScrollBar *vScrollbar, *hScrollbar;
signals:
    void mousePressed(QPoint);
    void mouseReleased(QPoint);
    void mouseMoved(QPoint);
    void imageUpdated();
};

// the crop manager
class Crop : public QObject
{
    Q_OBJECT
public:
    Crop(Canvas *canvas, QStatusBar *statusbar);
    Canvas *canvas;
    QStatusBar *statusbar;
private:
    QPixmap pixmap;
    bool mouse_pressed;
    QPoint topleft, btmright;   // corner pos at the time of mouse click
    QPoint p1, p2;              // corner pos while mouse moves
    QPoint clk_pos;
    int clk_area;
    float scaleX, scaleY, crop_w, crop_h;
    bool lock_crop_ratio;
    QList<QWidget *> crop_widgets;
    void drawCropBox();
private slots:
    void onMousePress(QPoint pos);
    void onMouseRelease(QPoint pos);
    void onMouseMove(QPoint pos);
    void lockCropRatio(bool);
    void setCropWidth(double);
    void setCropHeight(double);
    void crop();
    void finish();
};

// perspective transform manager
class PerspectiveTransform : public QObject
{
    Q_OBJECT
public:
    PerspectiveTransform(Canvas *canvas, QStatusBar *statusbar);
    Canvas *canvas;
    QStatusBar *statusbar;
private:
    QPixmap pixmap;
    bool mouse_pressed;
    QPoint topleft, topright, btmleft, btmright, clk_pos, p1,p2,p3,p4;
    int clk_area;
    float scaleX, scaleY;
    QList<QWidget *> crop_widgets;
    void drawCropBox();
private slots:
    void onMousePress(QPoint pos);
    void onMouseRelease(QPoint pos);
    void onMouseMove(QPoint pos);
    void transform();
    void finish();
};


void calcArc(QPoint center, QPoint from, QPoint to, QPoint through,
                            float &start, float &span);
