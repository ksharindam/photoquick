#pragma once

#include "ui_photogrid_dialog.h"
#include "ui_gridsetup_dialog.h"
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

class PhotoCell
{
public:
    int x;
    int y;
    int w;
    int h;
    QImage photo;
    QImage *src_photo;
    bool intersects(QRect rect);
};

// The canvas on which collage is created and displayed.
class GridPaper : public QLabel
{
    Q_OBJECT
public:
    GridPaper(QWidget *parent);
    void redraw();
    void setupGrid();
    void calcSpacingsMargins();
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
    float scale, spacingX, spacingY, marginX, marginY;
    bool min_spacing;
    bool add_border;
    QPixmap canvas_pixmap; // grid which is displayed on screen
    QPainter painter;
    QList<PhotoCell> cells;
    QImage *photo;   // currently selected photo
    QImage photo_grid;
public slots:
    void setPhoto(Thumbnail *thumb);
    void toggleMinSpacing(bool ok);
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
    Q_OBJECT
public:
    float paperW, paperH;
    int unit;
    GridSetupDialog(QWidget *parent);
    void accept();
public slots:
    void onPaperSizeChange(int index);
};


