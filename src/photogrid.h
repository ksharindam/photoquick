#pragma once

#include "ui_photogrid_dialog.h"
#include "ui_gridsetup_dialog.h"
#include "ui_collage_dialog.h"
#include "ui_collagesetup_dialog.h"
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>

// Thumbnail class holds a photo
class Thumbnail : public QLabel
{
    Q_OBJECT
public:
    Thumbnail(QImage img, QWidget *parent);
    void mousePressEvent(QMouseEvent *ev);
    void select(bool selected);
    // Variables
    QImage photo;
signals:
    void clicked(Thumbnail*);
};

// ThumbnailGroup contains one or more thumbnails, it is used to select a thumbnail
class ThumbnailGroup : public QObject
{
    Q_OBJECT
public:
    ThumbnailGroup(QObject *parent) : QObject(parent) {};
    void append(Thumbnail *thumbnail);
    // Variables
    QList<Thumbnail *> thumbnails;
    Thumbnail *selected_thumb = 0;
public slots:
    void selectThumbnail(Thumbnail *thumbnail);
};

// The canvas on which collage is created and displayed.
class GridPaper : public QLabel
{
    Q_OBJECT
public:
    GridPaper(QWidget *parent);
    void setupGrid();
    void mouseMoveEvent(QMouseEvent *ev);
    void mousePressEvent(QMouseEvent *ev);
    void mouseReleaseEvent(QMouseEvent *ev);
    void dragEnterEvent(QDragEnterEvent *ev);
    void dropEvent(QDropEvent *ev);
    void createFinalGrid();
    // Variables
    bool mouse_pressed = false;
    QPoint click_pos;
    int paperW, paperH; // original paper size in pixels
    int W, H;          // original cell size in pixels
    int cols, rows, DPI;
    float scale, spacingX, spacingY;
    bool add_border;
    QPixmap canvas_pixmap; // grid which is displayed on screen
    QPainter painter;
    QList<QRect> boxes;
    QMap<int, QImage> image_dict;
    QImage photo, photo_grid;
public slots:
    void setPhoto(Thumbnail *thumb);
    void toggleBorder(bool ok);
signals:
    void addPhotoRequested(QImage);
};

// The dialog to create the grid
class GridDialog : public QDialog, public Ui_GridDialog
{
    Q_OBJECT
public:
    GridDialog(QImage img, QWidget *parent);
    void setup();
    void updateStatus();
    void accept();
    // Variables
    GridPaper *gridPaper;
    ThumbnailGroup *thumbnailGr;
    float paperW, paperH; // paper size in inch or cm
    float cellW, cellH; // grid cell size in centimeter
    int unit, DPI; // unit 0 = inch, 1 = cm
public slots:
    void configure();
    void addPhoto();
    void addPhoto(QImage);
    void showHelp();
};

class GridSetupDialog : public QDialog, public Ui_GridSetupDialog
{
public:
    GridSetupDialog(QWidget *parent): QDialog(parent) { setupUi(this); }
};


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
    bool isNull();
    int img_w, img_h; // the original img resolution
    int x, y;
    int w, h;           // the size on collage paper
    QPixmap pixmap;
    QString filename;
    bool border = false;
    int rotation = 0;
    bool contains(QPoint pos);
    bool overCorner(QPoint pos);
    bool jpgOnDisk();
    QImage image();
private:
    bool isValid_ = true;
    QImage image_;      // either image_ or filename is stored
};

class CollagePaper : public QLabel
{
    Q_OBJECT
public:
    CollagePaper(QWidget *parent, int w, int h, int pdf_w, int pdf_h, int dpi);

    QPixmap paper;          // the scaled pixmap on which images are drawn
    int W, H, dpi;
    float pdf_w, pdf_h;     // size in points
    QString background_filename;
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
    QImage getFinalCollage();
public slots:
    void addPhoto();
    void removePhoto();
    void copyPhoto();
    void rotatePhoto();
    void toggleBorder(); // enable or disable border
    void updateStatus();
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
    void reject();
public slots:
    void setupBackground();
    void showStatus(QString);
};

class CollageSetupDialog : public QDialog, public Ui_CollageSetupDialog
{
    Q_OBJECT
public:
    CollageSetupDialog(QWidget *parent);
    void accept();
    QString filename;
public slots:
    void selectFile();
    void toggleUsePageSize(const QString&);
};
