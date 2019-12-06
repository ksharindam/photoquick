/*
This file is a part of qmageview program, which is GPLv3 licensed
*/
#include "canvas.h"
#include <QDebug>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QSizePolicy>
#include <QTransform>
#include <QPainter>
#include <QRect>
#include <cmath>

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))


Canvas:: Canvas(QWidget *parent, QScrollArea *scrollArea) : QLabel(parent)
{
    vScrollbar = scrollArea->verticalScrollBar();
    hScrollbar = scrollArea->horizontalScrollBar();
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setMouseTracking(true);
    mouse_pressed = false;
    animation = false;
    drag_to_scroll = true;
    scale = 1.0;
}

void
Canvas:: setAnimation(QMovie *anim)
{
    scale = 1.0;
    if (animation)
        movie()->deleteLater();
    else
        image = QImage();
    animation = true;
    setMovie(anim);
    anim->start();
}

void
Canvas:: setImage(QImage img)
{
    if (animation) {
        animation = false;
        movie()->deleteLater();
    }
    image = img;
    showScaled();
}

void
Canvas:: showScaled()
{
    QPixmap pm = QPixmap::fromImage(image);
    if (scale != 1.0) {
        Qt::TransformationMode mode = floorf(scale) == ceilf(scale)? // integer scale
                                    Qt::FastTransformation : Qt::SmoothTransformation;
        pm = pm.scaledToHeight(scale*pm.height(), mode);
    }
    setPixmap(pm);
    emit imageUpdated();
}

void
Canvas:: rotate(int degree, Qt::Axis axis)
{
    QTransform transform;
    transform.rotate(degree, axis);
    image = image.transformed(transform);
    showScaled();
}

void
Canvas:: mousePressEvent(QMouseEvent *ev)
{
    clk_global = ev->globalPos();
    v_scrollbar_pos = vScrollbar->value();
    h_scrollbar_pos = hScrollbar->value();
    mouse_pressed = true;
    emit mousePressed(ev->pos());
}

void
Canvas:: mouseReleaseEvent(QMouseEvent *ev)
{
    mouse_pressed = false;
    emit mouseReleased(ev->pos());
}

void
Canvas:: mouseMoveEvent(QMouseEvent *ev)
{
    emit mouseMoved(ev->pos());
    if (not (mouse_pressed and drag_to_scroll)) return;
    // Handle click and drag to scroll
    vScrollbar->setValue(v_scrollbar_pos + clk_global.y() - ev->globalY());
    hScrollbar->setValue(h_scrollbar_pos + clk_global.x() - ev->globalX());
}


// ******************************************************************* |
//                         Crop Manager
// ------------------------------------------------------------------- |
Crop:: Crop(Canvas *canvas, QStatusBar *statusbar) : QObject(canvas),
                                canvas(canvas), statusbar(statusbar)
{
    mouse_pressed = false;
    canvas->drag_to_scroll = false;
    pixmap = canvas->pixmap()->copy();
    scaleX = float(pixmap.width())/canvas->image.width();
    scaleY = float(pixmap.height())/canvas->image.height();
    topleft = QPoint(0,0);
    btmright = QPoint(pixmap.width()-1, pixmap.height()-1);
    p1 = QPoint(topleft);   // temporary top-left point
    p2 = QPoint(btmright);
    crop_w = 3.5;
    crop_h = 4.5;
    lock_crop_ratio = false;
    // add buttons
    QCheckBox *lockratio = new QCheckBox("Lock Ratio  ", statusbar);
    statusbar->addPermanentWidget(lockratio);
    QLabel *labelWH = new QLabel("<b>W:H =</b>", statusbar);
    statusbar->addPermanentWidget(labelWH);
    labelWH->setEnabled(false);
    QDoubleSpinBox *spinWidth = new QDoubleSpinBox(statusbar);
    spinWidth->setRange(0.1, 9.9);
    spinWidth->setSingleStep(0.1);
    spinWidth->setDecimals(1);
    spinWidth->setMaximumWidth(44);
    spinWidth->setValue(3.5);
    spinWidth->setEnabled(false);
    statusbar->addPermanentWidget(spinWidth);
    QLabel *colon = new QLabel(":", statusbar);
    statusbar->addPermanentWidget(colon);
    QDoubleSpinBox *spinHeight = new QDoubleSpinBox(statusbar);
    spinHeight->setRange(0.1, 9.9);
    spinHeight->setSingleStep(0.1);
    spinHeight->setDecimals(1);
    spinHeight->setMaximumWidth(44);
    spinHeight->setValue(4.5);
    spinHeight->setEnabled(false);
    statusbar->addPermanentWidget(spinHeight);
    QPushButton *cropnowBtn = new QPushButton("Crop Now", statusbar);
    statusbar->addPermanentWidget(cropnowBtn);
    QPushButton *cropcancelBtn = new QPushButton("Cancel", statusbar);
    statusbar->addPermanentWidget(cropcancelBtn);
    connect(lockratio, SIGNAL(toggled(bool)), labelWH, SLOT(setEnabled(bool)));
    connect(lockratio, SIGNAL(toggled(bool)), spinWidth, SLOT(setEnabled(bool)));
    connect(lockratio, SIGNAL(toggled(bool)), spinHeight, SLOT(setEnabled(bool)));
    connect(lockratio, SIGNAL(toggled(bool)), this, SLOT(lockCropRatio(bool)));
    connect(spinWidth, SIGNAL(valueChanged(double)), this, SLOT(setCropWidth(double)));
    connect(spinHeight, SIGNAL(valueChanged(double)), this, SLOT(setCropHeight(double)));
    connect(cropnowBtn, SIGNAL(clicked()), this, SLOT(crop()));
    connect(cropcancelBtn, SIGNAL(clicked()), this, SLOT(finish()));
    crop_widgets << lockratio << labelWH << spinWidth << colon << spinHeight << cropnowBtn << cropcancelBtn;
    drawCropBox();
}

void
Crop:: onMousePress(QPoint pos)
{
    clk_pos = pos;
    mouse_pressed = true;
    // Determine which position is clicked
    if (QRect(topleft, QSize(60, 60)).contains(clk_pos))
        clk_area = 1;   // Topleft is clicked
    else if (QRect(btmright, QSize(-60, -60)).contains(clk_pos))
        clk_area = 2;   // bottom right corner clicked
    else if (QRect(topleft, btmright).contains(clk_pos)) // clicked inside cropbox
        clk_area = 3;   // inside cropbox
    else
        clk_area = 0;   // ouside cropbox
}

void
Crop:: onMouseRelease(QPoint /*pos*/)
{
    mouse_pressed = false;
    topleft = p1;
    btmright = p2;
}

void
Crop:: onMouseMove(QPoint pos)
{
    if (not mouse_pressed) return;
    QPoint moved = pos - clk_pos;
    QPoint last_pt = QPoint(pixmap.width()-1, pixmap.height()-1);
    float imgAspect = float(canvas->image.width())/canvas->image.height();
    float boxAspect = crop_w/crop_h;
    switch (clk_area) {
    case 1 : { // Top left corner is clicked
        QPoint new_p1 = topleft + moved;
        p1 = QPoint(MAX(0, new_p1.x()), MAX(0, new_p1.y()));
        if (lock_crop_ratio) {
            if (imgAspect>boxAspect) p1.setX(round(p2.x() - (p2.y()-p1.y()+1)*boxAspect -1));
            else p1.setY(round(p2.y() - (p2.x()-p1.x()+1)/boxAspect -1));
        }
        break;
    }
    case 2 : { // Bottom right corner is clicked
        QPoint new_p2 = btmright + moved;
        p2 = QPoint(MIN(last_pt.x(), new_p2.x()), MIN(last_pt.y(), new_p2.y()));
        if (lock_crop_ratio) {
            if (imgAspect>boxAspect) p2.setX(round(p1.x() + (p2.y()-p1.y()+1)*boxAspect -1));
            else p2.setY(round(p1.y() + (p2.x()-p1.x()+1)/boxAspect -1));
        }
        break;
    }
    case 3 : { // clicked inside cropbox but none of the corner selected.
        int min_dx, max_dx, min_dy, max_dy, dx, dy;
        min_dx = -topleft.x();
        max_dx = last_pt.x()-btmright.x();
        min_dy = -topleft.y();
        max_dy = last_pt.y()-btmright.y();
        dx = (moved.x() < 0) ? MAX(moved.x(), min_dx) : MIN(moved.x(), max_dx);
        dy = (moved.y() < 0) ? MAX(moved.y(), min_dy) : MIN(moved.y(), max_dy);
        p1 = topleft + QPoint(dx, dy);
        p2 = btmright + QPoint(dx, dy);
        break;
    }
    }
    drawCropBox();
}

void
Crop:: drawCropBox()
{
    QPixmap pm = pixmap.copy();
    QPixmap pm_box = pm.copy(p1.x(), p1.y(), p2.x()-p1.x(), p2.y()-p1.y());
    QPainter painter(&pm);
    painter.fillRect(0,0, pm.width(), pm.height(), QColor(127,127,127,127));
    painter.drawPixmap(p1.x(), p1.y(), pm_box);
    painter.drawRect(p1.x(), p1.y(), p2.x()-p1.x(), p2.y()-p1.y());
    painter.drawRect(p1.x(), p1.y(), 59, 59);
    painter.drawRect(p2.x(), p2.y(), -59, -59);
    painter.setPen(Qt::white);
    painter.drawRect(p1.x()+1, p1.y()+1, 57, 57);
    painter.drawRect(p2.x()-1, p2.y()-1, -57, -57);
    painter.end();
    canvas->setPixmap(pm);
    int width = round((p2.x() - p1.x() + 1)/scaleX);
    int height = round((p2.y() - p1.y() + 1)/scaleY);
    QString text = "Resolution : %1x%2";
    statusbar->showMessage(text.arg(width).arg(height));
}

void
Crop:: crop()
{
    int w = round((btmright.x()-topleft.x()+1)/scaleX);
    int h = round((btmright.y()-topleft.y()+1)/scaleY);
    QImage img = canvas->image.copy(round(topleft.x()/scaleX), round(topleft.y()/scaleY), w, h);
    canvas->image = img;
    finish();
}

void
Crop:: finish()
{
    canvas->showScaled();
    canvas->drag_to_scroll = true;
    // remove buttons
    while (not crop_widgets.isEmpty()) {
        QWidget *widget = crop_widgets.takeLast();
        statusbar->removeWidget(widget);
        widget->deleteLater();
    }
    this->deleteLater();
}

void
Crop:: lockCropRatio(bool checked)
{
    lock_crop_ratio = checked;
}

void
Crop:: setCropWidth(double value)
{
    crop_w = float(value);
}

void
Crop:: setCropHeight(double value)
{
    crop_h = float(value);
}


// ******************************************************************* |
//                         Perspective Transform
// ------------------------------------------------------------------- |
PerspectiveTransform::
PerspectiveTransform(Canvas *canvas, QStatusBar *statusbar) : QObject(canvas),
                                canvas(canvas), statusbar(statusbar)
{
    mouse_pressed = false;
    canvas->drag_to_scroll = false;
    pixmap = canvas->pixmap()->copy();
    scaleX = float(pixmap.width())/canvas->image.width();
    scaleY = float(pixmap.height())/canvas->image.height();
    p1 = topleft = QPoint(0,0);
    p2 = topright = QPoint(pixmap.width()-1, 0);
    p3 = btmleft = QPoint(0, pixmap.height()-1);
    p4 = btmright = QPoint(pixmap.width()-1, pixmap.height()-1);
    // add buttons
    QPushButton *cropnowBtn = new QPushButton("Crop Now", statusbar);
    statusbar->addPermanentWidget(cropnowBtn);
    QPushButton *cropcancelBtn = new QPushButton("Cancel", statusbar);
    statusbar->addPermanentWidget(cropcancelBtn);
    connect(cropnowBtn, SIGNAL(clicked()), this, SLOT(transform()));
    connect(cropcancelBtn, SIGNAL(clicked()), this, SLOT(finish()));
    crop_widgets << cropnowBtn << cropcancelBtn;
    drawCropBox();
}

void
PerspectiveTransform:: onMousePress(QPoint pos)
{
    clk_pos = pos;
    mouse_pressed = true;
    // Determine which position is clicked
    if (QRect(topleft, QSize(60, 60)).contains(clk_pos))
        clk_area = 1;   // Topleft is clicked
    else if (QRect(topright, QSize(-60, 60)).contains(clk_pos))
        clk_area = 2;   // Topright is clicked
    else if (QRect(btmleft, QSize(60, -60)).contains(clk_pos))
        clk_area = 3;   // Bottomleft is clicked
    else if (QRect(btmright, QSize(-60, -60)).contains(clk_pos))
        clk_area = 4;   // bottom right corner clicked
    else
        clk_area = 0;
}

void
PerspectiveTransform:: onMouseRelease(QPoint /*pos*/)
{
    mouse_pressed = false;
    topleft = p1;
    topright = p2;
    btmleft = p3;
    btmright = p4;
}

void
PerspectiveTransform:: onMouseMove(QPoint pos)
{
    if (not mouse_pressed) return;
    QPoint moved = pos - clk_pos;
    QPoint last_pt = QPoint(pixmap.width()-1, pixmap.height()-1);
    QPoint new_pt;
    switch (clk_area) {
    case 1 : { // Top left corner is clicked
        new_pt = topleft + moved;
        p1 = QPoint(MAX(0, new_pt.x()), MAX(0, new_pt.y()));
        break;
    }
    case 2 : { // Top right corner is clicked
        new_pt = topright + moved;
        p2 = QPoint(MIN(last_pt.x(), new_pt.x()), MAX(0, new_pt.y()));
        break;
    }
    case 3 : { // Bottom left corner is clicked
        new_pt = btmleft + moved;
        p3 = QPoint(MAX(0, new_pt.x()), MIN(last_pt.y(), new_pt.y()));
        break;
    }
    case 4 : { // Bottom right corner is clicked
        QPoint new_pt = btmright + moved;
        p4 = QPoint(MIN(last_pt.x(), new_pt.x()), MIN(last_pt.y(), new_pt.y()));
        break;
    }
    default:
        break;
    }
    drawCropBox();
}

void
PerspectiveTransform:: drawCropBox()
{
    QPixmap pm = pixmap.copy();
    QPainter painter(&pm);
    QPolygonF polygon;
    polygon << p1<< p2<< p4<< p3;
    painter.drawPolygon(polygon);
    float start, span;
    calcArc(p1, p2, p3, p4, start, span);
    painter.drawArc(p1.x()-30, p1.y()-30, 60,60, 16*start, 16*span);
    calcArc(p2, p1, p4, p3, start, span);
    painter.drawArc(p2.x()-30, p2.y()-30, 60,60, 16*start, 16*span);
    calcArc(p3, p4, p1, p2, start, span);
    painter.drawArc(p3.x()-30, p3.y()-30, 60,60, 16*start, 16*span);
    calcArc(p4, p2, p3, p1, start, span);
    painter.drawArc(p4.x()-30, p4.y()-30, 60,60, 16*start, 16*span);
    painter.setPen(Qt::white);
    polygon.clear();
    polygon<< p1+QPoint(1,1)<< p2+QPoint(-1,1)<< p4+QPoint(-1,-1)<< p3+QPoint(1,-1);
    painter.drawPolygon(polygon);
    painter.end();
    canvas->setPixmap(pm);
}

void
PerspectiveTransform:: transform()
{
    p1 = QPoint(p1.x()/scaleX, p1.y()/scaleY);
    p2 = QPoint(p2.x()/scaleX, p2.y()/scaleY);
    p3 = QPoint(p3.x()/scaleX, p3.y()/scaleY);
    p4 = QPoint(p4.x()/scaleX, p4.y()/scaleY);
    int max_w = MAX(p2.x()-p1.x(), p4.x()-p3.x());
    int max_h = MAX(p3.y()-p1.y(), p4.y()-p2.y());
    QPolygonF mapFrom;
    mapFrom << p1<< p2<< p3<< p4;
    QPolygonF mapTo;
    mapTo << QPointF(0,0)<< QPointF(max_w,0)<< QPointF(0,max_h)<< QPointF(max_w,max_h);
    QTransform tfm;
    QTransform::quadToQuad(mapFrom, mapTo, tfm);
    QImage img = canvas->image.transformed(tfm, Qt::SmoothTransformation);
    QTransform trueMtx = QImage::trueMatrix(tfm,canvas->image.width(),canvas->image.height());
    topleft = trueMtx.map(p1);
    btmright = trueMtx.map(p4);
    canvas->image = img.copy(QRect(topleft, btmright));
    finish();
}

void
PerspectiveTransform:: finish()
{
    canvas->showScaled();
    canvas->drag_to_scroll = true;
    // remove buttons
    while (not crop_widgets.isEmpty()) {
        QWidget *widget = crop_widgets.takeLast();
        statusbar->removeWidget(widget);
        widget->deleteLater();
    }
    this->deleteLater();
}

// arc is drawn from p1 to p2 through p3
// p3 is at diagonal corner of p
// angle is  drawn counter clock-wise, and direction of y axis is
// upward, while direction of image y axis is downward
void calcArc(QPoint p/*center*/, QPoint p1, QPoint p2, QPoint p3,
                                    float &start, float &span)
{
    float x, ang1, ang2, ang3;
    x = (p.x()==0)? p.x()+1.0e-7: p.x();    // avoid zero division error
    ang1 = atan((p.y()-p1.y())/(p1.x()-x))*180/3.14159265;
    ang1 = (x>p1.x()) ? ang1+180 : ang1;
    ang1 = ang1<0 ? ang1+360: ang1;
    ang2 = atan((p.y()-p2.y())/(p2.x()-x))*180/3.14159265;
    ang2 = (x>p2.x()) ? ang2+180 : ang2;
    ang2 = ang2<0 ? ang2+360: ang2;
    ang3 = atan((p.y()-p3.y())/(p3.x()-x))*180/3.14159265;
    ang3 = (x>p3.x()) ? ang3+180 : ang3;
    ang3 = ang3<0 ? ang3+360: ang3;
    if (ang1 > ang2) {
        float tmp = ang1;
        ang1 = ang2;
        ang2 = tmp;
    }
    if (ang1<ang3 and ang3<ang2) {
        start = ang1;
        span = ang2-ang1;
    }
    else {
        start = ang2;
        span = 360 - (ang2-ang1);
    }
}
