#include "photogrid.h"
#include "exif.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QImageReader>
#include <QPainter>
#include <QPen>

GridDialog:: GridDialog(QPixmap pixmap, QWidget *parent) : QDialog(parent)
{
    setupUi(this);
    resize(1020, 640);
    QHBoxLayout *layout = new QHBoxLayout(scrollAreaWidgetContents);
    layout->setContentsMargins(0, 0, 0, 0);
    gridPaper = new GridPaper(this);
    layout->addWidget(gridPaper);
    thumbnailGr = new ThumbnailGroup(this);
    Thumbnail *thumbnail = new Thumbnail(pixmap, frame);
    verticalLayout->addWidget(thumbnail);
    thumbnail->select(true);
    QObject::connect(thumbnail, SIGNAL(clicked(QPixmap)), gridPaper, SLOT(setPhoto(QPixmap)));
    thumbnailGr->append(thumbnail);
    QObject::connect(addPhotoBtn, SIGNAL(clicked()), this, SLOT(addPhoto()));
    QObject::connect(helpBtn, SIGNAL(clicked()), this, SLOT(showHelp()));
    gridPaper->photo = pixmap;
}

void
GridDialog:: addPhoto()
{
    QString filepath = QFileDialog::getOpenFileName(this, "Open Image");           
    if (filepath.isEmpty()) return;
    QImageReader image_reader(filepath);
    QPixmap pm = QPixmap::fromImageReader(&image_reader);
    if (not pm.isNull()) {
        Thumbnail *thumbnail = new Thumbnail(pm, frame);
        verticalLayout->addWidget(thumbnail);
        QObject::connect(thumbnail, SIGNAL(clicked(QPixmap)), gridPaper, SLOT(setPhoto(QPixmap)));
        thumbnailGr->append(thumbnail);
    }
}

void
GridDialog:: accept()
{
    // Create final grid when ok is clicked
    gridPaper->createFinalGrid();
    QDialog::accept();
}

void
GridDialog:: showHelp()
{
    QString helptext = "Click on a image thumbnail to select an image to drop. Then click on the blank boxes to drop the selected photo.\n\n"
                       "If you want to create grid with more different photos then load photo by clicking Add Photo button.\n\n"
                       "You can change the photo of a box by selecting another image and clicking over the box.";
    QMessageBox::about(this, "How to Create Grid", helptext);
}


Thumbnail:: Thumbnail(QPixmap pixmap, QWidget *parent) : QLabel(parent)
{
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    photo = pixmap;
    setPixmap(pixmap.scaledToWidth(100));
}

void
Thumbnail:: mousePressEvent(QMouseEvent *)
{
    emit clicked(photo);
}

void
Thumbnail:: select(bool selected)
{
    if (selected) {
        QPixmap pm = photo.scaledToWidth(100);
        QPainter painter(&pm);
        QPen pen(Qt::blue);
        pen.setWidth(4);
        painter.setPen(pen);
        painter.drawRect(2, 2 , 100-4, pm.height()-4);
        painter.end();
        setPixmap(pm);
    }
    else
        setPixmap(photo.scaledToWidth(100));
}


ThumbnailGroup:: ThumbnailGroup(QObject *parent) : QObject(parent)
{
}

void
ThumbnailGroup:: append(Thumbnail *thumbnail)
{
    thumbnails << thumbnail;
    QObject::connect(thumbnail, SIGNAL(clicked(QPixmap)), this, SLOT(selectThumbnail(QPixmap)));
}

void
ThumbnailGroup:: selectThumbnail(QPixmap)
{
    for (int i=0;i<thumbnails.count();++i) {
        thumbnails[i]->select(false);
    }
    qobject_cast<Thumbnail*>(sender())->select(true);
}


GridPaper:: GridPaper(QWidget *parent) : QLabel(parent)
{
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setMouseTracking(true);
    //scale = 1.0;
    setupGrid();
}

void
GridPaper:: setupGrid()
{
    paperW = 1800;
    paperH = 1200;
    W = 413;
    H = 531;
    cols = 4;
    rows = 2;           // total no. of columns and rows
    spacingX = (paperW-cols*W)/(cols+1);
    spacingY = (paperH-rows*H)/(rows+1);
    // Setup Foreground Grid
    int w = W/2;
    int h = H/2;
    int spacing_x = spacingX/2;
    int spacing_y = spacingY/2;
    for (int i=0; i<cols*rows; ++i) {
        int row = i/cols;            // Position of the box as row & col
        int col = i%cols;
        QRect box = QRect(spacing_x+col*(spacing_x+w), spacing_y+row*(spacing_y+h), w-1, h-1);
        boxes << box;
    }
    QPixmap fg = QPixmap(paperW/2, paperH/2);
    fg.fill();
    QPainter painter(&fg);
    foreach (QRect box, boxes)
        painter.drawRect(box);
    painter.end();
    setPixmap(fg);
}

void
GridPaper:: setPhoto(QPixmap pixmap)
{
    photo = pixmap;
}

void
GridPaper:: mouseMoveEvent(QMouseEvent *ev)
{
    // Change cursor whenever cursor comes over a box
    foreach (QRect box, boxes) {
        if (box.contains(ev->pos())) {
            setCursor(Qt::PointingHandCursor);
            return;
        }
    }
    setCursor(Qt::ArrowCursor);
}

void
GridPaper:: mousePressEvent(QMouseEvent *ev)
{
    QPixmap blank_pm(W/2, H/2);
    blank_pm.fill();
    foreach (QRect box, boxes) {
        if (box.contains(ev->pos())) {
            QPoint topleft = box.topLeft();
            //print(topleft.x(), topleft.y())
            QPixmap pm = photo.scaled(W/2, H/2, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            QPixmap bg = *(pixmap());
            QPainter painter(&bg);
            painter.drawPixmap(topleft, blank_pm); // Erase older image by placing blank image over it
            painter.drawPixmap(topleft, pm);
            painter.end();
            setPixmap(bg);
            pixmap_dict[boxes.indexOf(box)] = photo;
            break;
        }
    }
}

void
GridPaper:: createFinalGrid()
{
    photo_grid = QPixmap(paperW, paperH);
    photo_grid.fill();
    QPainter painter(&photo_grid);
    foreach (int index, pixmap_dict.keys()) {
        int row = index/cols;
        int col = index%cols;
        QPoint topleft = QPoint(spacingX+col*(spacingX+W), spacingY+row*(spacingY+H));
        QPixmap pm = pixmap_dict.value(index).scaled(W, H, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        painter.drawPixmap(topleft, pm);
    }
    painter.end();
}

// Static functions
QPixmap loadImage(QString filename)
{
    QPixmap pm(filename);
    if (pm.isNull()) return pm;
    // Get image orientation
    int orientation = 0;
    QFile file(filename);
    file.open(QIODevice::ReadOnly);
    Exif *exif = new Exif();
    exif->getExifOrientation(file, &orientation);
    file.close();
    delete exif;
    // rotate if required
    if (orientation!=0) {
        QTransform transform;
        if (orientation==6)
            return pm.transformed(transform.rotate(90));
        else if (orientation==3)
            return pm.transformed(transform.rotate(180));
        else if (orientation==8)
            return pm.transformed(transform.rotate(270));
    }
    return pm;
}

