/* This file is a part of photoquick program, which is GPLv3 licensed */

#include "common.h"
#include "photogrid.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QPen>
#include <QDesktopWidget>
#include <QSettings>
//#include <QBuffer>
#include <QMimeData>
#include <QUrl>
#include <cmath>

#define UNIT_NAMES   {"in", "cm"}
#define UNIT_FACTORS {1, 1/2.54 }

GridDialog:: GridDialog(QImage img, QWidget *parent) : QDialog(parent)
{
    setupUi(this);
    QHBoxLayout *layout = new QHBoxLayout(scrollAreaWidgetContents);
    layout->setContentsMargins(0, 0, 0, 0);
    gridPaper = new GridPaper(this);
    layout->addWidget(gridPaper);
    thumbnailGr = new ThumbnailGroup(this);

    connect(configureBtn, SIGNAL(clicked()), this, SLOT(configure()));
    connect(addPhotoBtn, SIGNAL(clicked()), this, SLOT(addPhoto()));
    connect(checkMinSpacing, SIGNAL(clicked(bool)), gridPaper, SLOT(toggleMinSpacing(bool)));
    connect(checkAddBorder, SIGNAL(clicked(bool)), gridPaper, SLOT(toggleBorder(bool)));
    connect(helpBtn, SIGNAL(clicked()), this, SLOT(showHelp()));
    connect(gridPaper, SIGNAL(addPhotoRequested(QImage)), this, SLOT(addPhoto(QImage)));

    addPhoto(img);
    QSettings settings(this);
    settings.beginGroup("PhotoGrid");
    DPI = settings.value("DPI", 300).toInt();
    cellW = settings.value("CellW", 2.9).toFloat();
    cellH = settings.value("CellH", 3.8).toFloat();
    paperW = settings.value("PaperW", 4.0).toFloat();
    paperH = settings.value("PaperH", 6.0).toFloat();
    unit = settings.value("PaperUnit", 0).toInt();
    settings.endGroup();
    if (paperW<1.0 or paperW>30.0 or paperH<1.0 or paperH>30.0 or unit>1) {
        QMessageBox::critical(parent, "Error loading settings",
                "Error loading paper size settings.\nPlease Configure photo grid.");
        return;
    }
    gridPaper->min_spacing = true;
    setup();
}

void
GridDialog:: setup()
{
    QList<float> unit_factors = UNIT_FACTORS;
    float unit_factor = unit_factors[unit];
    gridPaper->paperW = paperW * unit_factor * DPI;
    gridPaper->paperH = paperH * unit_factor * DPI;
    gridPaper->W = cellW/2.54 * DPI;
    gridPaper->H = cellH/2.54 * DPI;
    gridPaper->DPI = DPI;
    gridPaper->setupGrid();
    updateStatus();
}

void
GridDialog:: configure()
{
    GridSetupDialog *dialog = new GridSetupDialog(this);
    dialog->spinPhotoWidth->setValue(cellW);
    dialog->spinPhotoHeight->setValue(cellH);
    dialog->spinDPI->setValue(DPI);
    if ( dialog->exec() != QDialog::Accepted )
        return;
    cellW = dialog->spinPhotoWidth->value();
    cellH = dialog->spinPhotoHeight->value();
    paperW = dialog->paperW;
    paperH = dialog->paperH;
    unit = dialog->unit;
    DPI = dialog->spinDPI->value();
    setup();
}

void
GridDialog:: addPhoto()
{
    QString filefilter = "JPEG Images (*.jpg *jpeg);;PNG Images (*.png);;All Files (*)";
    QString filepath = QFileDialog::getOpenFileName(this, "Open Image", "", filefilter);
    if (filepath.isEmpty()) return;
    QImage img = loadImage(filepath);
    addPhoto(img);
}

void
GridDialog:: addPhoto(QImage img)
{
    if (img.isNull())
        return;
    Thumbnail *thumbnail = new Thumbnail(img, frame);
    verticalLayout->insertWidget(verticalLayout->count()-1, thumbnail);
    connect(thumbnail, SIGNAL(clicked(Thumbnail*)), gridPaper, SLOT(setPhoto(Thumbnail*)));
    thumbnailGr->append(thumbnail);
    thumbnailGr->selectThumbnail(thumbnail);
    gridPaper->photo = &thumbnail->photo;
}

void
GridDialog:: accept()
{
    // Create final grid when ok is clicked
    gridPaper->createFinalGrid();
    QDialog::accept();
}

void
GridDialog:: updateStatus()
{
    QStringList unit_name = UNIT_NAMES;
    QString msg("Photo : %1x%2 cm,   Paper : %3x%4 %5,   %6 dpi");
    msg = msg.arg(cellW).arg(cellH).arg(paperW).arg(paperH).arg(unit_name[unit]).arg(DPI);
    statusbar->setText(msg);
}

void
GridDialog:: showHelp()
{
    QString helptext =
        "To create Photo Grid :\n"
        "1. Click on a image thumbnail to select an image.\n"
        "2. Click on each cells to fill them with selected photo. Click and drag to fill "
            "multiple cells at once.\n"
        "3. Finally, Click Ok button to get the photo grid.\n\n"
        "Multiple Photos : You can load multiple photos by clicking Add Photo button or"
        " drag and drop from file manager.\n\n"
        "Configure : Click Configure button to change page size or photo size.";
    QMessageBox::about(this, "Photo Grid for Printing", helptext);
}

typedef struct {
    double w;
    double h;
    int unit;
} PaperSize;


GridSetupDialog:: GridSetupDialog(QWidget *parent) : QDialog(parent)
{
    setupUi(this);
    connect(paperSizeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(onPaperSizeChange(int)));
    QSettings settings(this);
    settings.beginGroup("PhotoGrid");
    int paper_size_index = settings.value("PaperSizeIndex", 1).toInt();
    paperW = settings.value("CustomPaperW", 4.0).toFloat();
    paperH = settings.value("CustomPaperH", 6.0).toFloat();
    unit = settings.value("CustomPaperUnit", 0).toInt();
    settings.endGroup();
    paperSizeCombo->setCurrentIndex(paper_size_index);
    spinPaperWidth->setValue(paperW);
    spinPaperHeight->setValue(paperH);
    paperSizeUnit->setCurrentIndex(unit);
}

void
GridSetupDialog:: onPaperSizeChange(int index)
{
    bool hide = paperSizeCombo->itemText(index)!="Custom";
    spinPaperWidth->setHidden(hide);
    spinPaperHeight->setHidden(hide);
    xLabel->setHidden(hide);
    paperSizeUnit->setHidden(hide);
}

void
GridSetupDialog:: accept()
{
    QList<PaperSize> sizes = {/*A4*/{21.0, 29.7, 1}, /*4R*/{4.0, 6.0, 0}, /*2L*/{5.0, 7.0, 0},
        {spinPaperWidth->value(), spinPaperHeight->value(), paperSizeUnit->currentIndex()}};
    PaperSize size = sizes[paperSizeCombo->currentIndex()];
    paperW = (float)size.w;
    paperH = (float)size.h;
    unit = size.unit;
    // save setup
    QSettings settings(this);
    settings.beginGroup("PhotoGrid");
    settings.setValue("PaperSizeIndex", paperSizeCombo->currentIndex());
    if (paperSizeCombo->currentText()=="Custom"){
        settings.setValue("CustomPaperW", paperW);
        settings.setValue("CustomPaperH", paperH);
        settings.setValue("CustomPaperUnit", unit);
    }
    settings.setValue("PaperW", paperW);
    settings.setValue("PaperH", paperH);
    settings.setValue("PaperUnit", unit);
    settings.setValue("DPI", spinDPI->value());
    settings.setValue("CellW", spinPhotoWidth->value());
    settings.setValue("CellH", spinPhotoHeight->value());
    settings.endGroup();
    QDialog::accept();
}


Thumbnail:: Thumbnail(QImage img, QWidget *parent) : QLabel(parent)
{
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    photo = img;
    setPixmap(QPixmap::fromImage(img.scaledToWidth(100)));
}

void
Thumbnail:: mousePressEvent(QMouseEvent *)
{
    emit clicked(this);
}

// this should be only called by thumbnail group
void
Thumbnail:: select(bool selected)
{
    QImage img = photo.scaledToWidth(100);
    if (selected) {
        QPainter painter(&img);
        QPen pen(Qt::blue);
        pen.setWidth(4);
        painter.setPen(pen);
        painter.drawRect(2, 2 , 100-4, img.height()-4);
        painter.end();
    }
    setPixmap(QPixmap::fromImage(img));
}


// ThumbnailGroup contains one or more thumbnails, it is used to select a thumbnail
void
ThumbnailGroup:: append(Thumbnail *thumbnail)
{
    thumbnails << thumbnail;
    connect(thumbnail, SIGNAL(clicked(Thumbnail*)), this, SLOT(selectThumbnail(Thumbnail*)));
}

void
ThumbnailGroup:: selectThumbnail(Thumbnail *thumb)
{
    if (selected_thumb)
        selected_thumb->select(false);
    thumb->select(true);
    selected_thumb = thumb;
}

// calculates number of rows and cols of items in grid paper
void calcRowsCols(int &paperW, int &paperH, int W, int H, int &rows, int &cols)
{
    rows = paperH/H;
    cols = paperW/W;
    int rows2 = paperW/H;
    int cols2 = paperH/W;
    if (rows*cols < rows2*cols2) {
        int tmp = paperW;
        paperW = paperH;
        paperH = tmp;
        rows = rows2;
        cols = cols2;
    }
}


bool
PhotoCell:: intersects(QRect rect)
{
    return QRect(x,y,w,h).intersects(rect);
}

// GridPaper class methods
GridPaper:: GridPaper(QWidget *parent) : QLabel(parent)
{
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setMouseTracking(true);
    setAcceptDrops(true);
    add_border = true;
}

void
GridPaper:: calcSpacingsMargins()
{
    spacingX = (paperW-cols*W)/float(cols+1);
    spacingY = (paperH-rows*H)/float(rows+1);
    // half milimeter uniform spacing
    if (min_spacing){
        float spacing = 0.5/25.4*DPI;
        spacingX = spacing < spacingX ? spacing : spacingX;
        spacingY = spacing < spacingY ? spacing : spacingY;
    }
    marginX = (paperW-cols*W-(cols-1)*spacingX)/2;
    marginY = (paperH-rows*H-(rows-1)*spacingY)/2;
}

void
GridPaper:: setupGrid()
{
    calcRowsCols(paperW, paperH, W, H, rows, cols);
    calcSpacingsMargins();
    // Setup Foreground Grid
    float screenDPI = QApplication::desktop()->logicalDpiX();
    scale = screenDPI/DPI;
    int w = W*scale;
    int h = H*scale;

    float spacing_x = spacingX * scale;
    float spacing_y = spacingY * scale;
    float margin_x =  marginX * scale;
    float margin_y =  marginY * scale;

    // may have extra cells from prev configuration, remove them
    while (cells.count()>cols*rows)
        cells.pop_back();
    // set pos and size of each cells
    for (int i=0; i<cols*rows; ++i) {
        if (i==cells.count())
            cells.append(PhotoCell());
        int row = i/cols;            // Position of the box as row & col
        int col = i%cols;
        cells[i].x = margin_x + col*(spacing_x+w);
        cells[i].y = margin_y + row*(spacing_y+h);
        cells[i].w = w;
        cells[i].h = h;
        if (not cells[i].photo.isNull())// from prev setup
            cells[i].photo = cells[i].src_photo->scaled(w, h, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
    redraw();
}

void
GridPaper:: redraw()
{
    // create canvas pixmap for displaying on screen
    canvas_pixmap = QPixmap(paperW*scale, paperH*scale);
    canvas_pixmap.fill();
    // draw empty cells
    painter.begin(&canvas_pixmap);
    for (PhotoCell &cell : cells){
        if (not cell.photo.isNull()) {
            // center align the image in the box
            QImage img = cell.photo;
            QPoint offset = QPoint(cell.w-img.width(), cell.h-img.height())/2;
            QPoint img_pos = QPoint(cell.x, cell.y)+offset;
            painter.drawImage(img_pos, img);
            if (add_border)
                painter.drawRect(img_pos.x(), img_pos.y(), img.width()-1, img.height()-1);
        }
        else {
            painter.drawRect(cell.x, cell.y, cell.w-1, cell.h-1);
        }
    }
    painter.end();
    setPixmap(canvas_pixmap);
}

void
GridPaper:: toggleMinSpacing(bool enable)
{
    min_spacing = enable;
    setupGrid();
}

void
GridPaper:: toggleBorder(bool enable)
{
    add_border = enable;
    redraw();
}

void
GridPaper:: mousePressEvent(QMouseEvent *ev)
{
    click_pos = ev->pos();
    mouse_pressed = true;
}

void
GridPaper:: mouseMoveEvent(QMouseEvent *ev)
{
    if (not mouse_pressed)
        return;
    QRect rect(click_pos, ev->pos());
    if (not (abs(rect.width())>2 and abs(rect.height())>2)) return;
    QPixmap pm = canvas_pixmap.copy();// copy for temporary drawing
    painter.begin(&pm);
    painter.drawRect(rect);
    painter.end();
    this->setPixmap(pm);
}

void
GridPaper:: mouseReleaseEvent(QMouseEvent *ev)
{
    mouse_pressed = false;
    QRect rect(click_pos, ev->pos());
    // fill cells with images if the rect intersects
    for (PhotoCell &cell : cells) {
        if (not cell.intersects(rect))
            continue;
        cell.src_photo = photo;
        cell.photo = photo->scaled(cell.w, cell.h, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
    redraw();
}

void
GridPaper:: createFinalGrid()
{
    if (min_spacing) {
        int new_W=0, new_H=0;
        for (PhotoCell &cell : cells) {
            if (cell.photo.isNull())
                continue;
            int w, h;
            fitToSize(cell.src_photo->width(), cell.src_photo->height(), W, H, w, h);
            new_W = MAX(w, new_W);
            new_H = MAX(h, new_H);
        }
        W = new_W;
        H = new_H;
        calcSpacingsMargins();
    }
    photo_grid = QImage(paperW, paperH, QImage::Format_RGB32);
    photo_grid.fill(Qt::white);
    QPainter painter(&photo_grid);
    int border_width = (int)round(DPI/300.0);
    painter.setPen(QPen(Qt::black, border_width));
    for (int i=0; i<rows*cols; i++) {
        if (cells[i].photo.isNull())
            continue;
        QImage img = cells[i].src_photo->scaled(W, H, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        int row = i/cols;
        int col = i%cols;
        int x = marginX + col*(spacingX+W)  + (W-img.width())/2/*for center align*/;
        int y = marginY + row*(spacingY+H)  + (H-img.height())/2;
        painter.drawImage(x, y, img);
        if (add_border){
            painter.drawRect(x, y, img.width()-1, img.height()-1);
        }
    }
    painter.end();
}

void
GridPaper:: dragEnterEvent(QDragEnterEvent *ev)
{
    if (ev->mimeData()->hasUrls())
        ev->acceptProposedAction();
    else
        ev->ignore();
}

void
GridPaper:: dropEvent(QDropEvent *ev)
{
    if ( ev->mimeData()->hasUrls() )
    {
        foreach ( const QUrl & url, ev->mimeData()->urls() )
        {
            QString str = url.toLocalFile();
            if (not str.isEmpty())
            {
                QImage img = loadImage(str);
                emit addPhotoRequested(img);
            }
        }
    }
    ev->ignore();
}

void
GridPaper:: setPhoto(Thumbnail *thumb)
{
    photo = &thumb->photo;
}
