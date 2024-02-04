/* This file is a part of photoquick program, which is GPLv3 licensed */

#include "common.h"
#include "photogrid.h"
#include "pdfwriter.h"
#include <QFileDialog>
#include <QDesktopWidget>
#include <QSettings>
#include <QBuffer>
#include <QMimeData>
#include <QUrl>
#include <cmath>


typedef struct {
    float w;
    float h;
} PageSize;


GridDialog:: GridDialog(QImage img, QWidget *parent) : QDialog(parent)
{
    setupUi(this);
    QHBoxLayout *layout = new QHBoxLayout(scrollAreaWidgetContents);
    layout->setContentsMargins(0, 0, 0, 0);
    gridView = new GridView(this);
    layout->addWidget(gridView);

    thumbnailLayout = new QVBoxLayout(thumbnailContainer);
    thumbnailLayout->addStretch();

    addPhoto(img);
    // load settings
    QSettings settings(this);
    settings.beginGroup("PhotoGrid");
    int pagesize_index = settings.value("PaperSizeIndex", 0).toInt();
    float custom_page_w = settings.value("CustomPaperW", 4.0).toFloat();
    float custom_page_h = settings.value("CustomPaperH", 6.0).toFloat();
    QString custom_page_unit = settings.value("CustomPaperUnit", "in").toString();
    float min_margin = settings.value("MinMargin", 2.0).toFloat();
    int dpi = settings.value("DPI", 300).toInt();
    float cellW = settings.value("CellW", 2.8).toFloat();
    float cellH = settings.value("CellH", 3.6).toFloat();
    bool narrow_spacing = settings.value("NarrowSpacing", false).toBool();
    bool add_border = settings.value("AddBorder", true).toBool();
    settings.endGroup();

    pageSizeCombo->setCurrentIndex(pagesize_index);
    customPageSizeWidget->setVisible(pageSizeCombo->currentText()=="Custom");
    customWSpin->setValue(custom_page_w);
    customHSpin->setValue(custom_page_h);
    int index = customUnitCombo->findText(custom_page_unit);
    if (index>=0)
        customUnitCombo->setCurrentIndex(index);
    minMarginSpin->setValue(min_margin);
    dpiSpin->setValue(dpi);
    cellWSpin->setValue(cellW);
    cellHSpin->setValue(cellH);
    narrowSpacingBtn->setChecked(narrow_spacing);
    addBorderBtn->setChecked(add_border);


    // connect signals
    connect(pageSizeCombo, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(onPageSizeChange(QString)));
    connect(customWSpin, SIGNAL(valueChanged(double)), this, SLOT(setupGrid()));
    connect(customHSpin, SIGNAL(valueChanged(double)), this, SLOT(setupGrid()));
    connect(customUnitCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(setupGrid()));
    connect(minMarginSpin, SIGNAL(valueChanged(double)), this, SLOT(setupGrid()));
    connect(dpiSpin, SIGNAL(valueChanged(int)), this, SLOT(setupGrid()));
    connect(cellWSpin, SIGNAL(valueChanged(double)), this, SLOT(setupGrid()));
    connect(cellHSpin, SIGNAL(valueChanged(double)), this, SLOT(setupGrid()));
    connect(narrowSpacingBtn, SIGNAL(clicked(bool)), this, SLOT(setupGrid()));
    connect(addBorderBtn, SIGNAL(clicked(bool)), this, SLOT(setupGrid()));
    connect(addPhotoBtn, SIGNAL(clicked()), this, SLOT(addPhoto()));
    connect(gridView, SIGNAL(photoDropped(QImage)), this, SLOT(addPhoto(QImage)));
    connect(savePdfBtn, SIGNAL(clicked(bool)), gridView, SLOT(savePdf()));

    setupGrid();
}

void
GridDialog:: onPageSizeChange(QString page_size)
{
    customPageSizeWidget->setVisible(page_size=="Custom");
    setupGrid();
}


void
GridDialog:: setupGrid()
{
    float pageW, pageH;
    QList<PageSize> page_sizes = { {288, 432}, {360, 504}, // 4x6", 5x7"
                        {595, 842}, {420, 595}, {298, 420} };// A4,A5,A6
    if (pageSizeCombo->currentIndex()<page_sizes.size()){
        PageSize size = page_sizes[pageSizeCombo->currentIndex()];
        pageW = (float)size.w;
        pageH = (float)size.h;
    }
    else {// Custom
        std::map<QString, float> unit_factors = {{"cm", 72/2.54}, {"in", 72}};
        float unit_factor = unit_factors[customUnitCombo->currentText()];
        pageW = customWSpin->value() * unit_factor;
        pageH = customHSpin->value() * unit_factor;
    }
    gridView->pageW = pageW;
    gridView->pageH = pageH;
    gridView->cellW = cellWSpin->value()/2.54*72;
    gridView->cellH = cellHSpin->value()/2.54*72;
    gridView->minMargin = minMarginSpin->value()/25.4*72;
    gridView->dpi = dpiSpin->value();
    gridView->narrow_spacing = narrowSpacingBtn->isChecked();
    gridView->add_border = addBorderBtn->isChecked();
    gridView->setup();
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
    thumbnailLayout->insertWidget(0, thumbnail, 0/*stretch*/, Qt::AlignCenter);
    connect(thumbnail, SIGNAL(clicked(Thumbnail*)), this, SLOT(selectThumbnail(Thumbnail*)));
    thumbnails.push_back(thumbnail);
    selectThumbnail(thumbnail);
}

void
GridDialog:: selectThumbnail(Thumbnail *thumbnail)
{
    for (auto thumb : thumbnails)
        thumb->setSelected(false);
    thumbnail->setSelected(true);
    gridView->curr_photo = &thumbnail->photo;
}

void
GridDialog:: accept()
{
    QSettings settings(this);
    settings.beginGroup("PhotoGrid");
    settings.setValue("PaperSizeIndex", pageSizeCombo->currentIndex());
    settings.setValue("CustomPaperW", customWSpin->value());
    settings.setValue("CustomPaperH", customHSpin->value());
    settings.setValue("CustomPaperUnit", customUnitCombo->currentText());
    settings.setValue("MinMargin", minMarginSpin->value());
    settings.setValue("DPI", dpiSpin->value());
    settings.setValue("CellW", cellWSpin->value());
    settings.setValue("CellH", cellHSpin->value());
    settings.setValue("NarrowSpacing", narrowSpacingBtn->isChecked());
    settings.setValue("AddBorder", addBorderBtn->isChecked());
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


void
Thumbnail:: setSelected(bool select)
{
    QImage img = photo.scaledToWidth(100);
    if (select) {
        QPainter painter(&img);
        QPen pen(Qt::blue);
        pen.setWidth(4);
        painter.setPen(pen);
        painter.drawRect(2, 2 , 100-4, img.height()-4);
        painter.end();
    }
    setPixmap(QPixmap::fromImage(img));
}


// calculates number of rows and cols of items in grid paper
void calcRowsCols(float pageW, float pageH, float &cellW, float &cellH, int &rows, int &cols)
{
    rows = pageH/cellH;
    cols = pageW/cellW;
    // no. of rows and cols if we rotate the cell size
    int rows2 = pageH/cellW;
    int cols2 = pageW/cellH;
    if (rows*cols < rows2*cols2) {
        rows = rows2;
        cols = cols2;
        SWAP(cellW,cellH);
    }
}



GridView:: GridView(QWidget *parent) : QLabel(parent)
{
    // set size to pixmap size
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setMouseTracking(true);// accept mouse move event
    setAcceptDrops(true);
}

void
GridView:: setup()
{
    // cell size may be changed, need to clear cache
    cached_images.clear();
    // setup
    float availableW = pageW - 2*minMargin;
    float availableH = pageH - 2*minMargin;
    calcRowsCols(availableW, availableH, cellW, cellH, row_count, col_count);
    // Calculate Spacing
    float spacingX = (pageW - col_count*cellW)/float(col_count+1);
    if (spacingX<minMargin)
        spacingX = (availableW - col_count*cellW)/float(col_count-1);
    float spacingY = (pageH - row_count*cellH)/float(row_count+1);
    if (spacingY<minMargin)
        spacingY = (availableH - row_count*cellH)/float(row_count-1);
    spacing = MIN(spacingX, spacingY);
    // half milimeter uniform spacing
    if (narrow_spacing){
        spacing = MIN(spacing, 1.5);
    }
    marginX = (pageW - col_count*cellW - (col_count-1)*spacing)/2;
    marginY = (pageH - row_count*cellH - (row_count-1)*spacing)/2;

    // may have extra cells from prev configuration, remove them
    while ((int)cells.size() > row_count*col_count)
        cells.pop_back();
    // calculate pos of each cells
    for (int i=0; i<row_count*col_count; i++) {
        if (i==(int)cells.size())
            cells.push_back(GridCell({0,0,NULL}));
        int row = i/col_count;
        int col = i%col_count;
        cells[i].x = marginX + col*(cellW+spacing);
        cells[i].y = marginY + row*(cellH+spacing);
    }
    float screen_dpi = QApplication::desktop()->logicalDpiX();
    px_factor = screen_dpi/72.0;
    redraw();
}

QImage
GridView:: cachedScaledImage(QImage *image)
{
    if (cached_images.count(image)==0){// not yet cached
        int cell_w = cellW * px_factor;
        int cell_h = cellH * px_factor;
        // rotate when cell is not square and both cell and image have same orientation
        bool rotate = cell_w!=cell_h and (cell_w>cell_h) != (image->width()>image->height());
        if (rotate){
            SWAP(cell_w, cell_h);
        }
        QImage scaled = image->scaled(cell_w, cell_h, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        cached_images[image] = rotate ? scaled.transformed(QTransform().rotate(90)) : scaled;
        image_rotations[image] = rotate;
    }
    return cached_images[image];
}

void
GridView:: redraw()
{
    int page_w = pageW * px_factor;
    int page_h = pageH * px_factor;
    int cell_w = cellW * px_factor;
    int cell_h = cellH * px_factor;
    // create canvas pixmap for displaying on screen
    canvas_pixmap = QPixmap(page_w, page_h);
    canvas_pixmap.fill();
    // draw cells
    painter.begin(&canvas_pixmap);
    for (auto cell : cells) {
        int cell_x = cell.x * px_factor;
        int cell_y = cell.y * px_factor;
        if (cell.photo==NULL) {
            painter.drawRect(cell_x, cell_y, cell_w-1, cell_h-1);
            continue;
        }
        QImage img = cachedScaledImage(cell.photo);
        // center align the image in the box
        cell_x += (cell_w-img.width())/2;
        cell_y += (cell_h-img.height())/2;
        painter.drawImage(cell_x, cell_y, img);
        if (add_border)
            painter.drawRect(cell_x, cell_y, img.width()-1, img.height()-1);
    }
    painter.end();
    setPixmap(canvas_pixmap);
}

QImage
GridView:: finalImage()
{
    px_factor = dpi/72.0;
    int page_w = round(pageW * px_factor);
    int page_h = round(pageH * px_factor);
    int cell_w = cellW * px_factor;
    int cell_h = cellH * px_factor;

    QImage photo_grid(page_w, page_h, QImage::Format_RGB32);
    photo_grid.fill(Qt::white);
    if (cells.size()==0)
        return photo_grid;

    // if last row is empty, we want to cut and save
    // the empty bottom part of the paper for later use
    float offsetY = minMargin - cells[0].y;
    for (int i=(row_count-1)*col_count; i<row_count*col_count; i++){
        if (cells[i].photo != NULL){
            offsetY = 0;
            break;
        }
    }
    QPainter painter(&photo_grid);
    // set pen for drawing border
    int border_w = (int)round(0.24*px_factor);// 1px line per 300 dpi
    painter.setPen(QPen(Qt::black, border_w));

    for (auto cell : cells) {
        if (cell.photo == NULL)
            continue;
        QImage img = *cell.photo;
        if (image_rotations[cell.photo])
            img = img.transformed(QTransform().rotate(90));
        img = img.scaled(cell_w, cell_h, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        // center align image inside cell
        int cell_x = cell.x*px_factor + (cell_w-img.width())/2;
        int cell_y = cell.y*px_factor + (cell_h-img.height())/2;
        if (offsetY)// align to top to save paper
            cell_y = (cell.y + offsetY) * px_factor;
        // draw
        painter.drawImage(cell_x, cell_y, img);
        if (add_border){
            painter.drawRect(cell_x, cell_y, img.width()-1, img.height()-1);
        }
    }
    painter.end();
    return photo_grid;
}


void
GridView:: savePdf()
{
    QString filename("photogrid.pdf");
    QString dir = QDir::currentPath();
    QString path(dir + "/" + filename);
    int num=1;
    while (QFileInfo(path).exists()) {
        filename = "photogrid-" + QString::number(num++) + ".pdf";
        path = dir + "/" + filename;
    }
    if (cells.size()==0)
        return;

    // if last row is empty, we want to cut and save
    // the empty bottom part of the paper for later use
    float offsetY = minMargin - cells[0].y;
    for (int i=(row_count-1)*col_count; i<row_count*col_count; i++){
        if (cells[i].photo != NULL){
            offsetY = 0;
            break;
        }
    }
    PdfDocument doc;
    PdfPage *page = doc.newPage(pageW, pageH);

    std::map<QImage*, PdfObject*> pdf_img_map;

    for (auto cell : cells) {
        if (cell.photo == NULL)
            continue;
        if (pdf_img_map.count(cell.photo)<1){
            QImage image = *cell.photo;
            // Load as QImage, save to buffer as jpeg and then embed
            QBuffer buff;
            buff.open(QIODevice::WriteOnly);
            // set white background of transparent images
            if (image.format()==QImage::Format_ARGB32) {
                image = setImageBackgroundColor(image, 0xffffff);
            }
            image.save(&buff, "JPG");
            pdf_img_map[cell.photo] = doc.addImage(buff.data().data(), buff.size(),
                                    image.width(), image.height(), PDF_IMG_JPEG);
            buff.close();
        }
        PdfObject *img_obj = pdf_img_map[cell.photo];
        int img_w = cell.photo->width();
        int img_h = cell.photo->height();
        bool rotate = image_rotations[cell.photo];
        if (rotate)
            SWAP(img_w, img_h);
        float scale = fitToSizeScale(img_w, img_h, cellW, cellH);
        float imgW = scale * img_w;
        float imgH = scale * img_h;
        // center align image inside cell
        int imgX = cell.x + (cellW-imgW)/2;
        int imgY = cell.y + (cellH-imgH)/2;
        if (offsetY)// align to top to save paper
            imgY = cell.y + offsetY;
        imgY = pageH-imgY-imgH;// imgY in pdf coordinate
        page->drawImage(img_obj, imgX, imgY, imgW, imgH, rotate ? 90 : 0);
        if (add_border)
            page->drawRect(imgX, imgY, imgW, imgH, 0.24, STROKE);
    }
    doc.save(path.toStdString());
    Notifier *notifier = new Notifier(this);
    notifier->notify("PDF Saved !", "Pdf Saved as \n" + filename);
}


void
GridView:: mousePressEvent(QMouseEvent *ev)
{
    mouse_press_pos = ev->pos();
    mouse_pressed = true;
}

void
GridView:: mouseMoveEvent(QMouseEvent *ev)
{
    if (not mouse_pressed)
        return;
    QRect rect(mouse_press_pos, ev->pos());
    if (not (abs(rect.width())>2 and abs(rect.height())>2)) return;
    QPixmap pm = canvas_pixmap.copy();// copy for temporary drawing
    painter.begin(&pm);
    painter.drawRect(rect);
    painter.end();
    this->setPixmap(pm);
}

void
GridView:: mouseReleaseEvent(QMouseEvent *ev)
{
    mouse_pressed = false;
    QRect rect(mouse_press_pos, ev->pos());
    // fill cells with images if the rect intersects
    for (int i=0; i<(int)cells.size(); i++) {
        GridCell cell = cells[i];
        QRect cell_rect = QRect(cell.x*px_factor, cell.y*px_factor, cellW*px_factor, cellH*px_factor);
        if (not cell_rect.intersects(rect))
            continue;
        cells[i].photo = curr_photo;
    }
    redraw();
}


void
GridView:: dragEnterEvent(QDragEnterEvent *ev)
{
    if (ev->mimeData()->hasUrls())
        ev->acceptProposedAction();
    else
        ev->ignore();
}

void
GridView:: dropEvent(QDropEvent *ev)
{
    if ( ev->mimeData()->hasUrls() )
    {
        foreach ( const QUrl & url, ev->mimeData()->urls() )
        {
            QString str = url.toLocalFile();
            if (not str.isEmpty())
            {
                QImage img = loadImage(str);
                emit photoDropped(img);
            }
        }
    }
    ev->ignore();
}
