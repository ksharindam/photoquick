#include "photo_collage.h"

enum {
    UNIT_PIXEL,
    UNIT_POINT
};

#define PAPER_WIDTHS {595, 420, 288, 360}
#define PAPER_HEIGHTS {842, 595, 432, 504}
#define PAPER_SIZE_COUNT 4

// ******************* Collage Dialog ***********************

CollageDialog:: CollageDialog(QWidget *parent) : QDialog(parent)
{
    setupUi(this);
    QButtonGroup *bgBtnGrp = new QButtonGroup(this);
    bgBtnGrp->addButton(bgWhiteBtn);
    bgBtnGrp->addButton(bgColorBtn);
    bgBtnGrp->addButton(bgImageBtn);
    // create paper
    QHBoxLayout *layout = new QHBoxLayout(scrollAreaWidgetContents);
    layout->setContentsMargins(0, 0, 0, 0);
    collagePaper = new CollagePaper(this);
    layout->addWidget(collagePaper);
    // read settings and set values
    QSettings settings;
    settings.beginGroup("PhotoCollage");
    pageSizeCombo->setCurrentIndex( settings.value("PageSizeIndex", 0).toInt() );
    float out_w = settings.value("OutW", 595).toInt();
    float out_h = settings.value("OutH", 842).toInt();
    int unit = settings.value("Unit", UNIT_POINT).toInt();
    int out_dpi = settings.value("DPI", 300).toInt();
    settings.endGroup();
    updatePageSize(out_w, out_h, unit);
    collagePaper->out_dpi = out_dpi;
    dpiSpin->setValue(out_dpi);
    // connect signals
    connect(collagePaper, SIGNAL(statusChanged(QString)), statusbar, SLOT(setText(QString)));
    connect(pageSizeCombo, SIGNAL(activated(int)), this, SLOT(onPageSizeChange(int)));
    connect(rotatePageBtn, SIGNAL(clicked()), this, SLOT(onPageOrientationChange()));
    connect(dpiSpin, SIGNAL(valueChanged(int)), this, SLOT(onDpiChange(int)));
    connect(bgBtnGrp, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(onBackgroundChange(QAbstractButton*)));
    connect(addBtn, SIGNAL(clicked()), collagePaper, SLOT(addPhoto()));
    connect(removeBtn, SIGNAL(clicked()), collagePaper, SLOT(removePhoto()));
    connect(copyBtn, SIGNAL(clicked()), collagePaper, SLOT(copyPhoto()));
    connect(rotateBtn, SIGNAL(clicked()), collagePaper, SLOT(rotatePhoto()));
    connect(addBorderBtn, SIGNAL(clicked()), collagePaper, SLOT(toggleBorder()));
    connect(savePdfBtn, SIGNAL(clicked()), collagePaper, SLOT(savePdf()));
    connect(okBtn, SIGNAL(clicked()), this, SLOT(accept()));
    connect(cancelBtn, SIGNAL(clicked()), this, SLOT(reject()));
}

void
CollageDialog:: onPageSizeChange(int index)// SLOT
{
    float out_w, out_h;
    int out_unit = UNIT_POINT;
    QList<int> paper_widths  = PAPER_WIDTHS;// A4, A5, 4R, 5R
    QList<int> paper_heights = PAPER_HEIGHTS;
    if (index<PAPER_SIZE_COUNT){
        out_w = paper_widths[index];
        out_h = paper_heights[index];
    }
    else if (index==PAPER_SIZE_COUNT){// size from Background Image
        if (collagePaper->bg_img.isNull())
            return;
        out_w = collagePaper->bg_img.width();
        out_h = collagePaper->bg_img.height();
        out_unit = UNIT_PIXEL;
    }
    else { // Custom Size
        PageSizeDialog *dlg = new PageSizeDialog(this, collagePaper->out_w, collagePaper->out_h, collagePaper->out_unit);
        if (dlg->exec()!=QDialog::Accepted)
            return;
        out_w = dlg->out_w;
        out_h = dlg->out_h;
        out_unit = dlg->out_unit;
    }
    updatePageSize(out_w, out_h, out_unit);
}

void
CollageDialog:: updatePageSize(float out_w, float out_h, int out_unit)
{
    collagePaper->out_w = out_w;
    collagePaper->out_h = out_h;
    collagePaper->out_unit = out_unit;
    QString status("");
    if (collagePaper->out_unit==UNIT_POINT){
        status = QString("%1x%2 cm").arg(roundOff(out_w/72.0*2.54,1)).arg(roundOff(out_h/72.0*2.54,1));
    }
    else if (collagePaper->out_unit==UNIT_PIXEL)
        status = QString("%1x%2 px").arg(out_w).arg(out_h);
    pageSizeLabel->setText(status);
    collagePaper->updateSize();
    savePdfBtn->setEnabled(collagePaper->out_unit==UNIT_POINT);
}

void
CollageDialog:: onPageOrientationChange()
{
    int out_w = collagePaper->out_w;
    int out_h = collagePaper->out_h;
    updatePageSize(out_h, out_w, collagePaper->out_unit);
}

void
CollageDialog:: onDpiChange(int val)
{
    collagePaper->out_dpi = val;
}

void
CollageDialog:: onBackgroundChange(QAbstractButton* btn)
{
    if (btn->text()=="Color"){
        QColor clr = QColorDialog::getColor(collagePaper->bg_color, this);
        if (clr.isValid()){
            collagePaper->bg_color = clr;
        }
        collagePaper->bg_img = QImage();
    }
    else if (btn->text()=="Image"){
        bool ok = collagePaper->useSelectedImageAsBackground();
        if (!ok){
            QMessageBox::information(this, "No Image added !", "Please drag-drop a photo on paper \nthen change background type to Image.");
            return;
        }
        if (pageSizeCombo->currentIndex()==PAPER_SIZE_COUNT){
            onPageSizeChange(PAPER_SIZE_COUNT);
        }
    }
    else {
        collagePaper->bg_color = QColor(Qt::white);
        collagePaper->bg_img = QImage();
    }
    collagePaper->updateBackground();
}


void
CollageDialog:: accept()
{
    QSettings settings;
    settings.beginGroup("PhotoCollage");
    settings.setValue("PageSizeIndex", pageSizeCombo->currentIndex());
    settings.setValue("OutW", collagePaper->out_w);
    settings.setValue("OutH", collagePaper->out_h);
    settings.setValue("Unit", collagePaper->out_unit);
    settings.setValue("DPI", dpiSpin->value());
    settings.endGroup();
    collage = collagePaper->getFinalCollage();
    QDialog::accept();
}

void
CollageDialog:: done(int result)// gets called after accept() or reject()
{
    collagePaper->clean();
    QDialog::done(result);
}

// *************************** Collage Paper ***************************

// get a size of collage paper that fits inside window, and keeps aspect ratio
void getOptimumSize(int W, int H, int &out_w, int &out_h)
{
    if (W > H) { // landscape
        shrinkToFitSize(W,H, 1000,670, out_w, out_h);
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

CollagePaper:: CollagePaper(QWidget *parent) : QLabel(parent)
{
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setMouseTracking(true);
    setAcceptDrops(true);
    drag_icon = QPixmap(":/icons/drag.png");
    bg_color = Qt::white;
}

void
CollagePaper:: updateSize()
{
    if (out_unit==UNIT_POINT) {
        // calculate displayed paper resolution (assuming 100 dpi screen)
        scaled_w = round(100*out_w/72.0);
        scaled_h = round(100*out_h/72.0);
    }
    else { // UNIT_PIXEL
        getOptimumSize(out_w, out_h, scaled_w, scaled_h);
    }
    paper = QPixmap(scaled_w, scaled_h);
    // resize and reposition items when a paper size is changed
    for (CollageItem *item : collageItems) {
        item->x = 0;
        item->y = 0;
        if (item->w > paper.width() or item->h > paper.height())
            shrinkToFitSize(item->img_w, item->img_h, paper.width(), paper.height(), item->w, item->h);
    }
    updateBackground();// and draw()
    updateStatus();// because items may be resized
}

bool
CollagePaper:: useSelectedImageAsBackground() // must be followed by updateBackground()
{
    if (collageItems.isEmpty())
        return false;
    CollageItem *item = collageItems.takeLast();
    bg_img = item->image();
    delete item;
    updateStatus();
    return true;
}

void
CollagePaper:: updateBackground()
{
    if (!bg_img.isNull()){
        QImage scaled_bg = bg_img.scaled(paper.width(), paper.height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        paper = QPixmap::fromImage(scaled_bg);
    }
    else {
        paper.fill(bg_color);
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
        shrinkToFitSize(item->img_w, item->img_h, paper.width(), paper.height(), item->w, item->h);
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
    tmp = item->img_w;
    item->img_w = item->img_h;
    item->img_h = tmp;

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
    /*if (out_unit==UNIT_POINT) {  // my printer has bottom margin of 41.05 pt
        painter.setPen(Qt::gray);
        int offset = round((42.0*pm.height())/out_h);
        painter.drawLine(offset, pm.height()-offset, pm.width()-offset, pm.height()-offset);
    }*/
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
    double out_w_px = out_unit==UNIT_PIXEL ? out_w : round(out_dpi*out_w/72);
    double out_h_px = out_unit==UNIT_PIXEL ? out_h : round(out_dpi*out_h/72);
    double scaleX = out_w_px/paper.width();
    double scaleY = out_h_px/paper.height();
    // No Background image, use bg color
    if (bg_img.isNull()){
        bg_img = QImage(out_w_px, out_h_px, QImage::Format_RGB32);
        bg_img.fill(bg_color);
    }
    // use background image, scale only if output size mismatch
    else if (bg_img.width()!=out_w_px or bg_img.height()!=out_h_px) {
        bg_img = bg_img.scaled(out_w_px, out_h_px, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }
    QPainter painter(&bg_img);

    for (CollageItem *item : collageItems)
    {
        QImage img = item->image().scaled(round(item->w*scaleX), round(item->h*scaleY),
                            Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        painter.drawImage(round(item->x*scaleX), round(item->y*scaleY), img);
        if (item->border){
            painter.drawRect(round(item->x*scaleX), round(item->y*scaleY),
                            img.width()-1, img.height()-1);
        }
    }
    return bg_img;
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
    float scaleX = out_w/paper.width();
    float scaleY = out_h/paper.height();
    PdfDocument doc;
    PdfPage *page = doc.newPage(out_w, out_h);
    // draw background
    if (!bg_img.isNull()){
        QBuffer buff;
        buff.open(QIODevice::WriteOnly);
        bg_img.save(&buff, "JPG");
        PdfObject *img = doc.addImage(buff.data().data(), buff.size(), bg_img.width(), bg_img.height(), PDF_IMG_JPEG);
        page->drawImage(img, 0, 0, out_w, out_h);
        buff.close();
    }
    else if (bg_color!=Qt::white){
        page->setFillColor(bg_color.red(), bg_color.green(), bg_color.blue());
        page->drawRect(0,0, out_w, out_h, 1, FILL);
    }

    for (int i=0; i<collageItems.count(); i++)
    {
        CollageItem *item = collageItems.at(i);
        PdfObject *img;
        // Load as QImage, save to buffer as jpeg and then embed
        QBuffer buff;
        buff.open(QIODevice::WriteOnly);
        QImage image = item->originalImage();
        // set white background of transparent images
        if (image.format()==QImage::Format_ARGB32) {
            image = removeTransparency(image);
        }
        image.save(&buff, "JPG");
        img = doc.addImage(buff.data().data(), buff.size(), image.width(), image.height(), PDF_IMG_JPEG);
        buff.close();
        page->drawImage(img, item->x*scaleX, out_h - item->y*scaleY - item->h*scaleY, // img Y to pdf Y
                             item->w*scaleX, item->h*scaleY, item->rotation);
        if (item->border)
            page->drawRect(item->x*scaleX, out_h - item->y*scaleY - item->h*scaleY, // img Y to pdf Y
                             item->w*scaleX, item->h*scaleY, 0.3, STROKE);
    }
    doc.save(path.toStdString());
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
    double out_w_px = out_unit==UNIT_PIXEL ? out_w : out_dpi*out_w/72;
    double out_h_px = out_unit==UNIT_PIXEL ? out_h : out_dpi*out_h/72;
    CollageItem *item = collageItems.last();
    QString status;
    int item_w = item->w*out_w_px/paper.width();
    int item_h = item->h*out_h_px/paper.height();
    status.sprintf("Resolution : %dx%d", item_w, item_h);
    if (out_unit==UNIT_POINT) {
        float item_w_cm = 2.54*item_w/out_dpi;
        float item_h_cm = 2.54*item_h/out_dpi;
        status += QString().sprintf(",   Size : %.1fx%.1f cm", item_w_cm, item_h_cm);
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
CollageItem:: originalImage()
{
    if (not this->image_.isNull()) {
        return image_;
    }
    return loadImage(this->filename);
}

QImage
CollageItem:: image()
{
    if (rotation) {
        QTransform tfm;
        tfm.rotate(rotation);
        return originalImage().transformed(tfm);
    }
    return originalImage();
}

// ****************** Page Size Dialog ****************** //


PageSizeDialog:: PageSizeDialog(QWidget *parent, float w, float h, int unit) : QDialog(parent)
{
    this->resize(250, 120);
    this->setWindowTitle("Custom Page Size");
    QVBoxLayout *vLayout = new QVBoxLayout(this);
    QLabel *label = new QLabel("Set Page Size :", this);
    QWidget *widget = new QWidget(this);
    QHBoxLayout *hLayout = new QHBoxLayout(widget);
    QLabel *xLabel = new QLabel("x", widget);
    xLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    widthSpin = new QDoubleSpinBox(widget);
    heightSpin = new QDoubleSpinBox(widget);
    unitCombo = new QComboBox(this);
    QStringList items = { "cm", "in", "pt", "px" };
    unitCombo->addItems(items);
    hLayout->addWidget(widthSpin);
    hLayout->addWidget(xLabel);
    hLayout->addWidget(heightSpin);
    hLayout->addWidget(unitCombo);
    QDialogButtonBox *btnBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel, Qt::Horizontal, this);
    vLayout->addWidget(label);
    vLayout->addWidget(widget);
    vLayout->addWidget(btnBox);
    connect(unitCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(onUnitChange(int)));
    connect(btnBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(btnBox, SIGNAL(rejected()), this, SLOT(reject()));

    if (unit==UNIT_PIXEL)
        this->unitCombo->setCurrentIndex(3);
    else
        this->unitCombo->setCurrentIndex(2);
    widthSpin->setValue(w);
    heightSpin->setValue(h);
}

void
PageSizeDialog:: onUnitChange(int index)
{
    QList<int> minimum = {3, 1, 72, 100};
    QList<int> maximum = {100, 20, 1440, 8000};
    QList<int> precision = {1, 1, 0, 0};
    widthSpin->setDecimals(precision[index]);
    heightSpin->setDecimals(precision[index]);
    widthSpin->setRange(minimum[index], maximum[index]);
    heightSpin->setRange(minimum[index], maximum[index]);
}

void
PageSizeDialog:: accept()
{
    QList<float> factors = {72/2.54, 72, 1, 1}; // other unit to point conversion factor
    float factor = factors[unitCombo->currentIndex()];
    out_w = widthSpin->value()*factor;
    out_h = heightSpin->value()*factor;
    out_unit = unitCombo->currentIndex()==3 ? UNIT_PIXEL : UNIT_POINT;
    QDialog::accept();
}

