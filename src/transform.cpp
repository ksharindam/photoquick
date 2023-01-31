/*
This file is a part of photoquick program, which is GPLv3 licensed
*/
#include "transform.h"

// ******************************************************************* |
//                         Crop Manager
// ------------------------------------------------------------------- |

// calculate adaptive corner_drag_box size
int dragBoxWidth(QPoint p1, QPoint p2, float scaleX, float scaleY)
{
    int drag_box_w, dx, dy;
    
    dx = (p2.x() > p1.x()) ? (p2.x() - p1.x()) : (p1.x() - p2.x());
    dy = (p2.y() > p1.y()) ? (p2.y() - p1.y()) : (p1.y() - p2.y());
    dx *= scaleX;
    dy *= scaleY;
    drag_box_w = MAX(dx, dy) / 5;
    drag_box_w = MIN(50, MAX(16, drag_box_w));

    return drag_box_w;
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
    btmright = QPoint(canvas->data->image.width(), canvas->data->image.height());
    p1 = QPoint(topleft);   // temporary topleft point
    p2 = QPoint(btmright);
    ratio_w = 3.5;
    ratio_h = 4.5;
    crop_mode = NO_RATIO;
    drag_box_w = dragBoxWidth(p1, p2, scaleX, scaleY);
    // add buttons
    cropinfoBtn = new QPushButton("Info", statusbar);
    statusbar->addPermanentWidget(cropinfoBtn);
    stepLabel = new QLabel("Step :", statusbar);
    statusbar->addPermanentWidget(stepLabel);
    stepSpin = new QSpinBox(statusbar);
    stepSpin->setRange(1, 256);
    stepSpin->setValue(1);
    statusbar->addPermanentWidget(stepSpin);
    setRatioBtn = new QPushButton("Set Ratio", statusbar);
    statusbar->addPermanentWidget(setRatioBtn);
    ratioMenu = new QMenu(setRatioBtn);
    ratioActions = new QActionGroup(ratioMenu);
    action1 = ratioActions->addAction("No Ratio");
    action2 = ratioActions->addAction("Ratio - Custom");
    action3 = ratioActions->addAction("Ratio - 1:1");
    action4 = ratioActions->addAction("Ratio - 3:4");
    action5 = ratioActions->addAction("Ratio - 2.5:3.5");
    action6 = ratioActions->addAction("Ratio - 3.5:4.5");
    action7 = ratioActions->addAction("Fixed Resolution");
    action1->setCheckable(true);
    action2->setCheckable(true);
    action3->setCheckable(true);
    action4->setCheckable(true);
    action5->setCheckable(true);
    action6->setCheckable(true);
    action7->setCheckable(true);
    action1->setChecked(true);
    ratioMenu->addAction(action1);
    ratioMenu->addAction(action2);
    ratioMenu->addAction(action3);
    ratioMenu->addAction(action4);
    ratioMenu->addAction(action5);
    ratioMenu->addAction(action6);
    ratioMenu->addAction(action7);
    setRatioBtn->setMenu(ratioMenu);
    connect(ratioActions, SIGNAL(triggered(QAction*)), this, SLOT(setCropMode(QAction*)));
    spacer = new QWidget(statusbar);
    spacer->setMinimumWidth(40);
    statusbar->addPermanentWidget(spacer);
    cropnowBtn = new QPushButton("Crop Now", statusbar);
    statusbar->addPermanentWidget(cropnowBtn);
    cropcancelBtn = new QPushButton("Cancel", statusbar);
    statusbar->addPermanentWidget(cropcancelBtn);
    connect(cropinfoBtn, SIGNAL(clicked()), this, SLOT(cropinfo()));
    connect(cropnowBtn, SIGNAL(clicked()), this, SLOT(crop()));
    connect(cropcancelBtn, SIGNAL(clicked()), this, SLOT(finish()));
    crop_widgets << cropinfoBtn << stepLabel << stepSpin << setRatioBtn << spacer << cropnowBtn << cropcancelBtn;
    drawCropBox();
}

void
Crop:: onMousePress(QPoint pos)
{
    int r = drag_box_w;
    clk_pos = pos;
    mouse_pressed = true;
    // Determine which position is clicked
    if (QRect(topleft * scaleX, QSize(r, r)).contains(clk_pos))
        clk_area = 1;   // Topleft is clicked
    else if (QRect(btmright * scaleX, QSize(-r, -r)).contains(clk_pos))
        clk_area = 2;   // bottom right corner clicked
    else if (QRect(topleft * scaleX, btmright * scaleX).contains(clk_pos)) // clicked inside cropbox
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
    drag_box_w = dragBoxWidth(p1, p2, scaleX, scaleY);
    drawCropBox();
}

void
Crop:: onMouseMove(QPoint pos)
{
    int min_dx, max_dx, min_dy, max_dy, dx, dy, step;
    float imgAspect, boxAspect;
    QPoint moved, last_pt, new_p1, new_p2;

    if (not mouse_pressed) return;

    step = stepSpin->value();
    moved = pos - clk_pos;
    moved /= scaleX;
    last_pt = QPoint(canvas->data->image.width(), canvas->data->image.height());
    imgAspect = float(canvas->data->image.width())/canvas->data->image.height();
    boxAspect = ratio_w/ratio_h;
    switch (clk_area)
    {
        case 1 :
        { // Top left corner is clicked
            if (crop_mode==FIXED_RESOLUTION)
                break;
            new_p1 = topleft + moved;
            if ((new_p1.x() < p2.x()) && (new_p1.y() < p2.y()))
            {
                p1 = QPoint(MAX(0, new_p1.x()), MAX(0, new_p1.y()));
                if (crop_mode==FIXED_RATIO)
                {
                    if (imgAspect>boxAspect) p1.setX(round(p2.x() - (p2.y()-p1.y()+1)*boxAspect -1));
                    else p1.setY(round(p2.y() - (p2.x()-p1.x()+1)/boxAspect -1));
                }
            }
            break;
        }
        case 2 :
        { // Bottom right corner is clicked
            if (crop_mode==FIXED_RESOLUTION)
                break;
            new_p2 = btmright + moved;
            if ((p1.x() < new_p2.x()) && (p1.y() < new_p2.y()))
            {
                p2 = QPoint(MIN(last_pt.x(), new_p2.x()), MIN(last_pt.y(), new_p2.y()));
                if (crop_mode==FIXED_RATIO)
                {
                    if (imgAspect>boxAspect) p2.setX(round(p1.x() + (p2.y()-p1.y()+1)*boxAspect -1));
                    else p2.setY(round(p1.y() + (p2.x()-p1.x()+1)/boxAspect -1));
                }
            }
            break;
        }
        case 3 :
        { // clicked inside cropbox but none of the corner selected.
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
    if (step > 1)
    {
        p1 /= step;
        p2 /= step;
        p1 *= step;
        p2 *= step;
    }
    drawCropBox();
}

void
Crop:: drawCropBox()
{
    int r1, r3, width, height, left, top;
    QPoint p1t, p2t;
    QPixmap pm, pm_box;

    p1t = p1 * scaleX;
    p2t = (p2 - QPoint(1,1)) * scaleX;
    r1 = drag_box_w - 1;
    r3 = drag_box_w - 3;
    pm = pixmap.copy();
    pm_box = pm.copy(p1t.x(), p1t.y(), p2t.x()-p1t.x(), p2t.y()-p1t.y());
    QPainter painter(&pm);
    painter.fillRect(0,0, pm.width(), pm.height(), QColor(127,127,127,127));
    painter.drawPixmap(p1t.x(), p1t.y(), pm_box);
    painter.drawRect(p1t.x(), p1t.y(), p2t.x()-p1t.x(), p2t.y()-p1t.y());
    painter.drawRect(p1t.x(), p1t.y(), r1, r1);
    painter.drawRect(p2t.x(), p2t.y(), -r1, -r1);
    painter.setPen(Qt::white);
    painter.drawRect(p1t.x()+1, p1t.y()+1, r3, r3);
    painter.drawRect(p2t.x()-1, p2t.y()-1, -r3, -r3);
    painter.end();
    canvas->setPixmap(pm);
    QString text = "Resolution : %1x%2+%3+%4";
    if (crop_mode==FIXED_RESOLUTION)
    {
        left = topleft.x();
        top = topleft.y();
        statusbar->showMessage(text.arg(fixed_width).arg(fixed_height).arg(left).arg(top));
    }
    else
    {
        left = p1.x();
        top = p1.y();
        width = (p2.x() - left);
        height = (p2.y() - top);
        statusbar->showMessage(text.arg(width).arg(height).arg(left).arg(top));
    }
}

void
Crop:: cropinfo()
{
    int h, w, l, t, r, b, width, height;

    width = canvas->data->image.width();
    height = canvas->data->image.height();
    l = p1.x();
    t = p1.y();
    w = (p2.x() - l);
    h = (p2.y() - t);
    r = (width - p2.x());
    b = (height - p2.y());
    CropInfoDialog *res_dlg = new CropInfoDialog(canvas, w, h, l, t, r, b);
    res_dlg->exec();
}

void
Crop:: crop()
{
    int w, h;
    if (crop_mode==FIXED_RESOLUTION)
    {
        w = fixed_width;
        h = fixed_height;
    }
    else
    {
        w = btmright.x() - topleft.x();
        h = btmright.y() - topleft.y();
    }
    QImage img = canvas->data->image.copy(topleft.x(), topleft.y(), w, h);
    canvas->data->image = img.copy();
    finish();
}

void
Crop:: finish()
{
    canvas->showScaled();
    canvas->drag_to_scroll = true;
    // remove buttons
    while (not crop_widgets.isEmpty())
    {
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
    if (action->text()==QString("Fixed Resolution"))
    {
        CropResolutionDialog *res_dlg = new CropResolutionDialog(canvas,canvas->data->image.width(),canvas->data->image.height());
        res_dlg->exec();
        fixed_width = res_dlg->widthSpin->value();
        fixed_height = res_dlg->heightSpin->value();
        p1 = topleft = QPoint(0,0);
        p2 = btmright = QPoint(fixed_width, fixed_height);
        crop_mode = FIXED_RESOLUTION;
    }
    else if (action->text()==QString("Ratio - Custom"))
    {
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
    else if (action->text()==QString("Ratio - 3.5:4.5"))
    {
        ratio_w = 3.5;
        ratio_h = 4.5;
        crop_mode = FIXED_RATIO;
    }
    else if (action->text()==QString("Ratio - 2.5:3.5"))
    {
        ratio_w = 2.5;
        ratio_h = 3.5;
        crop_mode = FIXED_RATIO;
    }
    else if (action->text()==QString("Ratio - 3:4"))
    {
        ratio_w = 3.0;
        ratio_h = 4.0;
        crop_mode = FIXED_RATIO;
    }
    else if (action->text()==QString("Ratio - 1:1"))
    {
        ratio_w = 1.0;
        ratio_h = 1.0;
        crop_mode = FIXED_RATIO;
    }
    else
        crop_mode = NO_RATIO;
    // draw crop box in selected ratio
    if (crop_mode == FIXED_RATIO)
    {
        int w = p2.x() - p1.x();
        int h = p2.y() - p1.y();
        float aspect = (float)w / (float)h;
        float newAspect = ratio_w / ratio_h;
        if (aspect>newAspect)
            p2 = QPoint(p1.x()+ h * newAspect, p2.y());
        else
            p2 = QPoint(p2.x(), p1.y()+ w / newAspect);
        btmright = p2;
    }
    drawCropBox();
}

CropInfoDialog::
CropInfoDialog(QWidget *parent, int w, int h, int l, int t, int r, int b) : QDialog(parent)
{
    QString strBox = QString().sprintf("%dx%d+%d+%d", w, h, l, t);
    QString strMargin = QString().sprintf("%d,%d,%d,%d", l, t, r, b);
    setWindowTitle("Info");
    resize(300, 40);
    layout = new QGridLayout(this);
    textBox = new QTextEdit(this);
    textBox->setReadOnly(true);
    textBox->setPlainText(strBox);
    textMargin = new QTextEdit(this);
    textMargin->setReadOnly(true);
    textMargin->setPlainText(strMargin);
    btnBox = new QDialogButtonBox(QDialogButtonBox::Ok, Qt::Horizontal, this);
    layout->addWidget(textBox, 0,0,1,1);
    layout->addWidget(textMargin, 1,0,1,1);
    layout->addWidget(btnBox, 2,0,1,2);
    connect(btnBox, SIGNAL(accepted()), this, SLOT(accept()));
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
    layout = new QGridLayout(this);
    label1 = new QLabel("Width : Height", this);
    btnBox = new QDialogButtonBox(QDialogButtonBox::Ok, Qt::Horizontal, this);
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
    layout = new QGridLayout(this);
    label1 = new QLabel("Width", this);
    label2 = new QLabel("Height", this);
    btnBox = new QDialogButtonBox(QDialogButtonBox::Ok, Qt::Horizontal, this);
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
    int i, n = 4, sx = 1, sy = 1, xt, yt;
    mouse_pressed = false;
    fisometric = false;
    canvas->drag_to_scroll = false;
    clk_radius = 60;
    pixmap = canvas->pixmap()->copy();
    scaleX = float(pixmap.width())/canvas->data->image.width();
    scaleY = float(pixmap.height())/canvas->data->image.height();
    for (i = 0; i < n; i++)
    {
        xt = (sx < 0) ? (pixmap.width() - 1) : 0;
        yt = (sy < 0) ? (pixmap.height() - 1) : 0;
        p << QPointF(xt, yt);
        pt << QPointF(xt, yt);
        sx = sx - sy;
        sy += sx; //  1, -1, -1,  1
        sx -= sy; //  1,  1, -1, -1
    }
    // add buttons
    checkIso = new QCheckBox("Isometric?", statusbar);
    statusbar->addPermanentWidget(checkIso);
    cropnowBtn = new QPushButton("Exec Now", statusbar);
    statusbar->addPermanentWidget(cropnowBtn);
    cropcancelBtn = new QPushButton("Cancel", statusbar);
    statusbar->addPermanentWidget(cropcancelBtn);
    connect(checkIso, SIGNAL(clicked()), this, SLOT(isomode()));
    connect(cropnowBtn, SIGNAL(clicked()), this, SLOT(transform()));
    connect(cropcancelBtn, SIGNAL(clicked()), this, SLOT(finish()));
    crop_widgets << checkIso << cropnowBtn << cropcancelBtn;
    statusbar->showMessage("Drag corners to fit edges around tilted image/document");
    drawCropBox();
}

void
PerspectiveTransform:: onMousePress(QPoint pos)
{
    int i, n, r, sx = 1, sy = 1;
    clk_pos = pos;
    r = clk_radius;
    mouse_pressed = true;
    // Determine which position is clicked
    clk_area = 0;
    n = pt.count();
    for (i = 0; i < n; i++)
    {
        if (QRectF(pt[i], QSize(sx * r, sy * r)).contains(clk_pos))
            clk_area = i + 1;
        sx = sx - sy;
        sy += sx; //  1, -1, -1,  1
        sx -= sy; //  1,  1, -1, -1
    }
}

void
PerspectiveTransform:: onMouseRelease(QPoint /*pos*/)
{
    mouse_pressed = false;
    pt = p;
}

void
PerspectiveTransform:: onMouseMove(QPoint pos)
{
    if (not mouse_pressed) return;
    QPointF moved = pos - clk_pos;
    QPointF last_pt = QPoint(pixmap.width()-1, pixmap.height()-1);
    QPointF new_pt;
    if (clk_area > 0)
    {
        new_pt = pt[clk_area - 1] + moved;
        p[clk_area - 1] = QPointF(MIN(last_pt.x(), MAX(0, new_pt.x())), MIN(last_pt.y(), MAX(0, new_pt.y())));
    }
    drawCropBox();
}

void
PerspectiveTransform:: drawCropBox()
{
    float start, span;
    int i, n, r, r2, j0, j1, j2, j3;
    QPixmap pm = pixmap.copy();
    QPainter painter(&pm);
    QPolygonF polygon;
    painter.drawPolygon(p);
    r = clk_radius;
    r2 = r / 2;
    n = p.count();
    if (n == 4)
    {
        for (i = 0; i < n; i++)
        {
            j0 = i;
            j3 = (i + 2) % 4;
            j1 = ((i - 1) < 0) ? (1 - i) : (i - 1);
            j2 = (j1 + 2) % 4;

            calcArc(p[j0], p[j1], p[j2], p[j3], start, span);
            painter.drawArc(p[i].x() - r2, p[i].y() - r2, r, r, 16 * start, 16 * span);
        }
        painter.setPen(Qt::white);
        polygon.clear();
        polygon << (p[0] + QPointF(1,1)) << (p[1] + QPointF(-1,1)) << (p[2] + QPointF(-1,-1)) << (p[3] + QPointF(1,-1));
        painter.drawPolygon(polygon);
        painter.end();
        canvas->setPixmap(pm);
    }
}

void
PerspectiveTransform:: isomode()
{
    fisometric = !fisometric;
}

void
PerspectiveTransform:: transform()
{
    int i, n;
    float min_w, min_h, max_w, max_h;
    QPointF mxy, sxy;
    QPolygonF mapFrom, mapTo;
    QTransform tfm, trueMtx;
    QImage img;
    n = p.count();
    if (n == 4)
    {
        for (i = 0; i < n; i++)
            p[i] = QPointF(p[i].x() / scaleX, p[i].y() / scaleY);
        if (fisometric)
        {
            mxy = meanx2(p);
            sxy = stdevx2(p);
            min_w = mxy.x() - sxy.x();
            min_h = mxy.y() - sxy.y();
            max_w = mxy.x() + sxy.x();
            max_h = mxy.y() + sxy.y();
        }
        else
        {
            min_w = 0;
            min_h = 0;
            max_w = MAX(p[1].x() - p[0].x(), p[2].x() - p[3].x());
            max_h = MAX(p[3].y() - p[0].y(), p[2].y() - p[1].y());
        }
        mapFrom << p[0] << p[1] << p[2] << p[3];
        mapTo << QPointF(min_w, min_h) << QPointF(max_w, min_h) << QPointF(max_w, max_h) << QPointF(min_w, max_h);
        QTransform::quadToQuad(mapFrom, mapTo, tfm);
        img = canvas->data->image.transformed(tfm, Qt::SmoothTransformation);
        trueMtx = QImage::trueMatrix(tfm,canvas->data->image.width(),canvas->data->image.height());
        if (fisometric)
        {
            canvas->data->image = img.copy();
        }
        else
        {
            pt[0] = trueMtx.map(p[0]);
            pt[2] = trueMtx.map(p[2]);
            canvas->data->image = img.copy(QRect(QPoint(pt[0].x(),pt[0].y()), QPoint(pt[2].x(),pt[2].y())));
        }
    }
    finish();
}

void
PerspectiveTransform:: finish()
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
    ratioEdit->setValidator(&validator);
    multRIS->setValidator(&validator);
    orig_width = img_width;
    orig_height = img_height;
    widthEdit->setText( QString::number(img_width));
    heightEdit->setText( QString::number(img_height));
    multRIS->setText( QString::number(1.0));
    spinWidth->setValue(img_width*2.54/300);
    spinHeight->setValue(img_height*2.54/300);
    QObject::connect(ratioEdit, SIGNAL(editingFinished()), this, SLOT(ratioTextChanged()));
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

void
ResizeDialog:: ratioTextChanged()
{
    float ratio = ratioEdit->text().toDouble();
    float dr = (ratio > orig_ratio) ? (ratio - orig_ratio) : (orig_ratio - ratio);
    if (dr > 0.0)
    {
        if (ratio > 0.0)
        {
            widthEdit->setText( QString::number(round(ratio * orig_width)));
            heightEdit->setText( QString::number(round(ratio * orig_height)));
        } else {
            widthEdit->setText( QString::number(round(orig_width)));
            heightEdit->setText( QString::number(round(orig_height)));
        }
    }
    orig_ratio = ratio;
}

// ******************************************************************* |
//                         Simple DeWarping
// ------------------------------------------------------------------- |

DeWarping::
DeWarping(Canvas *canvas, QStatusBar *statusbar, int count) : QObject(canvas),
                                canvas(canvas), statusbar(statusbar)
{
    int i, n = count, dxt, xt, yth, ytd;
    mouse_pressed = false;
    flagrange = false;
    fequalarea = false;
    clk_radius = 10;
    canvas->drag_to_scroll = false;
    pixmap = canvas->pixmap()->copy();
    scaleX = float(pixmap.width())/canvas->data->image.width();
    scaleY = float(pixmap.height())/canvas->data->image.height();
    dxt = (pixmap.width() - 1) / (n + 1);
    xt = dxt;
    ytd = (pixmap.height() - 1) / 4;
    yth = ytd + ytd + ytd;
    lnh << QPointF(0, 0);
    lnht << QPointF(0, 0);
    lnd << QPointF(0, 0);
    lndt << QPointF(0, 0);
    lnh << QPointF(0, yth);
    lnht << QPointF(0, yth);
    lnd << QPointF(0, ytd);
    lndt << QPointF(0, ytd);
    for (i = 0; i < n; i++)
    {
        lnh << QPointF(xt, yth);
        lnht << QPointF(xt, yth);
        lnd << QPointF(xt, ytd);
        lndt << QPointF(xt, ytd);
        xt += dxt;
    }
    lnh << QPointF(pixmap.width(), yth);
    lnht << QPointF(pixmap.width(), yth);
    lnd << QPointF(pixmap.width(), ytd);
    lndt << QPointF(pixmap.width(), ytd);
    lnh << QPointF(pixmap.width(), 0);
    lnht << QPointF(pixmap.width(), 0);
    lnd << QPointF(pixmap.width(), 0);
    lndt << QPointF(pixmap.width(), 0);
    // add buttons
    LagrangeCheck = new QCheckBox("Akima?", statusbar);
    statusbar->addPermanentWidget(LagrangeCheck);
    EqualAreaCheck = new QCheckBox("Equal Area?", statusbar);
    statusbar->addPermanentWidget(EqualAreaCheck);
    cropnowBtn = new QPushButton("DeWarp Now", statusbar);
    statusbar->addPermanentWidget(cropnowBtn);
    cropcancelBtn = new QPushButton("Cancel", statusbar);
    statusbar->addPermanentWidget(cropcancelBtn);
    connect(LagrangeCheck, SIGNAL(clicked()), this, SLOT(LagrangeMode()));
    connect(EqualAreaCheck, SIGNAL(clicked()), this, SLOT(EqualAreaMode()));
    connect(cropnowBtn, SIGNAL(clicked()), this, SLOT(transform()));
    connect(cropcancelBtn, SIGNAL(clicked()), this, SLOT(finish()));
    crop_widgets << EqualAreaCheck << LagrangeCheck << cropnowBtn << cropcancelBtn;
    statusbar->showMessage("Set node for high and down contors image/document");
    drawDeWarpLine();
}

void
DeWarping:: onMousePress(QPoint pos)
{
    int i, n, r;
    clk_pos = pos;
    mouse_pressed = true;
    // Determine which position is clicked
    clk_area_h = 0;
    clk_area_d = 0;
    r = clk_radius;
    n = lnht.count();
    for (i = 2; i < n - 2; i++)
    {
        if (QRectF((lnht[i] + QPointF(-r, -r)), QSize(r + r, r + r)).contains(clk_pos))
            clk_area_h = i - 1;
        if (QRectF((lndt[i] + QPointF(-r, -r)), QSize(r + r, r + r)).contains(clk_pos))
            clk_area_d = i - 1;
    }
}

void
DeWarping:: onMouseRelease(QPoint /*pos*/)
{
    mouse_pressed = false;
    lnht = lnh;
    lndt = lnd;
}

void
DeWarping:: onMouseMove(QPoint pos)
{
    if (not mouse_pressed) return;
    int clk_area, n = lnht.count();
    QPointF moved = pos - clk_pos;
    QPointF null_pt = QPointF(-1, -1);
    QPointF last_pt = QPointF(pixmap.width(), pixmap.height());
    QPointF new_pt, lnhc, lndc, lnhp, lndp, lnhn, lndn;
    clk_area = (clk_area_h > 0) ? clk_area_h : clk_area_d;
    lnhc = (clk_area > 0) ? lnht[clk_area + 1] : null_pt;
    lndc = (clk_area > 0) ? lndt[clk_area + 1] : null_pt;
    lnhp = (clk_area > 1) ? lnht[clk_area] : null_pt;
    lndp = (clk_area > 1) ? lndt[clk_area] : null_pt;
    lnhn = (clk_area < n - 2) ? lnht[clk_area + 2] : last_pt;
    lndn = (clk_area < n - 2) ? lndt[clk_area + 2] : last_pt;
    if (clk_area_h > 0)
    {
        new_pt = lnhc + moved;
        lnh[clk_area_h + 1] = QPointF(MIN(lnhn.x() - 1, MAX(lnhp.x() + 1, new_pt.x())), MIN(last_pt.y() - 1, MAX(lndc.y() + 1, new_pt.y())));
        lnh[1] = QPointF(0, lnh[2].y());
        lnh[n-2] = QPointF(pixmap.width(), lnh[n - 3].y());
    }
    if (clk_area_d > 0)
    {
        new_pt = lndc + moved;
        lnd[clk_area_d + 1] = QPointF(MIN(lndn.x() - 1, MAX(lndp.x() + 1, new_pt.x())), MIN(lnhc.y() - 1, MAX(0, new_pt.y())));
        lnd[1] = QPointF(0, lnd[2].y());
        lnd[n-2] = QPointF(pixmap.width(), lnd[n - 3].y());
    }
    drawDeWarpLine();
}

void
DeWarping:: drawDeWarpLine()
{
    int i, n, r, r2, rs2;
    float area;
    QPixmap pm = pixmap.copy();
    QPainter painter(&pm);
    n = lnh.count();
    r = clk_radius;
    r2 = r - 1;
    rs2 = r * 0.707107f;
    area = calcArea(lnh);
    ylnh = area / pixmap.width();
    area = calcArea(lnd);
    ylnd = area / pixmap.width();
    painter.setPen(Qt::gray);
    for (i = 2; i < n - 2; i++)
    {
        painter.drawLine(lnh[i].x() - rs2, lnh[i].y() - rs2, lnh[i].x() + rs2, lnh[i].y() + rs2);
        painter.drawLine(lnh[i].x() - rs2, lnh[i].y() + rs2, lnh[i].x() + rs2, lnh[i].y() - rs2);
        painter.drawLine(lnd[i].x() - rs2, lnd[i].y() - rs2, lnd[i].x() + rs2, lnd[i].y() + rs2);
        painter.drawLine(lnd[i].x() - rs2, lnd[i].y() + rs2, lnd[i].x() + rs2, lnd[i].y() - rs2);
    }
    painter.setPen(Qt::white);
    for (i = 2; i < n - 2; i++)
    {
        painter.drawArc(lnh[i].x() - r2, lnh[i].y() - r2, r2 + r2, r2 + r2, 0, 5760.0f);
        painter.drawArc(lnd[i].x() - r2, lnd[i].y() - r2, r2 + r2, r2 + r2, 0, 5760.0f);
    }
    painter.setPen(Qt::black);
    for (i = 2; i < n - 2; i++)
    {
        painter.drawArc(lnh[i].x() - r, lnh[i].y() - r, r + r, r + r, 0, 5760.0f);
        painter.drawArc(lnd[i].x() - r, lnd[i].y() - r, r + r, r + r, 0, 5760.0f);
    }
    painter.drawPolygon(lnh);
    painter.drawPolygon(lnd);
    painter.setPen(Qt::red);
    painter.drawRect(0, ylnh, pixmap.width(), 0);
    painter.drawRect(0, ylnd, pixmap.width(), 0);
    painter.end();
    canvas->setPixmap(pm);
}

void
DeWarping:: LagrangeMode()
{
    flagrange = !flagrange;
}

void
DeWarping:: EqualAreaMode()
{
    fequalarea = !fequalarea;
}

void
DeWarping:: transform()
{
    int i, n, y, x, w, h, ih, id;
//    int ic; // InterpolateLagrangePolynomial
    float yh, yd, dyh, dyd, yk0, yk1, yk2, oy;
//    QPolygonF lni; // InterpolateLagrangePolynomial
    n = lnh.count();
    if (n > 4)
    {
        QImage img = canvas->data->image.copy();
        QRgb *row;
        w = img.width();
        h = img.height();
        for (i = 0; i < n; i++)
        {
            lnh[i] = QPointF(lnh[i].x() / scaleX, lnh[i].y() / scaleY);
            lnd[i] = QPointF(lnd[i].x() / scaleX, lnd[i].y() / scaleY);
        }
        ylnh /= scaleY;
        ylnd /= scaleY;
        ih = id = 2;
        dyh = (float)(lnh[ih].y() - lnh[ih - 1].y()) / (lnh[ih].x() - lnh[ih - 1].x());
        dyd = (float)(lnd[id].y() - lnd[id - 1].y()) / (lnd[id].x() - lnd[id - 1].x());
        for (x = 0; x < w; x++)
        {
            if (x >= lnh[ih].x())
            {
                ih++;
                dyh = (float)(lnh[ih].y() - lnh[ih - 1].y()) / (lnh[ih].x() - lnh[ih - 1].x());
            }
            if (x >= lnd[id].x())
            {
                id++;
                dyd = (float)(lnd[id].y() - lnd[id - 1].y()) / (lnd[id].x() - lnd[id - 1].x());
            }
            if (flagrange)
            {
                /* // InterpolateLagrangePolynomial
                lni.clear();
                ic = (ih < 3) ? 3 : (ih > n - 3) ? (n - 3) : ih;
                for (i = ic - 2; i < ic + 2; i++)
                    lni << lnh[i];
                yh = InterpolateLagrangePolynomial (x, lni);
                lni.clear();
                ic = (id < 3) ? 3 : (id > n - 3) ? (n - 3) : id;
                for (i = ic - 2; i < ic + 2; i++)
                    lni << lnd[i];
                yd = InterpolateLagrangePolynomial (x, lni);
                yh = (yh < 1.0f) ? 1.0f : ((yh < h - 1) ? yh : (h - 1));
                yd = (yd < 1.0f) ? 1.0f : ((yd < h - 1) ? yd : (h - 1));
                */
                yh = InterpolateAkima (x, lnh);
                yd = InterpolateAkima (x, lnd);
                yh = (yh < 1.0f) ? 1.0f : ((yh < h - 1) ? yh : (h - 1));
                yd = (yd < 1.0f) ? 1.0f : ((yd < h - 1) ? yd : (h - 1));
            }
            else
            {
                yh = (float)lnh[ih - 1].y() + dyh * (x - lnh[ih - 1].x());
                yd = (float)lnd[id - 1].y() + dyd * (x - lnd[id - 1].x());
            }
            yk0 = yd / ylnd;
            yk1 = (yh - yd) / (ylnh - ylnd);
            yk2 = (h - yh) / (h - ylnh);
            if (fequalarea)
            {
                for (y = 0; y < (int)(ylnd + 1.0f); y++)
                {
                    row = (QRgb*)img.constScanLine(y);
                    oy = (float)y * yk0;
                    row[x] = InterpolateBiCubic (canvas->data->image, oy, (float)x);
                }
                for (y = (int)(ylnd + 1.0f); y < (int)(ylnh + 1.0f); y++)
                {
                    row = (QRgb*)img.constScanLine(y);
                    oy = yd + (float)(y - ylnd) * yk1;
                    row[x] = InterpolateBiCubic (canvas->data->image, oy, (float)x);
                }
                for (y = (int)(ylnh + 1.0f); y < h; y++)
                {
                    row = (QRgb*)img.constScanLine(y);
                    oy = yh + (float)(y - ylnh) * yk2;
                    row[x] = InterpolateBiCubic (canvas->data->image, oy, (float)x);
                }
            }
            else
            {
                for (y = 0; y < h; y++)
                {
                    row = (QRgb*)img.constScanLine(y);
                    oy = yd + (float)(y - ylnd) * yk1;
                    row[x] = InterpolateBiCubic (canvas->data->image, oy, (float)x);
                }
            }
        }
        canvas->data->image = img.copy();
    }
    finish();
}

void
DeWarping:: finish()
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

// ******************************************************************* |
//                         DeOblique
// ------------------------------------------------------------------- |
DeOblique::
DeOblique(Canvas *canvas, QStatusBar *statusbar) : QObject(canvas),
                                canvas(canvas), statusbar(statusbar)
{
    int wt, ht;
    float xt, yt;
    mouse_pressed = false;
    canvas->drag_to_scroll = false;
    pixmap = canvas->pixmap()->copy();
    scaleX = float(pixmap.width())/canvas->data->image.width();
    scaleY = float(pixmap.height())/canvas->data->image.height();
    wt = pixmap.width() - 1;
    ht = pixmap.height() - 1;
    xt = 0.5f * wt;
    yt = 0.5f * ht;
    p << QPointF(0, yt) << QPointF(wt, yt) << QPointF(xt, 0) << QPointF(xt, ht);
    pt << QPointF(0, yt) << QPointF(wt, yt) << QPointF(xt, 0) << QPointF(xt, ht);
    // add buttons
    cropnowBtn = new QPushButton("Exec Now", statusbar);
    statusbar->addPermanentWidget(cropnowBtn);
    cropcancelBtn = new QPushButton("Cancel", statusbar);
    statusbar->addPermanentWidget(cropcancelBtn);
    connect(cropnowBtn, SIGNAL(clicked()), this, SLOT(transform()));
    connect(cropcancelBtn, SIGNAL(clicked()), this, SLOT(finish()));
    crop_widgets << cropnowBtn << cropcancelBtn;
    statusbar->showMessage("Drag corners to fit edges around tilted image/document");
    drawCropBox();
}

void
DeOblique:: onMousePress(QPoint pos)
{
    int i;
    float dx, dy, r, rmin = 0.0f;
    clk_pos = pos;
    mouse_pressed = true;
    // Determine which position is clicked
    clk_area = 0;
    for (i = 0; i < 4; i++)
    {
        dx = clk_pos.x() - pt[i].x();
        dy = clk_pos.y() - pt[i].y();
        r = dx * dx + dy * dy;
        if ((i == 0) || (r < rmin))
        {
            clk_area = i;
            rmin = r;
        }
    }
}

void
DeOblique:: onMouseRelease(QPoint /*pos*/)
{
    mouse_pressed = false;
    pt = p;
}

void
DeOblique:: onMouseMove(QPoint pos)
{
    if (not mouse_pressed) return;
    float x0, y0, x = 0.0f, y = 0.0f, dx, dy;
    int w = pixmap.width(), h = pixmap.height();
    QPointF moved = pos - clk_pos;
    QPointF last_pt = QPoint(w - 1, h - 1);
    QPointF new_pt;
    x0 = pt[clk_area].x();
    y0 = pt[clk_area].y();
    dx = moved.x();
    dy = moved.y();
    switch (clk_area)
    {
     case 0:
        x = 0.0f;
        y = y0 + dy;
        break;
    case 1:
        x = w - 1.0f;
        y = y0 + dy;
        break;
    case 2:
        x = x0 + dx;
        y = 0.0f;
        break;
    case 3:
        x = x0 + dx;
        y = h - 1.0f;
        break;
    }
    new_pt = QPointF(x, y);
    p[clk_area] = QPointF(MIN(last_pt.x(), MAX(0, new_pt.x())), MIN(last_pt.y(), MAX(0, new_pt.y())));
    drawCropBox();
}

void
DeOblique:: drawCropBox()
{
    QPixmap pm = pixmap.copy();
    QPainter painter(&pm);
    painter.drawLine(p[0], p[1]);
    painter.drawLine(p[2], p[3]);
    painter.setPen(Qt::white);
    painter.drawLine(p[0] + QPointF(0,1), p[1]  + QPointF(0,1));
    painter.drawLine(p[0] - QPointF(0,1), p[1]  - QPointF(0,1));
    painter.drawLine(p[2] + QPointF(1,0), p[3]  + QPointF(1,0));
    painter.drawLine(p[2] - QPointF(1,0), p[3]  - QPointF(1,0));
    painter.end();
    canvas->setPixmap(pm);
}

void
DeOblique:: transform()
{
    int i, w = canvas->data->image.width(), h = canvas->data->image.height();
    float dx, dy;
    QPolygonF mapFrom, mapTo;
    QTransform tfm;
    QImage img;
    for (i = 0; i < 4; i++)
        p[i] = QPointF(p[i].x() / scaleX, p[i].y() / scaleY);
    dy = (p[1].y() - p[0].y()) * 0.5f;
    dx = (p[3].x() - p[2].x()) * 0.5f;
    mapFrom << QPointF(0, 0) << QPointF(w - 1, 0) << QPointF(w - 1, h - 1) << QPointF(0, h - 1);
    mapTo << QPointF(dx, dy) << QPointF(w + dx - 1, -dy) << QPointF(w - dx - 1, h - dy - 1) << QPointF(-dx, h + dy - 1);
    QTransform::quadToQuad(mapFrom, mapTo, tfm);
    img = canvas->data->image.transformed(tfm, Qt::SmoothTransformation);
    canvas->data->image = img.copy();
    finish();
}

void
DeOblique:: finish()
{
    canvas->showScaled();
    canvas->drag_to_scroll = true;
    // remove buttons
    while (not crop_widgets.isEmpty())
    {
        QWidget *widget = crop_widgets.takeLast();
        statusbar->removeWidget(widget);
        widget->deleteLater();
    }
    emit finished();
    this->deleteLater();
}

// transformation begin
//Mean for Isometric mode Un-tilt (PerspectiveTransform)
QPointF meanx2(QPolygonF p)
{
    int i, n = p.count();
    float mx = 0.0f, my = 0.0f;
    if (n > 0)
    {
        for (i = 0; i < n; i++)
        {
            mx += (float)p[i].x();
            my += (float)p[i].y();
        }
        mx /= (float)n;
        my /= (float)n;
    }
    return QPoint((int)(mx + 0.5f), (int)(my + 0.5f));
}
//StDev for Isometric mode Un-tilt (PerspectiveTransform)
QPointF stdevx2(QPolygonF p)
{
    int i, n = p.count();
    float mx = 0.0f, my = 0.0f, sx = 0.0f, sy = 0.0f;
    float dx, dy;
    if (n > 0)
    {
        for (i = 0; i < n; i++)
        {
            mx += (float)p[i].x();
            my += (float)p[i].y();
        }
        mx /= (float)n;
        my /= (float)n;
        for (i = 0; i < n; i++)
        {
            dx = mx - (float)p[i].x();
            dy = my - (float)p[i].y();
            sx += dx * dx;
            sy += dy * dy;
        }
        sx /= (float)n;
        sy /= (float)n;
        sx = sqrtf(sx);
        sy = sqrtf(sy);
    }
    return QPoint((int)(sx + 0.5f), (int)(sy + 0.5f));
}

// arc is drawn from p1 to p2 through p3
// p3 is at diagonal corner of p
// angle is  drawn counter clock-wise, and direction of y axis is
// upward, while direction of image y axis is downward
void calcArc(QPointF p/*center*/, QPointF p1, QPointF p2, QPointF p3,
                                    float &start, float &span)
{
    float ang1, ang2, ang3, gr = 180.0f / 3.14159265f;
    ang1 = atan2(p.y() - p1.y(), p1.x() - p.x()) * gr;
    ang2 = atan2(p.y() - p2.y(), p2.x() - p.x()) * gr;
    ang3 = atan2(p.y() - p3.y(), p3.x() - p.x()) * gr;
    if (ang1 > ang2)
    {
        ang1 += ang2;
        ang2 = ang1 - ang2;
        ang1 -= ang2;
    }
    if ((ang1 < ang3) and (ang3 < ang2))
    {
        start = ang1;
        span = ang2 - ang1;
    }
    else
    {
        start = ang2;
        span = 360 - (ang2 - ang1);
    }
}

float calcArea(QPolygonF p)
{
    int i, n = p.count();
    float area = 0.0f;
    if (n > 0)
    {
        area = p[0].y() * p[n - 1].x() - p[0].x() * p[n - 1].y();
        for (i = 0; i < n - 1; i++)
        {
            area += (p[i + 1].y() * p[i].x() - p[i + 1].x() * p[i].y());
        }
        area *= 0.5f;
        area = (area < 0) ? -area : area;
    }
    return area;
}

float InterpolateLagrangePolynomial (float x, QPolygonF p)
{
    int i, j, n = p.count();
    float basics_pol, lagrange_pol = 0.0f;

    for (i = 0; i < n; i++)
    {
        basics_pol = 1.0f;
        for (j = 0; j < n; j++)
        {
            if (j != i)
                basics_pol *= (x - p[j].x())/(p[i].x() - p[j].x());
        }
        lagrange_pol += basics_pol * p[i].y();
    }
    return lagrange_pol;
}

float InterpolateAkima (float x, QPolygonF p)
{
    int i, k, l, n = p.count();
    float xt, dx, dy, a, b, fx, val, m[5], t[2];

    k = 0;
    dx = (p[n - 1].x() - p[0].x()) / n;
    for (i = 1; i < (n - 1); i++)
    {
        xt = p[i].x();
        k = (xt < x) ? i : k;
    }
    if (k < 2)
    {
        for (i = (2 - k); i < 5; i++)
        {
            l = k + i - 2;
            dx = p[l + 1].x() - p[l].x();
            dy = p[l + 1].y() - p[l].y();
            m[i] = (dx > 0.0f) ? (dy / dx) : 0.0f;
        }
        for (i = (1 - k); i >= 0; i--)
        {
            m[i] = 2.0f * m[i + 1] - m[i + 2];
        }
    }
    else
    {
        if (k > (n - 4))
        {
            for (i = 0; i < (n - k + 1); i++)
            {
                l = k + i - 2;
                dx = p[l + 1].x() - p[l].x();
                dy = p[l + 1].y() - p[l].y();
                m[i] = (dx > 0.0f) ? (dy / dx) : 0.0f;
            }
            for (i = (n - k + 1); i < 5; i++)
            {
                m[i] = 2.0f * m[i - 1] - m[i - 2];
            }
        }
        else
        {
            for (i = 0; i < 5; i++)
            {
                l = k + i - 2;
                dx = p[l + 1].x() - p[l].x();
                dy = p[l + 1].y() - p[l].y();
                m[i] = (dx > 0.0f) ? (dy / dx) : 0.0f;
            }
        }
    }
    for(i = 0; i < 2; i++)
    {
        a = (m[i + 2] > m[i + 3]) ? (m[i + 2] - m[i + 3]) : (m[i + 3] - m[i + 2]);
        b = (m[i] > m[i + 1]) ? (m[i] - m[i + 1]) : (m[i + 1] - m[i]);
        t[i] = ((a + b) > 0) ? ((a * m[i + 1] + b * m[i + 2]) / (a + b)) : (0.5f * (m[i + 1] + m[i + 2]));
    }
    dx = x - p[k].x();
    dy = p[k + 1].x() - p[k].x();
    fx = (dy > 0.0f) ? (dx / dy) : 0.0f;
    val = p[k].y() + t[0] * dx + (3.0f * m[2] - 2.0f * t[0] - t[1]) * dx * fx + (t[0] + t[1] - 2.0f * m[2]) * dx * fx * fx;

    return val;
}

QRgb InterpolateBiCubic (QImage img, float y, float x)
{
    int i, d, dn, xi, yi, xf, yf;
    float d0, d2, d3, a0, a1, a2, a3;
    float dx, dy, k2 = 1.0f / 2.0f, k3 = 1.0f / 3.0f, k6 = 1.0f / 6.0f;
    float Cc, C[4];
    int Ci, pt[4];
    int height = img.height();
    int width = img.width();;
    QRgb *row, imgpix;

    yi = (int)y;
    dy = y - yi;
    yi = (yi < 0) ? 0 : (yi < height) ? yi : (height - 1);
    xi = (int)x;
    dx = x - xi;
    xi = (xi < 0) ? 0 : (xi < width) ? xi : (width - 1);
    dn = (img.hasAlphaChannel()) ? 4 : 3;
    for(d = 0; d < dn; d++)
    {
        if (dy > 0.0f)
        {
            for(i = -1; i < 3; i++)
            {
                yf = (int)y + i;
                yf = (yf < 0) ? 0 : (yf < height) ? yf : (height - 1);
                row = (QRgb*)img.constScanLine(yf);
                if (dx > 0.0f)
                {
                    xf = xi;
                    a0 = SelectChannelPixel(row[xf], d);
                    xf = (int)x - 1;
                    xf = (xf < 0) ? 0 : (xf < width) ? xf : (width - 1);
                    d0 = SelectChannelPixel(row[xf], d);
                    d0 -= a0;
                    xf = (int)x + 1;
                    xf = (xf < 0) ? 0 : (xf < width) ? xf : (width - 1);
                    d2 = SelectChannelPixel(row[xf], d);
                    d2 -= a0;
                    xf = (int)x + 2;
                    xf = (xf < 0) ? 0 : (xf < width) ? xf : (width - 1);
                    d3 = SelectChannelPixel(row[xf], d);
                    d3 -= a0;
                    a1 = -k3 * d0 + d2 - k6 * d3;
                    a2 = k2 * d0 + k2 * d2;
                    a3 = -k6 * d0 - k2 * d2 + k6 * d3;
                    C[i + 1] = a0 + (a1 + (a2 + a3 * dx) * dx) * dx;
                }
                else
                {
                    xf = xi;
                    C[i + 1] = SelectChannelPixel(row[xf], d);
                }
            }
            a0 = C[1];
            d0 = C[0] - a0;
            d2 = C[2] - a0;
            d3 = C[3] - a0;
            a1 = -k3 * d0 + d2 - k6 * d3;
            a2 = k2 * d0 + k2 * d2;
            a3 = -k6 * d0 - k2 * d2 + k6 * d3;
            Cc = a0 + (a1 + (a2 + a3 * dy) * dy) * dy;
        }
        else
        {
            yf = yi;
            row = (QRgb*)img.constScanLine(yf);
            if (dx > 0.0f)
            {
                xf = xi;
                a0 = SelectChannelPixel(row[xf], d);
                xf = (int)x - 1;
                xf = (xf < 0) ? 0 : (xf < width) ? xf : (width - 1);
                d0 = SelectChannelPixel(row[xf], d);
                d0 -= a0;
                xf = (int)x + 1;
                xf = (xf < 0) ? 0 : (xf < width) ? xf : (width - 1);
                d2 = SelectChannelPixel(row[xf], d);
                d2 -= a0;
                xf = (int)x + 2;
                xf = (xf < 0) ? 0 : (xf < width) ? xf : (width - 1);
                d3 = SelectChannelPixel(row[xf], d);
                d3 -= a0;
                a1 = -k3 * d0 + d2 - k6 * d3;
                a2 = k2 * d0 + k2 * d2;
                a3 = -k6 * d0 - k2 * d2 + k6 * d3;
                Cc = a0 + (a1 + (a2 + a3 * dx) * dx) * dx;
            }
            else
            {
                xf = xi;
                Cc = SelectChannelPixel(row[xf], d);
            }
        }
        Ci = (int)(Cc + 0.5f);
        pt[d] = (Ci < 0) ? 0 : (Ci > 255) ? 255 : Ci;
    }
    imgpix = (dn > 3) ? qRgba(pt[0], pt[1], pt[2],  pt[3]) : qRgb(pt[0], pt[1], pt[2]);

    return imgpix;
}

QRgb InterpolateBiAkima (QImage img, float y, float x)
{
    int i, j, d, dn, xi, yi, xf, yf;
    float dx, dy, zp, zf, a, b;
    float Cc, C[6], m[6], t[2];
    int Ci, pt[4];
    int height = img.height();
    int width = img.width();;
    QRgb *row, imgpix;

    yi = (int)y;
    dy = y - yi;
    yi = (yi < 0) ? 0 : (yi < height) ? yi : (height - 1);
    xi = (int)x;
    dx = x - xi;
    xi = (xi < 0) ? 0 : (xi < width) ? xi : (width - 1);
    dn = (img.hasAlphaChannel()) ? 4 : 3;
    for(d = 0; d < dn; d++)
    {
        if (dy > 0.0f)
        {
            for(i = -2; i < 4; i++)
            {
                yf = (int)y + i;
                yf = (yf < 0) ? 0 : (yf < height) ? yf : (height - 1);
                row = (QRgb*)img.constScanLine(yf);
                if (dx > 0.0f)
                {
                    zp = 0.0f;
                    for(j = -2; j < 4; j++)
                    {
                        xf = (int)x + j;
                        xf = (xf < 0) ? 0 : (xf < width) ? xf : (width - 1);
                        zf = SelectChannelPixel(row[xf], d);
                        m[j + 2] = zf - zp;
                        zp = zf;
                    }
                    for(j = 0; j < 2; j++)
                    {
                        a = (m[j + 3] > m[j + 4]) ? (m[j + 3] - m[j + 4]) : (m[j + 4] - m[j + 3]);
                        b = (m[j + 2] > m[j + 1]) ? (m[j + 2] - m[j + 1]) : (m[j + 1] - m[j + 2]);
                        t[j] = ((a + b) > 0) ? ((a * m[j + 2] + b * m[j + 3]) / (a + b)) : (0.5f * (m[j + 2] + m[j + 3]));
                    }
                    zf = SelectChannelPixel(row[xi], d);
                    C[i + 2] = zf + (t[0] + ((m[3] + m[3] + m[3] - t[0] - t[0] - t[1]) + (t[0] + t[1] - m[3] - m[3]) * dx) * dx) * dx;
                }
                else
                {
                    xf = xi;
                    C[i + 2] = SelectChannelPixel(row[xf], d);
                }
            }
            zp = 0.0f;
            for(i = -2; i < 4; i++)
            {
                m[i + 2] = C[i + 2] - zp;
                zp = C[i + 2];
            }
            for(i = 0; i < 2; i++)
            {
                a = (m[i + 3] > m[i + 4]) ? (m[i + 3] - m[i + 4]) : (m[i + 4] - m[i + 3]);
                b = (m[i + 2] > m[i + 1]) ? (m[i + 2] - m[i + 1]) : (m[i + 1] - m[i + 2]);
                t[i] = ((a + b) > 0) ? ((a * m[i + 2] + b * m[i + 3]) / (a + b)) : (0.5f * (m[i + 2] + m[i + 3]));
            }
            Cc = C[2] + (t[0] + ((m[3] + m[3] + m[3] - t[0] - t[0] - t[1]) + (t[0] + t[1] - m[3] - m[3]) * dy) * dy) * dy;
        }
        else
        {
            yf = yi;
            row = (QRgb*)img.constScanLine(yf);
            if (dx > 0.0f)
            {
                zp = 0.0f;
                for(j = -2; j < 4; j++)
                {
                    xf = (int)x + j;
                    xf = (xf < 0) ? 0 : (xf < width) ? xf : (width - 1);
                    zf = SelectChannelPixel(row[xf], d);
                    m[j + 2] = zf - zp;
                    zp = zf;
                }
                for(j = 0; j < 2; j++)
                {
                    a = (m[j + 3] > m[j + 4]) ? (m[j + 3] - m[j + 4]) : (m[j + 4] - m[j + 3]);
                    b = (m[j + 2] > m[j + 1]) ? (m[j + 2] - m[j + 1]) : (m[j + 1] - m[j + 2]);
                    t[j] = ((a + b) > 0) ? ((a * m[j + 2] + b * m[j + 3]) / (a + b)) : (0.5f * (m[j + 2] + m[j + 3]));
                }
                zf = SelectChannelPixel(row[xi], d);
                Cc = zf + (t[0] + ((m[3] + m[3] + m[3] - t[0] - t[0] - t[1]) + (t[0] + t[1] - m[3] - m[3]) * dx) * dx) * dx;
            }
            else
            {
                xf = xi;
                Cc = SelectChannelPixel(row[xf], d);
            }
        }
        Ci = (int)(Cc + 0.5f);
        pt[d] = (Ci < 0) ? 0 : (Ci > 255) ? 255 : Ci;
    }
    imgpix = (dn > 3) ? qRgba(pt[0], pt[1], pt[2],  pt[3]) : qRgb(pt[0], pt[1], pt[2]);

    return imgpix;
}
// transformation end
