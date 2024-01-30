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
    void setSelected(bool select);
    // Variables
    QImage photo;
signals:
    void clicked(Thumbnail*);
};

class GridCell
{
public:
    float x;
    float y;
    QImage *photo;
};


class GridView : public QLabel
{
    Q_OBJECT
public:
    GridView(QWidget *parent);
    void setup();
    void redraw();
    QImage cachedScaledImage(QImage *image);
    QImage finalImage();
    // event handling
    void mouseMoveEvent(QMouseEvent *ev);
    void mousePressEvent(QMouseEvent *ev);
    void mouseReleaseEvent(QMouseEvent *ev);
    void dragEnterEvent(QDragEnterEvent *ev);
    void dropEvent(QDropEvent *ev);

    // Properties
    float pageW, pageH;// in points
    float minMargin;
    float cellW, cellH;
    int dpi;
    bool narrow_spacing;
    bool add_border;
    // calculated properties
    int row_count, col_count;
    float marginX, marginY;// page margin in points
    float spacing;
    float px_factor;

    bool mouse_pressed = false;
    QPoint mouse_press_pos;
    QImage *curr_photo;
    QPixmap canvas_pixmap; // grid which is displayed on screen
    QPainter painter;
    std::vector<GridCell> cells;
    std::map<QImage*, QImage> cached_images;
    std::map<QImage*, bool> image_rotations;
signals:
    void photoDropped(QImage);
};

// The dialog to create the grid
class GridDialog : public QDialog, public Ui_GridDialog
{
    Q_OBJECT
public:
    GridDialog(QImage img, QWidget *parent);
    void accept();

    // Variables
    GridView *gridView;
    std::vector<Thumbnail*> thumbnails;
    QVBoxLayout *thumbnailLayout;
public slots:
    void setupGrid();
    void addPhoto();
    void addPhoto(QImage);
    void onPageSizeChange(QString page_size);
    void selectThumbnail(Thumbnail *thumbnail);
};

