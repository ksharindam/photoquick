#pragma once

#include "ui_photogrid_dialog.h"
#include "ui_gridsetup_dialog.h"
#include "ui_collage_dialog.h"
#include "ui_collagesetup_dialog.h"
#include <QLabel>
#include <QMouseEvent>

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
    void clicked(QImage);
};

// ThumbnailGroup contains one or more thumbnails, it is used to select a thumbnail
class ThumbnailGroup : public QObject
{
    Q_OBJECT
public:
    ThumbnailGroup(QObject *parent);
    void append(Thumbnail *thumbnail);
    // Variables
    QList<Thumbnail *> thumbnails;
public slots:
    void selectThumbnail(QImage);
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
    void createFinalGrid();
    // Variables
    int DPI, paperW, paperH, W, H, cols, rows;
    float scale, spacingX, spacingY;
    bool add_border;
    QList<QRect> boxes;
    QMap<int, QImage> image_dict;
    QImage photo, photo_grid;
public slots:
    void setPhoto(QImage img);
    void toggleBorder(bool ok);
};

// The dialog to create the grid
class GridDialog : public QDialog, public Ui_GridDialog
{
    Q_OBJECT
public:
    GridDialog(QImage img, QWidget *parent);
    void accept();
    // Variables
    GridPaper *gridPaper;
    ThumbnailGroup *thumbnailGr;
public slots:
    void configure();
    void addPhoto();
    void showHelp();
};

class GridSetupDialog : public QDialog, public Ui_GridSetupDialog
{
public:
    GridSetupDialog(QWidget *parent);
    void accept();
    int W, H, DPI, rows, cols, paperW, paperH;
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
    void draw();
    void setup();
    void clean();       // delete collage items
    void addItem(CollageItem *item);
    QImage getCollage();
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
