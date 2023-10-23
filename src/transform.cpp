/*
This file is a part of photoquick program, which is GPLv3 licensed
*/
#include "transform.h"
#include "common.h"
#include "filters.h"
#include <QSettings>
#include <QPainter>
#include <QPushButton>
#include <QMenu>
#include <QRect>
#include <cmath>


// ******************************************************************* |
//                         Crop Manager
// ------------------------------------------------------------------- |

// calculate adaptive corner_drag_box size
int dragBoxWidth(QPoint p1, QPoint p2) {
    int drag_box_w = MAX(p2.x()-p1.x(), p2.y()-p1.y())/5;
    return MIN(50, MAX(16, drag_box_w) );
}

Crop:: Crop(Canvas *canvas, QStatusBar *statusbar) : QObject(canvas),
                                canvas(canvas), statusbar(statusbar)
{
    mouse_pressed = false;
    canvas->drag_to_scroll = false;
    pixmap = canvas->pixmap()->copy();
    scaleX = float(pixmap.width())/canvas->data->image.width();
    scaleY = float(pixmap.height())/canvas->data->image.height();
    topleft = QPoint(0,0);
    btmright = QPoint(pixmap.width()-1, pixmap.height()-1);
    p1 = QPoint(topleft);   // temporary topleft point
    p2 = QPoint(btmright);
    ratio_w = 3.5;
    ratio_h = 4.5;
    crop_mode = NO_RATIO;
    drag_box_w = dragBoxWidth(p1, p2);
    // add buttons
    QPushButton *setRatioBtn = new QPushButton("Set Ratio", statusbar);
    statusbar->addPermanentWidget(setRatioBtn);
    QMenu *ratioMenu = new QMenu(setRatioBtn);
    QActionGroup *ratioActions = new QActionGroup(ratioMenu);
    QStringList ratio_names = {"No Ratio", "Custom Ratio", "Square - 1:1",
            "Photo 4R - 4x6", "Stamp - 2.0x2.5", "Passport - 3.5x4.5", "Fixed Resolution"};
    for (QString title : ratio_names){
        QAction *action = ratioActions->addAction(title);
        action->setCheckable(true);
        ratioMenu->addAction(action);
    }
    ratioActions->actions()[0]->setChecked(true);
    setRatioBtn->setMenu(ratioMenu);
    connect(ratioActions, SIGNAL(triggered(QAction*)), this, SLOT(setCropMode(QAction*)));
    QWidget *spacer = new QWidget(statusbar);
    spacer->setMinimumWidth(40);
    statusbar->addPermanentWidget(spacer);
    QPushButton *cropnowBtn = new QPushButton("Crop Now", statusbar);
    statusbar->addPermanentWidget(cropnowBtn);
    QPushButton *cropcancelBtn = new QPushButton("Cancel", statusbar);
    statusbar->addPermanentWidget(cropcancelBtn);
    connect(cropnowBtn, SIGNAL(clicked()), this, SLOT(crop()));
    connect(cropcancelBtn, SIGNAL(clicked()), this, SLOT(finish()));
    crop_widgets << setRatioBtn << spacer << cropnowBtn << cropcancelBtn;
    drawCropBox();
}

void
Crop:: onMousePress(QPoint pos)
{
    clk_pos = pos;
    mouse_pressed = true;
    // Determine which position is clicked
    if (QRect(topleft, QSize(drag_box_w, drag_box_w)).contains(clk_pos))
        clk_area = 1;   // Topleft is clicked
    else if (QRect(btmright, QSize(-drag_box_w, -drag_box_w)).contains(clk_pos))
        clk_area = 2;   // bottom right corner clicked
    else if (QRect(topleft, btmright).contains(clk_pos)) // clicked inside cropbox
        clk_area = 3;   // inside cropbox
    else
        clk_area = 0;   // outside cropbox
}

void
Crop:: onMouseRelease(QPoint /*pos*/)
{
    mouse_pressed = false;
    topleft = p1;
    btmright = p2;
    drag_box_w = dragBoxWidth(p1, p2);
    drawCropBox();
}

void
Crop:: onMouseMove(QPoint pos)
{
    if (not mouse_pressed) return;
    QPoint moved = pos - clk_pos;
    QPoint last_pt = QPoint(pixmap.width()-1, pixmap.height()-1);
    float imgAspect = float(canvas->data->image.width())/canvas->data->image.height();
    float boxAspect = ratio_w/ratio_h;
    switch (clk_area) {
    case 1 : { // Top left corner is clicked
        if (crop_mode==FIXED_RESOLUTION)
            break;
        QPoint new_p1 = topleft + moved;
        p1 = QPoint(MAX(0, new_p1.x()), MAX(0, new_p1.y()));
        if (crop_mode==FIXED_RATIO) {
            if (imgAspect>boxAspect) p1.setX(round(p2.x() - (p2.y()-p1.y()+1)*boxAspect -1));
            else p1.setY(round(p2.y() - (p2.x()-p1.x()+1)/boxAspect -1));
        }
        break;
    }
    case 2 : { // Bottom right corner is clicked
        if (crop_mode==FIXED_RESOLUTION)
            break;
        QPoint new_p2 = btmright + moved;
        p2 = QPoint(MIN(last_pt.x(), new_p2.x()), MIN(last_pt.y(), new_p2.y()));
        if (crop_mode==FIXED_RATIO) {
            if (imgAspect>boxAspect) p2.setX(round(p1.x() + (p2.y()-p1.y()+1)*boxAspect -1));
            else p2.setY(round(p1.y() + (p2.x()-p1.x()+1)/boxAspect -1));
        }
        break;
    }
    case 3 : { // clicked inside cropbox but none of the corner selected.
        int min_dx, max_dx, min_dy, max_dy, dx, dy;
        min_dx = -topleft.x();
        max_dx = last_pt.x()-btmright.x();
        min_dy = -topleft.y();
        max_dy = last_pt.y()-btmright.y();
        dx = (moved.x() < 0) ? MAX(moved.x(), min_dx) : MIN(moved.x(), max_dx);
        dy = (moved.y() < 0) ? MAX(moved.y(), min_dy) : MIN(moved.y(), max_dy);
        p1 = topleft + QPoint(dx, dy);
        p2 = btmright + QPoint(dx, dy);
        break;
    }
    }
    drawCropBox();
}

void
Crop:: drawCropBox()
{
    QPixmap pm = pixmap.copy();
    QPixmap pm_box = pm.copy(p1.x(), p1.y(), p2.x()-p1.x(), p2.y()-p1.y());
    QPainter painter(&pm);
    painter.fillRect(0,0, pm.width(), pm.height(), QColor(127,127,127,127));
    painter.drawPixmap(p1.x(), p1.y(), pm_box);
    painter.drawRect(p1.x(), p1.y(), p2.x()-p1.x(), p2.y()-p1.y());
    // draw corner drag box
    painter.drawRect(p1.x(), p1.y(), drag_box_w-1, drag_box_w-1);
    painter.drawRect(p2.x(), p2.y(), -drag_box_w+1, -drag_box_w+1);
    painter.setPen(Qt::white);
    painter.drawRect(p1.x()+1, p1.y()+1, drag_box_w-3, drag_box_w-3);
    painter.drawRect(p2.x()-1, p2.y()-1, -drag_box_w+3, -drag_box_w+3);
    painter.end();
    canvas->setPixmap(pm);
    int w, h;
    if (crop_mode==FIXED_RESOLUTION){
        w = fixed_width;
        h = fixed_height;
    }
    else {
        w = round((p2.x() - p1.x() + 1)/scaleX);
        h = round((p2.y() - p1.y() + 1)/scaleY);
    }
    int x = round(p1.x()/scaleX);
    int y = round(p1.y()/scaleY);
    statusbar->showMessage(QString("CropBox : %1,%2 - %3x%4").arg(x).arg(y).arg(w).arg(h));
}

void
Crop:: crop()
{
    int w,h;
    if (crop_mode==FIXED_RESOLUTION) {
        w = fixed_width;
        h = fixed_height;
    }
    else {
        w = round((btmright.x()-topleft.x()+1)/scaleX);
        h = round((btmright.y()-topleft.y()+1)/scaleY);
    }
    QImage img = canvas->data->image.copy(round(topleft.x()/scaleX), round(topleft.y()/scaleY), w, h);
    canvas->data->image = img;
    finish();
}

void
Crop:: finish()
{
    canvas->showScaled();
    canvas->drag_to_scroll = true;
    // remove buttons
    while (not crop_widgets.isEmpty()) {
        QWidget *widget = crop_widgets.takeLast();
        statusbar->removeWidget(widget);
        widget->deleteLater();
    }
    emit finished();
    this->deleteLater();
}

void
Crop:: setCropMode(QAction *action)
{
    if (action->text()==QString("Fixed Resolution")) {
        CropResolutionDialog *res_dlg = new CropResolutionDialog(canvas,canvas->data->image.width(),canvas->data->image.height());
        res_dlg->exec();
        fixed_width = res_dlg->widthSpin->value();
        fixed_height = res_dlg->heightSpin->value();
        p1 = topleft = QPoint(0,0);
        p2 = btmright = QPoint( round((fixed_width-1)*scaleX), round((fixed_height-1)*scaleY) );
        crop_mode = FIXED_RESOLUTION;
    }
    else if (action->text()==QString("Custom Ratio")) {
        QSettings settings;
        ratio_w = settings.value("CropRatioX", 3.5).toFloat();
        ratio_h = settings.value("CropRatioY", 4.5).toFloat();
        CropRatioDialog *dlg = new CropRatioDialog(canvas, ratio_w, ratio_h);
        dlg->exec();
        ratio_w = dlg->widthSpin->value();
        ratio_h = dlg->heightSpin->value();
        settings.setValue("CropRatioX", ratio_w);
        settings.setValue("CropRatioY", ratio_h);
        crop_mode = FIXED_RATIO;
    }
    else if (action->text()==QString("Square - 1:1")) {
        ratio_w = 1.0;
        ratio_h = 1.0;
        crop_mode = FIXED_RATIO;
    }
    else if (action->text()==QString("Photo 4R - 4x6")) {
        ratio_w = 2.0;
        ratio_h = 3.0;
        crop_mode = FIXED_RATIO;
    }
    else if (action->text()==QString("Passport - 3.5x4.5")) {
        ratio_w = 3.5;
        ratio_h = 4.5;
        crop_mode = FIXED_RATIO;
    }
    else if (action->text()==QString("Stamp - 2.0x2.5")) {
        ratio_w = 2.0;
        ratio_h = 2.5;
        crop_mode = FIXED_RATIO;
    }
    else
        crop_mode = NO_RATIO;
    // draw crop box in selected ratio
    if (crop_mode == FIXED_RATIO) {
        int w = p2.x()-p1.x()+1;
        int h = p2.y()-p1.y()+1;
        float aspect = w/(float)h;
        float newAspect = ratio_w/ratio_h;
        if (aspect>newAspect)
            p2 = QPoint(p1.x()+ h*newAspect-1, p2.y());
        else
            p2 = QPoint(p2.x(), p1.y()+ w/newAspect -1);
        btmright = p2;
    }
    drawCropBox();
}


CropRatioDialog::
CropRatioDialog(QWidget *parent, double w, double h) : QDialog(parent)
{
    setWindowTitle("Enter Ratio");
    resize(200, 50);
    widthSpin = new QDoubleSpinBox(this);
    widthSpin->setAlignment(Qt::AlignHCenter);
    widthSpin->setDecimals(1);
    widthSpin->setSingleStep(0.1);
    widthSpin->setRange(0.1, 9.9);
    widthSpin->setValue(w);
    heightSpin = new QDoubleSpinBox(this);
    heightSpin->setAlignment(Qt::AlignHCenter);
    heightSpin->setDecimals(1);
    heightSpin->setSingleStep(0.1);
    heightSpin->setRange(0.1, 9.9);
    heightSpin->setValue(h);
    QGridLayout *layout = new QGridLayout(this);
    QLabel *label1 = new QLabel("Width : Height", this);
    QDialogButtonBox *btnBox = new QDialogButtonBox(QDialogButtonBox::Ok, Qt::Horizontal, this);
    layout->addWidget(label1, 0,0,1,2);
    layout->addWidget(widthSpin, 1,0,1,1);
    layout->addWidget(heightSpin, 1,1,1,1);
    layout->addWidget(btnBox, 2,0,1,2);
    connect(btnBox, SIGNAL(accepted()), this, SLOT(accept()));
}

CropResolutionDialog::
CropResolutionDialog(QWidget *parent, int w, int h) : QDialog(parent)
{
    setWindowTitle("Enter Ratio");
    resize(200, 50);
    widthSpin = new QSpinBox(this);
    widthSpin->setAlignment(Qt::AlignHCenter);
    widthSpin->setRange(1, w);
    widthSpin->setValue(w);
    heightSpin = new QSpinBox(this);
    heightSpin->setAlignment(Qt::AlignHCenter);
    heightSpin->setRange(1, h);
    heightSpin->setValue(h);
    QGridLayout *layout = new QGridLayout(this);
    QLabel *label1 = new QLabel("Width", this);
    QLabel *label2 = new QLabel("Height", this);
    QDialogButtonBox *btnBox = new QDialogButtonBox(QDialogButtonBox::Ok, Qt::Horizontal, this);
    layout->addWidget(label1, 0,0,1,1);
    layout->addWidget(label2, 0,1,1,1);
    layout->addWidget(widthSpin, 1,0,1,1);
    layout->addWidget(heightSpin, 1,1,1,1);
    layout->addWidget(btnBox, 2,0,1,2);
    connect(btnBox, SIGNAL(accepted()), this, SLOT(accept()));
}

// ******************************************************************* |
//                         Perspective Transform
// ------------------------------------------------------------------- |
PerspectiveTransform::
PerspectiveTransform(Canvas *canvas, QStatusBar *statusbar) : QObject(canvas),
                                canvas(canvas), statusbar(statusbar)
{
    mouse_pressed = false;
    canvas->drag_to_scroll = false;
    pixmap = canvas->pixmap()->copy();
    scaleX = float(pixmap.width())/canvas->data->image.width();
    scaleY = float(pixmap.height())/canvas->data->image.height();
    p1 = topleft = QPoint(0,0);
    p2 = topright = QPoint(pixmap.width()-1, 0);
    p3 = btmleft = QPoint(0, pixmap.height()-1);
    p4 = btmright = QPoint(pixmap.width()-1, pixmap.height()-1);
    // add buttons
    cropCheckBox = new QCheckBox("Crop", statusbar);
    statusbar->addPermanentWidget(cropCheckBox);
    QPushButton *untiltBtn = new QPushButton("Untilt", statusbar);
    statusbar->addPermanentWidget(untiltBtn);
    QPushButton *cancelBtn = new QPushButton("Cancel", statusbar);
    statusbar->addPermanentWidget(cancelBtn);
    connect(untiltBtn, SIGNAL(clicked()), this, SLOT(transform()));
    connect(cancelBtn, SIGNAL(clicked()), this, SLOT(finish()));
    crop_widgets << cropCheckBox << untiltBtn << cancelBtn;
    QSettings settings;
    bool crop_on_untilt = settings.value("UntiltCrop", true).toBool();
    cropCheckBox->setChecked(crop_on_untilt);

    statusbar->showMessage("Drag corners to fit edges around tilted image");
    drawCropBox();
}

void
PerspectiveTransform:: onMousePress(QPoint pos)
{
    clk_pos = pos;
    mouse_pressed = true;
    // Determine which position is clicked
    if (QRect(topleft, QSize(60, 60)).contains(clk_pos))
        clk_area = 1;   // Topleft is clicked
    else if (QRect(topright, QSize(-60, 60)).contains(clk_pos))
        clk_area = 2;   // Topright is clicked
    else if (QRect(btmleft, QSize(60, -60)).contains(clk_pos))
        clk_area = 3;   // Bottomleft is clicked
    else if (QRect(btmright, QSize(-60, -60)).contains(clk_pos))
        clk_area = 4;   // bottom right corner clicked
    else
        clk_area = 0;
}

void
PerspectiveTransform:: onMouseRelease(QPoint /*pos*/)
{
    mouse_pressed = false;
    topleft = p1;
    topright = p2;
    btmleft = p3;
    btmright = p4;
}

void
PerspectiveTransform:: onMouseMove(QPoint pos)
{
    if (not mouse_pressed) return;
    QPoint moved = pos - clk_pos;
    QPoint last_pt = QPoint(pixmap.width()-1, pixmap.height()-1);
    QPoint new_pt;
    switch (clk_area) {
    case 1 : { // Top left corner is clicked
        new_pt = topleft + moved;
        p1 = QPoint(MAX(0, new_pt.x()), MAX(0, new_pt.y()));
        break;
    }
    case 2 : { // Top right corner is clicked
        new_pt = topright + moved;
        p2 = QPoint(MIN(last_pt.x(), new_pt.x()), MAX(0, new_pt.y()));
        break;
    }
    case 3 : { // Bottom left corner is clicked
        new_pt = btmleft + moved;
        p3 = QPoint(MAX(0, new_pt.x()), MIN(last_pt.y(), new_pt.y()));
        break;
    }
    case 4 : { // Bottom right corner is clicked
        QPoint new_pt = btmright + moved;
        p4 = QPoint(MIN(last_pt.x(), new_pt.x()), MIN(last_pt.y(), new_pt.y()));
        break;
    }
    default:
        break;
    }
    drawCropBox();
}

void
PerspectiveTransform:: drawCropBox()
{
    QPixmap pm = pixmap.copy();
    QPainter painter(&pm);
    QPolygonF polygon;
    polygon << p1<< p2<< p4<< p3;
    painter.drawPolygon(polygon);
    float start, span;
    calcArc(p1, p2, p3, p4, start, span);
    painter.drawArc(p1.x()-30, p1.y()-30, 60,60, 16*start, 16*span);
    calcArc(p2, p1, p4, p3, start, span);
    painter.drawArc(p2.x()-30, p2.y()-30, 60,60, 16*start, 16*span);
    calcArc(p3, p4, p1, p2, start, span);
    painter.drawArc(p3.x()-30, p3.y()-30, 60,60, 16*start, 16*span);
    calcArc(p4, p2, p3, p1, start, span);
    painter.drawArc(p4.x()-30, p4.y()-30, 60,60, 16*start, 16*span);
    painter.setPen(Qt::white);
    polygon.clear();
    polygon<< p1+QPoint(1,1)<< p2+QPoint(-1,1)<< p4+QPoint(-1,-1)<< p3+QPoint(1,-1);
    painter.drawPolygon(polygon);
    painter.end();
    canvas->setPixmap(pm);
}

void
PerspectiveTransform:: transform()
{
    p1 = QPoint(p1.x()/scaleX, p1.y()/scaleY);
    p2 = QPoint(p2.x()/scaleX, p2.y()/scaleY);
    p3 = QPoint(p3.x()/scaleX, p3.y()/scaleY);
    p4 = QPoint(p4.x()/scaleX, p4.y()/scaleY);
    int w1 = sqrt(SQR(p2.x()-p1.x()) + SQR(p2.y()-p1.y()));
    int w2 = sqrt(SQR(p4.x()-p3.x()) + SQR(p4.y()-p3.y()));
    int h1 = sqrt(SQR(p3.x()-p1.x()) + SQR(p3.y()-p1.y()));
    int h2 = sqrt(SQR(p4.x()-p2.x()) + SQR(p4.y()-p2.y()));
    int max_w = MAX(w1, w2);
    int max_h = MAX(h1, h2);
    QPolygonF mapFrom;
    mapFrom << p1<< p2<< p3<< p4;
    QPolygonF mapTo;
    mapTo << QPointF(0,0)<< QPointF(max_w,0)<< QPointF(0,max_h)<< QPointF(max_w,max_h);
    QTransform tfm;
    QTransform::quadToQuad(mapFrom, mapTo, tfm);
    QImage img = canvas->data->image.transformed(tfm, Qt::SmoothTransformation);

    if (cropCheckBox->isChecked()) {
        QTransform trueMtx = QImage::trueMatrix(tfm,canvas->data->image.width(),canvas->data->image.height());
        topleft = trueMtx.map(p1);
        btmright = trueMtx.map(p4);
        canvas->data->image = img.copy(QRect(topleft, btmright));
    }
    else {
        QImage new_img(img.width(), img.height(), QImage::Format_RGB32);
        new_img.fill(borderAverageForTransperant(img));
        QPainter painter(&new_img);
        painter.drawImage(0,0, img);
        painter.end();
        canvas->data->image = new_img;
    }

    finish();
}

void
PerspectiveTransform:: finish()
{
    canvas->showScaled();
    canvas->drag_to_scroll = true;
    QSettings settings;
    settings.setValue("UntiltCrop", cropCheckBox->isChecked());
    // remove buttons
    while (not crop_widgets.isEmpty()) {
        QWidget *widget = crop_widgets.takeLast();
        statusbar->removeWidget(widget);
        widget->deleteLater();
    }
    emit finished();
    this->deleteLater();
}

// arc is drawn from p1 to p2 through p3
// p3 is at diagonal corner of p
// angle is  drawn counter clock-wise, and direction of y axis is
// upward, while direction of image y axis is downward
void calcArc(QPoint p/*center*/, QPoint p1, QPoint p2, QPoint p3,
                                    float &start, float &span)
{
    float x, ang1, ang2, ang3;
    x = (p.x()==0)? p.x()+1.0e-7: p.x();    // avoid zero division error
    ang1 = atan((p.y()-p1.y())/(p1.x()-x))*180/3.14159265;
    ang1 = (x>p1.x()) ? ang1+180 : ang1;
    ang1 = ang1<0 ? ang1+360: ang1;
    ang2 = atan((p.y()-p2.y())/(p2.x()-x))*180/3.14159265;
    ang2 = (x>p2.x()) ? ang2+180 : ang2;
    ang2 = ang2<0 ? ang2+360: ang2;
    ang3 = atan((p.y()-p3.y())/(p3.x()-x))*180/3.14159265;
    ang3 = (x>p3.x()) ? ang3+180 : ang3;
    ang3 = ang3<0 ? ang3+360: ang3;
    if (ang1 > ang2) {
        float tmp = ang1;
        ang1 = ang2;
        ang2 = tmp;
    }
    if (ang1<ang3 and ang3<ang2) {
        start = ang1;
        span = ang2-ang1;
    }
    else {
        start = ang2;
        span = 360 - (ang2-ang1);
    }
}


// *********************************************************************
//                              Resize Manager
// _____________________________________________________________________

// ResizeDialog object to get required image size
ResizeDialog:: ResizeDialog(QWidget *parent, int img_width, int img_height) : QDialog(parent)
{
    setupUi(this);
    frame->hide();
    resize(350, 100);
    QIntValidator validator(this);
    widthEdit->setValidator(&validator);
    heightEdit->setValidator(&validator);
    spinWidth->setValue(img_width*2.54/300);
    spinHeight->setValue(img_height*2.54/300);
    QObject::connect(checkBox, SIGNAL(toggled(bool)), this, SLOT(toggleAdvanced(bool)));
    QObject::connect(spinWidth, SIGNAL(valueChanged(double)), this, SLOT(onValueChange(double)));
    QObject::connect(spinHeight, SIGNAL(valueChanged(double)), this, SLOT(onValueChange(double)));
    QObject::connect(spinDPI, SIGNAL(valueChanged(int)), this, SLOT(onValueChange(int)));
    widthEdit->setFocus();
}

void
ResizeDialog:: toggleAdvanced(bool checked)
{
    if (checked)
        frame->show();
    else {
        frame->hide();
        waitFor(50);
        resize(350, 100);
    }
}

void
ResizeDialog:: onValueChange(int)
{
    int DPI = spinDPI->value();
    widthEdit->setText( QString::number(round(DPI * spinWidth->value()/2.54)));
    heightEdit->setText( QString::number(round(DPI * spinHeight->value()/2.54)));
}



// *********************************************************************
//                              Aspect Ratio
// _____________________________________________________________________

AspectRatioDialog:: AspectRatioDialog(QWidget *parent) : QDialog(parent)
{
    this->resize(200, 100);
    this->setWindowTitle("Scale to Ratio");
    QGridLayout *layout = new QGridLayout(this);
    QLabel *label = new QLabel("Enter Aspect Ratio :", this);
    widthEdit = new QLineEdit(this);
    widthEdit->setPlaceholderText("Width");
    heightEdit = new QLineEdit(this);
    heightEdit->setPlaceholderText("Height");
    QDoubleValidator *validator = new QDoubleValidator(this);
    widthEdit->setValidator(validator);
    heightEdit->setValidator(validator);
    QLabel *label2 = new QLabel(" : ", this);
    QDialogButtonBox *btnBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel, Qt::Horizontal, this);
    layout->addWidget(label, 0,0, 1,3);
    layout->addWidget(widthEdit, 1,0, 1,1);
    layout->addWidget(label2, 1,1, 1,1);
    layout->addWidget(heightEdit, 1,2, 1,1);
    layout->addWidget(btnBox, 2,0, 1,3);
    connect(btnBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(btnBox, SIGNAL(rejected()), this, SLOT(reject()));
}

QImage
AspectRatioDialog:: getResult(QImage img)
{
    // return in case of empty or zero value
    if (widthEdit->text().toFloat()==0 or heightEdit->text().toFloat()==0)
        return img;
    int w = img.width();
    int h = img.height();
    float new_aspect = widthEdit->text().toFloat() / heightEdit->text().toFloat();
    float orig_aspect = (float)w / h;
    if (new_aspect > orig_aspect){
        return img.scaled(h*new_aspect, h, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }
    else if (new_aspect < orig_aspect){
        return img.scaled(w, w/new_aspect, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }
    return img;
}
