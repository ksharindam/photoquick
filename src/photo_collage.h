#pragma once

#include <QButtonGroup>// Qt5+
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QMessageBox>
#include <QPainter>
#include <QPen>
#include <QColorDialog>
#include <QSettings>
#include <QBuffer>
#include <QMimeData>
#include <QUrl>
#include <QMouseEvent>
#include <cmath>
#include "common.h"
#include "pdfwriter.h"
#include "ui_collage_dialog.h"

#ifndef __PHOTOQUICK_COLLAGE
#define __PHOTOQUICK_COLLAGE

// *********** Collage maker *****************
class CollageItem
{
public:
    CollageItem(QString filename);
    CollageItem(QImage img);
    CollageItem(const CollageItem* p){
        pixmap = p->pixmap; filename = p->filename;
        image_ = p->image_;
        img_w = p->img_w; img_h = p->img_h;
        x = p->x+2; y = p->y+2; w = p->w; h = p->h;
        border = p->border;
        rotation = p->rotation;
    };
    int x, y;
    int w, h;         // the item size on screen
    int img_w, img_h; // the original QImage size after rotation
    QPixmap pixmap;// downscaled pixmap of fixed size, pixmap size != item size
    QString filename;
    bool border = false;
    int rotation = 0;

    QImage image();         // original QImage after roation
    QImage originalImage();
    bool isNull();
    bool contains(QPoint pos);
    bool overCorner(QPoint pos);
private:
    bool isValid_ = true;
    QImage image_;      // either image_ or filename is stored
};

class CollagePaper : public QLabel
{
    Q_OBJECT
public:
    CollagePaper(QWidget *parent);

    QPixmap paper;   // the scaled pixmap on which images are drawn
    QImage bg_img;
    QColor bg_color;
    int scaled_w, scaled_h;
    float out_w, out_h; // output image size (point or pixel)
    int out_unit;       // output size unit
    int out_dpi;
    //QString background_filename;
    QPixmap drag_icon;
    //QString dir;            // directory for opening files
    QList<CollageItem*> collageItems;
    bool mouse_pressed = false;
    bool corner_clicked = false;
    QPoint clk_pos;
    void mousePressEvent(QMouseEvent *ev);
    void mouseMoveEvent(QMouseEvent *ev);
    void mouseReleaseEvent(QMouseEvent *ev);
    void dragEnterEvent(QDragEnterEvent *ev);
    void dropEvent(QDropEvent *ev);
    void draw();
    void setup();
    void clean();       // delete collage items
    void addItem(CollageItem *item);
    void updateSize();
    void updateBackground();
    bool useSelectedImageAsBackground();
    void updateStatus();
    QImage getFinalCollage();
public slots:
    void addPhoto();
    void removePhoto();
    void copyPhoto();
    void rotatePhoto();
    void toggleBorder(); // enable or disable border
    void savePdf();
signals:
    void statusChanged(QString);
};


class CollageDialog : public QDialog, public Ui_CollageDialog
{
    Q_OBJECT
public:
    CollageDialog(QWidget *parent);
    CollagePaper *collagePaper;
    QImage collage;
    void accept();
    void done(int result);
public slots:
    void onPageSizeChange(int index);
    void onPageOrientationChange();
    void updatePageSize(float out_w, float out_h, int out_unit);
    void onDpiChange(int val);
    void onBackgroundChange(QAbstractButton*);
};


class PageSizeDialog : public QDialog
{
    Q_OBJECT
public:
    float out_w, out_h;
    int out_unit;
    QDoubleSpinBox *widthSpin, *heightSpin;
    QComboBox *unitCombo;
    PageSizeDialog(QWidget *parent, float w, float h, int unit);
    void accept();
public slots:
    void onUnitChange(int);
};

#endif /* __PHOTOQUICK_COLLAGE */
