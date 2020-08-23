/* This file is a part of photoquick program, which is GPLv3 licensed */

#include "common.h"
#include "photogrid.h"
#include "pdfwriter.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QPen>
#include <QDesktopWidget>
#include <QSettings>
#include <QBuffer>
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
    connect(checkAddBorder, SIGNAL(clicked(bool)), gridPaper, SLOT(toggleBorder(bool)));
    connect(helpBtn, SIGNAL(clicked()), this, SLOT(showHelp()));
    connect(gridPaper, SIGNAL(addPhotoRequested(QImage)), this, SLOT(addPhoto(QImage)));

    addPhoto(img);
    QSettings settings(this);
    DPI = settings.value("DPI", 300).toInt();
    cellW = settings.value("GridCellW", 3.0).toFloat();
    cellH = settings.value("GridCellH", 4.0).toFloat();
    paperW = settings.value("GridPaperW", 6.0).toFloat();
    paperH = settings.value("GridPaperH", 4.0).toFloat();
    unit = settings.value("GridPaperUnit", 0).toInt();
    if (paperW<1.0 or paperW>30.0 or paperH<1.0 or paperH>30.0 or unit>1) {
        QMessageBox::critical(parent, "Error loading settings",
                "Error loading paper size settings.\nPlease Configure photo grid.");
        return;
    }
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
    dialog->spinPaperWidth->setValue(paperW);
    dialog->spinPaperHeight->setValue(paperH);
    dialog->paperSizeUnit->setCurrentIndex(unit);
    dialog->spinDPI->setValue(DPI);
    if ( dialog->exec() != QDialog::Accepted )
        return;
    cellW = dialog->spinPhotoWidth->value();
    cellH = dialog->spinPhotoHeight->value();
    paperW = dialog->spinPaperWidth->value();
    paperH = dialog->spinPaperHeight->value();
    unit = dialog->paperSizeUnit->currentIndex();
    DPI = dialog->spinDPI->value();
    setup();
    // save settings
    QSettings settings(this);
    settings.setValue("DPI", DPI);
    settings.setValue("GridCellW", cellW);
    settings.setValue("GridCellH", cellH);
    settings.setValue("GridPaperW", paperW);
    settings.setValue("GridPaperH", paperH);
    settings.setValue("GridPaperUnit", unit);
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
    verticalLayout->addWidget(thumbnail);
    connect(thumbnail, SIGNAL(clicked(Thumbnail*)), gridPaper, SLOT(setPhoto(Thumbnail*)));
    thumbnailGr->append(thumbnail);
    thumbnailGr->selectThumbnail(thumbnail);
    gridPaper->photo = img;
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

// GridPaper class methods
GridPaper:: GridPaper(QWidget *parent) : QLabel(parent)
{
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setMouseTracking(true);
    setAcceptDrops(true);
    add_border = true;
}

void
GridPaper:: setupGrid()
{
    boxes.clear();
    image_dict.clear();
    calcRowsCols(paperW, paperH, W, H, rows, cols);
    spacingX = (paperW-cols*W)/float(cols+1);
    spacingY = (paperH-rows*H)/float(rows+1);
    // Setup Foreground Grid
    float screenDPI = QApplication::desktop()->logicalDpiX();
    scale = screenDPI/DPI;
    int w = W*scale;
    int h = H*scale;
    float spacing_x = spacingX*scale;
    float spacing_y = spacingY*scale;
    for (int i=0; i<cols*rows; ++i) {
        int row = i/cols;            // Position of the box as row & col
        int col = i%cols;
        int box_x = spacing_x + col*(spacing_x+w);
        int box_y = spacing_y + row*(spacing_y+h);
        QRect box = QRect(box_x, box_y, w, h);
        boxes << box;
    }
    // create canvas pixmap for displaying on screen
    canvas_pixmap = QPixmap(paperW*scale, paperH*scale);
    canvas_pixmap.fill();
    // draw empty cells
    painter.begin(&canvas_pixmap);
    foreach (QRect box, boxes)
        painter.drawRect(box.x(), box.y(), box.width()-1, box.height()-1);
    painter.end();
    setPixmap(canvas_pixmap);
}


void
GridPaper:: toggleBorder(bool enable)
{
    add_border = enable;
    // first fill cells with image, then draw border if reauired. empty cells are untouched
    painter.begin(&canvas_pixmap);
    foreach (int index, image_dict.keys()) {
        QRect box = boxes[index];
        QImage img = image_dict.value(index).scaled(box.width(), box.height(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        QPoint offset = QPoint(box.width()-img.width(), box.height()-img.height())/2;
        painter.drawImage(box.topLeft()+offset, img);
        if (enable)
            painter.drawRect(box.x()+offset.x(), box.y()+offset.y(), img.width()-1, img.height()-1);
    }
    painter.end();
    setPixmap(canvas_pixmap);
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
    int rect_w = ev->pos().x() - click_pos.x();
    int rect_h = ev->pos().y() - click_pos.y();
    if (not (abs(rect_w)>2 and abs(rect_h)>2)) return;
    QPixmap pm = canvas_pixmap.copy();// copy for temporary drawing
    painter.begin(&pm);
    painter.drawRect(click_pos.x(), click_pos.y(), rect_w, rect_h);
    painter.end();
    this->setPixmap(pm);
}

void
GridPaper:: mouseReleaseEvent(QMouseEvent *ev)
{
    int rect_w = ev->pos().x() - click_pos.x() + 1;
    int rect_h = ev->pos().y() - click_pos.y() + 1;
    QRect rect(click_pos.x(), click_pos.y(), rect_w, rect_h);
    // draw images if the rect intersects cells
    painter.begin(&canvas_pixmap);
    foreach (QRect box, boxes) {
        if (not box.intersects(rect))
            continue;
        QImage img = photo.scaled(box.width(), box.height(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        painter.fillRect(box, Qt::white); // erase prev photo if any
        // put the image at the center of the box
        QPoint offset = QPoint(box.width()-img.width(), box.height()-img.height())/2;
        painter.drawImage(box.topLeft()+offset, img);
        if (add_border)
            painter.drawRect(box.x()+offset.x(), box.y()+offset.y(), img.width()-1, img.height()-1);
        image_dict[boxes.indexOf(box)] = photo;
    }
    painter.end();
    this->setPixmap(canvas_pixmap);
    mouse_pressed = false;
}

void
GridPaper:: createFinalGrid()
{
    photo_grid = QImage(paperW, paperH, QImage::Format_RGB32);
    photo_grid.fill(Qt::white);
    QPainter painter(&photo_grid);
    foreach (int index, image_dict.keys()) {
        int row = index/cols;
        int col = index%cols;
        QImage img = image_dict.value(index).scaled(W, H, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        int x = spacingX + col*(spacingX+W)  + (W-img.width())/2/*for center align*/;
        int y = spacingY + row*(spacingY+H)  + (H-img.height())/2;
        painter.drawImage(x, y, img);
        if (add_border)
            painter.drawRect(x, y, img.width()-1, img.height()-1);
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
    photo = thumb->photo;
}




// *************************** ------------- ***************************
// *************************** Photo Collage ***************************
// *************************** ------------- ***************************

// ******************* Collage Dialog ***********************

CollageDialog:: CollageDialog(QWidget *parent) : QDialog(parent)
{
    setupUi(this);
    QHBoxLayout *layout = new QHBoxLayout(scrollAreaWidgetContents);
    layout->setContentsMargins(0, 0, 0, 0);
    QSettings settings;
    int W = settings.value("CollageW", 1024).toInt();
    int H = settings.value("CollageH", 720).toInt();
    int pdf_w = settings.value("CollagePdfW", 595).toInt();
    int pdf_h = settings.value("CollagePdfH", 842).toInt();
    int dpi = settings.value("CollagePdfDpi", 300).toInt();
    collagePaper = new CollagePaper(this, W, H, pdf_w, pdf_h, dpi);
    layout->addWidget(collagePaper);
    connect(collagePaper, SIGNAL(statusChanged(QString)), this, SLOT(showStatus(QString)));
    connect(addBtn, SIGNAL(clicked()), collagePaper, SLOT(addPhoto()));
    connect(removeBtn, SIGNAL(clicked()), collagePaper, SLOT(removePhoto()));
    connect(copyBtn, SIGNAL(clicked()), collagePaper, SLOT(copyPhoto()));
    connect(rotateBtn, SIGNAL(clicked()), collagePaper, SLOT(rotatePhoto()));
    connect(addBorderBtn, SIGNAL(clicked()), collagePaper, SLOT(toggleBorder()));
    connect(configBtn, SIGNAL(clicked()), this, SLOT(setupBackground()));
    connect(savePdfBtn, SIGNAL(clicked()), collagePaper, SLOT(savePdf()));
    connect(okBtn, SIGNAL(clicked()), this, SLOT(accept()));
    connect(cancelBtn, SIGNAL(clicked()), this, SLOT(reject()));
    savePdfBtn->setEnabled(dpi != 0);
}

void
CollageDialog:: setupBackground()
{
    CollageSetupDialog *dlg = new CollageSetupDialog(this);
    if (dlg->exec() == QDialog::Rejected) return;
    QSettings settings;
    collagePaper->background_filename = "";
    collagePaper->dpi = 0;
    if (dlg->resolutionBtn->isChecked()) {
        collagePaper->W = dlg->widthEdit->text().toInt();
        collagePaper->H = dlg->heightEdit->text().toInt();
        settings.setValue("CollageW", collagePaper->W);
        settings.setValue("CollageH", collagePaper->H);
        settings.setValue("CollagePdfDpi", 0);  // this prevents using pdf paper size
    }
    else if (dlg->pageSizeBtn->isChecked()) {
        // convert other units to point (1/72 inch))
        float factor = 1.0;
        if (dlg->unitsCombo->currentText() == QString("in")) {
            factor = 72.0;
        }
        else if (dlg->unitsCombo->currentText() == QString("mm"))
            factor = 72/25.4;
        collagePaper->pdf_w = dlg->pageWidth->value()*factor;
        collagePaper->pdf_h = dlg->pageHeight->value()*factor;
        collagePaper->dpi = dlg->dpiCombo->currentText().toInt();
        settings.setValue("CollagePdfW", collagePaper->pdf_w);
        settings.setValue("CollagePdfH", collagePaper->pdf_h);
        settings.setValue("CollagePdfDpi", collagePaper->dpi);
    }
    else {
        collagePaper->background_filename = dlg->filename;
    }
    savePdfBtn->setEnabled(collagePaper->dpi != 0);
    collagePaper->setup();
}

void
CollageDialog:: showStatus(QString status)
{
    statusbar->setText(status);
}

void
CollageDialog:: accept()
{
    collage = collagePaper->getFinalCollage();
    collagePaper->clean();
    QDialog::accept();
}

void
CollageDialog:: reject()
{
    collagePaper->clean();
    QDialog::reject();
}

// *************************** Collage Paper ***************************

// get a size of collage paper that fits inside window, and keeps aspect ratio
void getOptimumSize(int W, int H, int &out_w, int &out_h)
{
    if (W > H) { // landscape
        fitToSize(W,H, 1000,670, out_w, out_h);
    }
    else {
        if (W<=800) {
            out_w = W;
            out_h = H;
            return;
        }
        out_w = 800;
        out_h = round((800.0/W)*H);
    }
}

CollagePaper:: CollagePaper(QWidget *parent, int w, int h, int pdf_w, int pdf_h, int dpi)
                        : QLabel(parent), W(w), H(h), dpi(dpi), pdf_w(pdf_w), pdf_h(pdf_h)
{
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setMouseTracking(true);
    setAcceptDrops(true);
    background_filename = "";
    drag_icon = QPixmap(":/images/drag.png");
    setup();
}

void
CollagePaper:: setup()
{
    // load and set bg image
    if (dpi) {
        W = pdf_w*dpi/72.0;
        H = pdf_h*dpi/72.0;
        // calculate optimum paper resolution (assuming 100 dpi screen)
        paper = QPixmap(round(pdf_w*100/72.0), round(pdf_h*100/72.0));
        paper.fill();
    }
    else if (not background_filename.isEmpty()) {
        QImage img = loadImage(background_filename);
        W = img.width();
        H = img.height();
        int opt_w, opt_h;
        getOptimumSize(W,H, opt_w, opt_h);
        paper = QPixmap::fromImage(img.scaled(opt_w, opt_h));
    }
    else {
        int opt_w, opt_h;
        getOptimumSize(W,H, opt_w, opt_h);
        paper = QPixmap(opt_w, opt_h);
        paper.fill();
    }
    // resize and reposition items when a paper size is changed
    for (CollageItem *item : collageItems) {
        item->x = 0;
        item->y = 0;
        if (item->w > paper.width() or item->h > paper.height())
            fitToSize(item->img_w, item->img_h, paper.width(), paper.height(), item->w, item->h);
    }
    draw();
}

void
CollagePaper:: addItem(CollageItem *item)
{
    if (item->isNull()){
        delete item;
        return;
    }
    item->w = round(item->img_w*100/300.0); // 300 dpi image over 100 ppi screen
    item->h = round(item->img_h*100/300.0);
    if (item->w > paper.width() or item->h > paper.height())
        fitToSize(item->img_w, item->img_h, paper.width(), paper.height(), item->w, item->h);
    item->x = MIN(item->x, paper.width() -item->w);
    item->y = MIN(item->y, paper.height()-item->h);
    if (not collageItems.isEmpty())
        item->border = collageItems.last()->border;
    collageItems.append(item);
    draw();
    updateStatus();
}

void
CollagePaper:: addPhoto()
{
    QString filefilter = "Image files (*.jpg *.jpeg *.png)";
    QStringList filenames = QFileDialog::getOpenFileNames(this, "Add Photos", "", filefilter);
    if (filenames.isEmpty()) return;
    for (QString filepath : filenames) {
        CollageItem *item = new CollageItem(filepath);
        addItem(item);
    }
}

void
CollagePaper:: removePhoto()
{
    if (collageItems.isEmpty()) return;
    CollageItem *item = collageItems.takeLast();
    delete item;
    draw();
    updateStatus();
}

void
CollagePaper:: copyPhoto()
{
    if (collageItems.isEmpty()) return;
    CollageItem *item = new CollageItem(collageItems.last());
    collageItems.append(item);
    draw();
}

void
CollagePaper:: rotatePhoto()
{
    if (collageItems.isEmpty()) return;
    CollageItem *item = collageItems.last();
    item->rotation += 90;
    item->rotation %= 360;  // keep angle between 0 and 359
    int tmp = item->w;
    item->w = item->h;
    item->h = tmp;
    QTransform tfm;
    tfm.rotate(90);
    item->pixmap = item->pixmap.transformed(tfm);
    draw();
}

void
CollagePaper:: toggleBorder()
{
    if (collageItems.isEmpty()) return;
    CollageItem *item = collageItems.last();
    item->border = not item->border;
    draw();
}

void
CollagePaper:: dragEnterEvent(QDragEnterEvent *ev)
{
    if (ev->mimeData()->hasUrls())
        ev->acceptProposedAction();
    else
        ev->ignore();
}

void
CollagePaper:: dropEvent(QDropEvent *ev)
{
    if ( ev->mimeData()->hasUrls() )
    {
        QPoint item_pos = ev->pos();
        foreach ( const QUrl & url, ev->mimeData()->urls() )
        {
            QString str = url.toLocalFile();
            if (not str.isEmpty())
            {
                CollageItem *item = new CollageItem(str);
                item->x = item_pos.x();
                item->y = item_pos.y();
                addItem(item);
                item_pos += QPoint(2,2);
            }
        }
    }
    ev->ignore();
}

void
CollagePaper:: mousePressEvent(QMouseEvent *ev)
{
    clk_pos = ev->pos();
    for (int i=collageItems.size()-1; i>=0; i--)
    {
        CollageItem *item = collageItems.at(i);
        if (item->contains(ev->pos()))
        {
            collageItems.removeAt(i);
            collageItems.append(item);
            mouse_pressed = true;
            if (item->overCorner(ev->pos()))
                corner_clicked = true;
            break;
        }
    }
    draw();
    updateStatus();
}

void
CollagePaper:: mouseMoveEvent(QMouseEvent *ev)
{
    if (not mouse_pressed) return;
    if (collageItems.isEmpty()) return;
    CollageItem *item = collageItems.last();
    QPoint moved = ev->pos() - clk_pos;
    clk_pos = ev->pos();
    if (corner_clicked) {
        int box_w = item->w + moved.x();
        int box_h = item->h + moved.y();
        double img_aspect = item->pixmap.width()/(double)item->pixmap.height();
        int w = round(box_h * img_aspect);
        int h = round(box_w / img_aspect);
        if (h<=box_h) {
            item->w = box_w;
            item->h = h;
        }
        else {
            item->h = box_h;
            item->w = w;
        }
    }
    else
    {
        int new_x = MAX(item->x+moved.x(), 0);
        int new_y = MAX(item->y+moved.y(), 0);
        item->x = MIN(new_x, paper.width()-item->w);
        item->y = MIN(new_y, paper.height()-item->h);
    }
    draw();
    updateStatus();
}

void
CollagePaper:: mouseReleaseEvent(QMouseEvent */*ev*/)
{
    mouse_pressed = false;
    corner_clicked = false;
}

void
CollagePaper:: draw()
{
    QPixmap pm = paper.copy();
    QPainter painter(&pm);
    if (dpi) {              // my printer has bottom margin of 41.05 pt
        painter.setPen(Qt::gray);
        int offset = round((42.0*pm.height())/pdf_h);
        painter.drawLine(offset, pm.height()-offset, pm.width()-offset, pm.height()-offset);
    }
    for (CollageItem *item : collageItems)
    {
        painter.drawPixmap(item->x, item->y, item->w, item->h, item->pixmap);
        if (item == collageItems.last()) {     // highlight the selected (last) item
            QPen pen(Qt::blue, 2);
            painter.setPen(pen);
            painter.drawRect(item->x+1, item->y+1, item->w-2, item->h-2);
            painter.setPen(Qt::black);
        }
        if (item->border)
            painter.drawRect(item->x, item->y, item->w-1, item->h-1);
        painter.drawPixmap(item->x+item->w-16, item->y+item->h-16, drag_icon);
    }
    painter.end();
    setPixmap(pm);
}

QImage
CollagePaper:: getFinalCollage()
{
    double scaleX = ((double)W)/paper.width();
    double scaleY = ((double)H)/paper.height();
    QPixmap pm;
    if (not background_filename.isEmpty())
        pm = QPixmap::fromImage(loadImage(background_filename));
    else {
        pm = QPixmap(W, H);
        pm.fill();
    }
    QPainter painter(&pm);
    for (CollageItem *item : collageItems)
    {
        QPixmap pixmap = QPixmap::fromImage(item->image());
        if (item->rotation) {
            QTransform tfm;
            tfm.rotate(item->rotation);
            pixmap = pixmap.transformed(tfm);
        }
        pixmap = pixmap.scaled(round(item->w*scaleX), round(item->h*scaleY),
                            Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        painter.drawPixmap(round(item->x*scaleX), round(item->y*scaleY), pixmap);
        if (item->border)
            painter.drawRect(round(item->x*scaleX), round(item->y*scaleY),
                            pixmap.width()-1, pixmap.height()-1);
    }
    return pm.toImage();
}

void
CollagePaper:: savePdf()
{
    if (collageItems.isEmpty()) return;
    QFileInfo fi(collageItems.last()->filename);
    QString dir = fi.dir().path();
    QString filename("photo-collage.pdf");
    QString path(dir+ "/" + filename);
    int num=1;
    while (QFileInfo(path).exists()) {
        filename = "photo-collage-" + QString::number(num++) + ".pdf";
        path = dir + "/" + filename;
    }
    float scaleX = pdf_w/(float)paper.width();
    float scaleY = pdf_h/(float)paper.height();
    PdfWriter writer;
    writer.begin(path.toStdString());
    PdfObj cont;
    std::string cont_strm = "";
    PdfDict resources;
    PdfDict imgs;
    PdfObj img;
    img.set("Type", "/XObject");
    img.set("Subtype", "/Image");
    img.set("ColorSpace", "/DeviceRGB");
    img.set("BitsPerComponent", "8");
    img.set("Filter", "/DCTDecode"); // jpg = DCTDecode, for png = FlateDecode
    for (int i=0; i<collageItems.count(); i++)
    {
        CollageItem *item = collageItems.at(i);
        img.set("Width", item->img_w);
        img.set("Height", item->img_h);
        if (item->jpgOnDisk()) {
            std::string path_str = item->filename.toUtf8().constData();
            writer.addObj(img, readFile(path_str) );
        }
        else {
            QByteArray bArray;
            QBuffer buffer(&bArray);
            buffer.open(QIODevice::WriteOnly);
            QImage image = item->image();
            // remove transperancy
            if (image.format()==QImage::Format_ARGB32) {
                QImage new_img(image.width(), image.height(), QImage::Format_RGB32);
                new_img.fill(Qt::white);
                QPainter painter(&new_img);
                painter.drawImage(0,0, image);
                painter.end();
                image = new_img;
            }
            image.save(&buffer, "JPG");
            std::string data(bArray.data(), bArray.size());
            writer.addObj(img, data);
            bArray.clear();
            buffer.close();
        }
        std::string matrix = imgMatrix(item->x*scaleX,
                                pdf_h - item->y*scaleY - item->h*scaleY, // img Y to pdf Y
                                item->w*scaleX, item->h*scaleY, item->rotation);
        cont_strm += format("q %s /img%d Do Q\n", matrix.c_str(), i);
        imgs.set(format("img%d",i), img.byref());
    }
    writer.addObj(cont, cont_strm);
    resources.set("XObject", imgs);
    PdfObj page = writer.createPage(pdf_w, pdf_h);
    page.set("Contents", cont);
    page.set("Resources", resources);
    writer.addPage(page);
    writer.finish();
    Notifier *notifier = new Notifier(this);
    notifier->notify("PDF Saved !", "Pdf Saved as \n" + filename);
}

void
CollagePaper:: updateStatus()
{
    if (collageItems.isEmpty()) {
        emit statusChanged(QString(""));
        return;
    }
    CollageItem *item = collageItems.last();
    QString status;
    int w = item->w*((float)W)/paper.width();
    int h = item->h*((float)H)/paper.height();
    status.sprintf("Resolution : %dx%d", w, h);
    if (dpi) {
        float w_cm = item->w*pdf_w/(float)paper.width()*2.54/72;
        float h_cm = item->h*pdf_h/(float)paper.height()*2.54/72;
        status += QString().sprintf(",   Size : %.1fx%.1f cm", w_cm, h_cm);
    }
    emit statusChanged(status);
}

void
CollagePaper:: clean()
{
    while (not collageItems.isEmpty()) {
        CollageItem *item = collageItems.takeLast();
        delete item;
    }
}

// ******************* Collage Item *********************

CollageItem:: CollageItem(QString filename) : x(0), y(0)
{
    QImage img = loadImage(filename);
    if (img.isNull()) {
        isValid_ = false;
        return;
    }
    // resize it
    pixmap = QPixmap::fromImage(img);
    img_w = pixmap.width();
    img_h = pixmap.height();
    if (img_w > 600 and img_h > 600)
        pixmap = pixmap.scaled(600, 600, Qt::KeepAspectRatio,Qt::SmoothTransformation);
    this->filename = filename;
    this->image_ = QImage();     // null image
}

CollageItem:: CollageItem(QImage img) : x(0), y(0)
{
    if (img.isNull()) {
        isValid_ = false;
        return;
    }
    // resize it
    pixmap = QPixmap::fromImage(img);
    img_w = pixmap.width();
    img_h = pixmap.height();
    if (img_w > 600 and img_h > 600)
        pixmap = pixmap.scaled(600, 600, Qt::KeepAspectRatio,Qt::SmoothTransformation);
    this->filename = QString("");
    this->image_ = img;
}

bool
CollageItem:: isNull()
{
    return not isValid_;
}

bool
CollageItem:: jpgOnDisk()
{
    return (not (not image_.isNull() or this->border or
            this->filename.endsWith(".png", Qt::CaseInsensitive)));
}

bool
CollageItem:: contains(QPoint pos)
{
    return ((x <= pos.x() and pos.x() < x+w) and
            (y <= pos.y() and pos.y() < y+h));
}

bool
CollageItem:: overCorner(QPoint pos)
{
    return (x+w-20 <= pos.x() and pos.x() < x+w and
            y+h-20 <= pos.y() and pos.y() < y+h);
}

QImage
CollageItem:: image()
{
    QImage img;
    if (not this->image_.isNull()) {
        img = image_;
    }
    else {
        img = loadImage(this->filename);
    }
    if (this->border) {
        QPainter painter(&img);
        painter.drawRect(0,0, img.width()-1, img.height()-1);
        painter.end();
    }
    return img;
}

// ****************** Collage Setup Dialog ****************** //

CollageSetupDialog:: CollageSetupDialog(QWidget *parent) : QDialog(parent)
{
    setupUi(this);
    customSize->setEnabled(false);
    connect(selectFileBtn, SIGNAL(clicked()), this, SLOT(selectFile()));
    connect(pageSizeCombo, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(toggleUsePageSize(const QString&)));
    filename = "";
}

void
CollageSetupDialog:: selectFile()
{
    if (not bgPhotoBtn->isChecked()) return;
    QString filefilter = "Image files (*.jpg *.png *.jpeg);;JPEG Images (*.jpg *.jpeg);;"
                         "PNG Images (*.png)";
    QString filepath = QFileDialog::getOpenFileName(this, "Add Photo", "", filefilter);
    if (filepath.isEmpty()) return;
    this->filename = filepath;
    QString text = QFileInfo(filepath).fileName();
    QFontMetrics fontMetrics(filenameLabel->font());
    text = fontMetrics.elidedText(text, Qt::ElideMiddle, filenameLabel->width()-2);
    filenameLabel->setText(text);
}

void
CollageSetupDialog:: toggleUsePageSize(const QString& text)
{
    customSize->setEnabled(text == QString("Custom"));
}

void
CollageSetupDialog:: accept()
{
    if (resolutionBtn->isChecked()) {
        if (widthEdit->text().isEmpty() or heightEdit->text().isEmpty()) return;
    }
    else if (pageSizeBtn->isChecked()) {
        QString text = pageSizeCombo->currentText();
        if (text == QString("A4")) {
            pageWidth->setValue(595.0);
            pageHeight->setValue(842.0);
        }
        else if (text == QString("A5")) {
            pageWidth->setValue(420.0);
            pageHeight->setValue(595.0);
        }
        else if (text == QString("4x6 inch")) {
            pageWidth->setValue(432.0);
            pageHeight->setValue(288.0);
        }
        if (text != QString("Custom"))
            unitsCombo->setCurrentIndex(0); // unit = pt
    }
    else if (bgPhotoBtn->isChecked()) {
        if (filename.isEmpty()) return;
    }
    QDialog::accept();
}
