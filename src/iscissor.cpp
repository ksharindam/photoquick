/* This file is a part of photoquick program, which is GPLv3 licensed */

#include "iscissor.h"
#include "common.h"
#include "filters.h"
#include <QColorDialog>
#include <QButtonGroup>
#include <QComboBox>
#include <QDialogButtonBox>
#include <cmath>

// ---------------------------------------------------------------------
//*************** Intelligent Scissor and Manual Eraser ****************
// _____________________________________________________________________

typedef unsigned int uint;

class IntBuffer
{
public:
    uint *data;
    int width;
    int height;
    IntBuffer(int width, int height);
    ~IntBuffer();
};

void findOptimalPath(GradMap *grad_map, IntBuffer &dp_buff,
                    int x1, int y1, int xs, int ys);
std::vector<QPoint> plotShortPath(IntBuffer *dp_buff, int x1, int y1,
                    int target_x, int target_y);

void floodfill(QImage &img, QPoint pos, QRgb oldColor, QRgb newColor);


void updateImageArea(QImage &dst, QImage &src, int pos_x, int pos_y);


// *********************** IScissor Dialog ************************

IScissorDialog:: IScissorDialog(QImage &img, QWidget *parent) : QDialog(parent)
{
    this->image = img;
    timer = new QTimer(this);
    timer->setSingleShot(true);
    setupUi(this);
    QButtonGroup *btnGrp = new QButtonGroup(frame);
    btnGrp->addButton(iScissorBtn, 1);
    btnGrp->addButton(eraserBtn, 2);

    eraserSettingsWidget->setHidden(true);

    QHBoxLayout *layout = new QHBoxLayout(scrollAreaWidgetContents);
    layout->setContentsMargins(0, 0, 0, 0);
    canvas = new PaintCanvas(this);
    layout->addWidget(canvas);

    undoBtn->setEnabled(false);
    redoBtn->setEnabled(false);

    connect(acceptBtn, SIGNAL(clicked()), this, SLOT(confirmAccept()));
    connect(useAsMaskBtn, SIGNAL(clicked()), this, SLOT(useAsMask()));
    connect(cancelBtn, SIGNAL(clicked()), this, SLOT(reject()));
    connect(undoBtn, SIGNAL(clicked()), this, SLOT(undo()));
    connect(redoBtn, SIGNAL(clicked()), this, SLOT(redo()));
    connect(zoomInBtn, SIGNAL(clicked()), this, SLOT(zoomIn()));
    connect(zoomOutBtn, SIGNAL(clicked()), this, SLOT(zoomOut()));
    connect(btnGrp, SIGNAL(buttonClicked(int)), this, SLOT(onToolClick(int)));
    connect(canvas, SIGNAL(mousePressed(QPoint)), this, SLOT(onMousePress(QPoint)));
    connect(canvas, SIGNAL(mouseReleased(QPoint)), this, SLOT(onMouseRelease(QPoint)));
    connect(canvas, SIGNAL(mouseMoved(QPoint)), this, SLOT(onMouseMove(QPoint)));
    connect(eraserSizeSlider, SIGNAL(valueChanged(int)), this, SLOT(setEraserSize(int)));
    connect(timer, SIGNAL(timeout()), this, SLOT(updateEraserSize()));

    // scale the canvas to fit to scrollarea width
    int available_w = 1020 - frame->width() - 4;
    int img_w = img.width();
    scale = 1.0;
    while (img_w > available_w) {
        scale /= 1.5;
        img_w = roundf(scale*img.width());
    }
    zoomLabel->setText(QString("Zoom : %1x").arg(scale));
    onToolClick(1);//redraw
}


void
IScissorDialog:: zoomIn()
{
    scale *= 1.5;
    redraw();
    zoomLabel->setText(QString("Zoom : %1x").arg(scale));
    if (tool_type==TOOL_ERASER) {
        updateEraserSize();
    }
}

void
IScissorDialog:: zoomOut()
{
    scale /= 1.5;
    redraw();
    zoomLabel->setText(QString("Zoom : %1x").arg(scale));
    if (tool_type==TOOL_ERASER) {
        updateEraserSize();
    }
}

void
IScissorDialog:: undo()
{
    if (tool_type==TOOL_ISCISSOR)
        undo_iScissor();
    else
        undo_Eraser();
}

void
IScissorDialog:: redo()
{
    if (tool_type==TOOL_ISCISSOR)
        redo_iScissor();
    else
        redo_Eraser();
}

void
IScissorDialog:: keyPressEvent(QKeyEvent *ev)
{
    // prevent closing dialog on accidental Esc key press
    if (ev->key() != Qt::Key_Escape)
        return QDialog::keyPressEvent(ev);
    ev->accept();
}

void
IScissorDialog:: confirmAccept()
{
    // set Background color
    BgColorDialog *dlg = new BgColorDialog(this);
    connect(dlg, SIGNAL(bgColorSelected(int,QRgb)), this, SLOT(setBgColor(int,QRgb)));
    if (dlg->exec()==QDialog::Rejected) {
        redraw();
        return;
    }
    if (bg_color_type!=TRANSPERANT)
    {
        QImage img(image.width(), image.height(), QImage::Format_RGB32);
        QRgb clr = (bg_color_type==COLOR_WHITE) ? 0xffffff : bg_color;
        img.fill(clr);
        painter.begin(&img);
        painter.drawImage(QPoint(0,0), image);
        painter.end();
        image = img;
    }
    QDialog::accept();
}

void
IScissorDialog:: useAsMask()
{
    int w = image.width();
    int h = image.height();
    for (int y=0; y<h; y++) {
        QRgb *row = (QRgb*)image.scanLine(y);
        for (int x=0; x<w; x++) {
            int clr = 255 - qAlpha(row[x]);// use transperant areas as masked area
            row[x] = qRgb(clr, clr, clr);
        }
    }
    is_mask = true;
    QDialog::accept();
}

void
IScissorDialog:: done(int val)
{
    if (grad_map)
        delete grad_map;
    QDialog::done(val);
}

void
IScissorDialog:: setBgColor(int type, QRgb bg_clr)
{
    bg_color_type = type;
    bg_color = bg_clr;
    if (type==TRANSPERANT)
        redraw();
    else {
        //scaleImage();
        QImage img(image_scaled.width(), image_scaled.height(), QImage::Format_RGB32);
        QRgb clr = (type==COLOR_WHITE) ? 0xffffff : bg_clr;
        img.fill(clr);
        painter.begin(&img);
        painter.drawImage(QPoint(0,0), image_scaled);
        painter.end();
        canvas->setImage(img);
    }
}

void
IScissorDialog:: scaleImage()
{
    if (scale == 1.0)
        image_scaled = image;
    else {
        Qt::TransformationMode mode = scale<1.0? Qt::SmoothTransformation: Qt::FastTransformation;
        image_scaled = image.scaled(scale*image.width(), scale*image.height(),
                        Qt::IgnoreAspectRatio, mode);
    }
    // make it conversion to QPixmap and drawing faster
    if (image.format()==QImage::Format_ARGB32)
        image_scaled = image_scaled.convertToFormat(QImage::Format_ARGB32_Premultiplied);
}

void
IScissorDialog:: redraw()
{
    scaleImage();
    if (tool_type==TOOL_ISCISSOR)
        drawFullPath();
    canvas->setImage(image_scaled);
}

void
IScissorDialog:: onToolClick(int btn_id)
{
    if (tool_type == btn_id)
        return;
    // finish previously selected button
    if (tool_type==TOOL_ISCISSOR) {
        smoothEdgesBtn->setHidden(true);
        seeds.clear();
        shortPath.clear();
        fullPath.clear();
        redoStack.clear();
        if (grad_map) {
            delete grad_map;
            grad_map = 0;
        }
        statusbar->setText("");
    }
    else if (tool_type==TOOL_ERASER) {
        eraserSettingsWidget->setHidden(true);
        undoStack_Eraser.clear();
        redoStack_Eraser.clear();
    }
    // init currently selected button
    tool_type = btn_id;
    if (tool_type==TOOL_ISCISSOR) {
        smoothEdgesBtn->show();
        canvas->setCursor(QCursor(QPixmap(":/images/cursor-cross.png")));
        statusbar->setText("Tip : Click to place seeds around object");
        seed_mode = NO_SEED;
    }
    else if (tool_type==TOOL_ERASER) {
        eraserSettingsWidget->show();
        eraserSizeSlider->setMaximum((image.width()/850+1)*100);
        eraserSizeSlider->setValue((image.width()/850+1)*48);
        setEraserSize(eraserSizeSlider->value());
        if (image.format()!=QImage::Format_ARGB32)
            image = image.convertToFormat(QImage::Format_ARGB32);
    }
    undoBtn->setEnabled(false);
    redoBtn->setEnabled(false);
    redraw();
}

void
IScissorDialog:: setEraserSize(int val)
{
    eraserSizeLabel->setText(QString("Eraser Size : %1").arg(val));
    timer->start(400); // calls updateEraserSize
}

void
IScissorDialog:: updateEraserSize()
{
    int w = eraserSizeSlider->value();
    // create brush mask
    brush = QImage(w, w, QImage::Format_RGB32);
    brush.fill(Qt::black);
    painter.begin(&brush);
    painter.setBrush(QBrush(Qt::white));
    painter.drawEllipse(QPoint(w/2, w/2), (w*3)/8, (w*3)/8);
    painter.end();
    gaussianBlur(brush, w/8);
    //brush.save("brush2.png");
    brush_scaled = brush.scaledToWidth(w*scale, Qt::SmoothTransformation );
    canvas->setCursor(roundCursor(scale * w*7/8.0));
}

void
IScissorDialog:: onMousePress(QPoint pos)
{
    mouse_pressed = true;
    mouse_pos = pos/scale;
    if (tool_type==TOOL_ISCISSOR)
        onMousePress_iScissor(mouse_pos);
    else
        onMousePress_Eraser(mouse_pos);
}

void
IScissorDialog:: onMouseMove(QPoint pos)
{
    pos = pos/scale;
    if (tool_type==TOOL_ISCISSOR)
        onMouseMove_iScissor(pos);
    else
        onMouseMove_Eraser(pos);
    mouse_pos = pos;
}

void
IScissorDialog:: onMouseRelease(QPoint pos)
{
    mouse_pressed = false;
    if (tool_type==TOOL_ERASER)
        onMouseRelease_Eraser(pos/scale);
}

void
IScissorDialog:: onMousePress_Eraser(QPoint pos)
{
    min_x = max_x = pos.x();
    min_y = max_y = pos.y();
    image_tmp = image.copy();
}

void
IScissorDialog:: onMouseRelease_Eraser(QPoint)
{
    redraw();
    min_x = MAX(min_x - brush.width()/2, 0);
    min_y = MAX(min_y - brush.width()/2, 0);
    max_x = MIN(max_x + brush.width()/2, image.width()-1);
    max_y = MIN(max_y + brush.width()/2, image.height()-1);
    int w = max_x - min_x + 1;
    int h = max_y - min_y + 1;

    redoStack_Eraser.clear();
    redoBtn->setEnabled(false);
    HistoryItem undoItem = { min_x, min_y, image_tmp.copy(min_x, min_y, w, h) };
    undoStack_Eraser.append(undoItem);
    undoBtn->setEnabled(true);
    if (undoStack_Eraser.size()>10)
        undoStack_Eraser.removeFirst();
    image_tmp = QImage();
}

void
IScissorDialog:: onMouseMove_Eraser(QPoint pos)
{
    if (not mouse_pressed)
        return;
    int x0 = mouse_pos.x();
    int y0 = mouse_pos.y();
    int x = pos.x();
    int y = pos.y();

    min_x = MIN(x, min_x);
    min_y = MIN(y, min_y);
    max_x = MAX(x, max_x);
    max_y = MAX(y, max_y);

    float d = sqrtf((x-x0)*(x-x0) + (y-y0)*(y-y0));

    for (int dt=10; dt < d; dt+=10) {
        float t = dt/d;
        int xt = x0 + t*(x-x0);
        int yt = y0 + t*(y-y0);
        eraseAt(xt, yt);
    }
    eraseAt(x,y);
    canvas->setImage(image_scaled);
}


void
IScissorDialog:: eraseAt(int pos_x, int pos_y)
{
    // draw mask over original image
    int x = pos_x - brush.width()/2;
    int y = pos_y - brush.width()/2;

    int min_j = x<0 ? -x : 0;
    int min_i = y<0 ? -y : 0;

    int max_i = y + brush.width() > image.height()? image.height()-1-y : brush.height()-1;
    int max_j = x + brush.width() > image.width() ? image.width() -1-x : brush.width() -1;

    #pragma omp parallel for
    for (int i=min_i; i<=max_i; i++) {
        QRgb *row, *brush_row;
        #pragma omp critical
        {row = (QRgb*)image.scanLine(y+i);
         brush_row = (QRgb*)brush.constScanLine(i);}
        for (int j=min_j; j<=max_j; j++) {
            int clr = row[x+j];
            int alpha = MAX(qAlpha(clr) - qRed(brush_row[j]), 0);
            row[x+j] = ((alpha & 0xff) << 24) | (clr & 0xffffff);
        }
    }
    // draw over scaled image for display
    int brush_w = brush_scaled.width();
    int img_w = image_scaled.width();
    int img_h = image_scaled.height();

    x = pos_x*scale - brush_w/2;
    y = pos_y*scale - brush_w/2;

    min_j = x<0 ? -x : 0;
    min_i = y<0 ? -y : 0;

    max_i = y + brush_w > img_h ? img_h-1-y : brush_w-1;
    max_j = x + brush_w > img_w ? img_w-1-x : brush_w-1;

    #pragma omp parallel for
    for (int i=min_i; i<=max_i; i++) {
        QRgb *row, *brush_row;
        #pragma omp critical
        {row = (QRgb*)image_scaled.scanLine(y+i);
         brush_row = (QRgb*)brush_scaled.constScanLine(i);}
        for (int j=min_j; j<=max_j; j++) {
            int clr = row[x+j];
            if (qAlpha(clr)==0) {
                row[x+j] = 0;
                continue;
            }
            int alpha = MAX(qAlpha(clr) - qRed(brush_row[j]), 0);
            // set alpha channel for ARGB32 Premultiplied format
            int r = (qRed(clr)*alpha)/qAlpha(clr);
            int g = (qGreen(clr)*alpha)/qAlpha(clr);
            int b = (qBlue(clr)*alpha)/qAlpha(clr);
            row[x+j] = qRgba(r,g,b, alpha);
        }
    }
}

void
IScissorDialog:: undo_Eraser()
{
    if (undoStack_Eraser.isEmpty()) return;
    HistoryItem item = undoStack_Eraser.takeLast();
    HistoryItem redoItem = {item.x, item.y, image.copy(item.x,item.y,item.image.width(),item.image.height())};
    redoStack_Eraser.append(redoItem);
    redoBtn->setEnabled(true);
    undoBtn->setEnabled(not undoStack_Eraser.isEmpty());
    updateImageArea(image, item.image, item.x, item.y);
    redraw();
}

void
IScissorDialog:: redo_Eraser()
{
    if (redoStack_Eraser.isEmpty()) return;
    HistoryItem item = redoStack_Eraser.takeLast();
    HistoryItem undoItem = {item.x, item.y, image.copy(item.x,item.y,item.image.width(),item.image.height())};
    undoStack_Eraser.append(undoItem);
    undoBtn->setEnabled(true);
    redoBtn->setEnabled(not redoStack_Eraser.isEmpty());
    updateImageArea(image, item.image, item.x, item.y);
    redraw();
}

// ****************** Intelligent Scissor Tool *******************

void
IScissorDialog:: onMousePress_iScissor(QPoint pos)
{
    // no seed was placed before so cant draw permanent path
    if (seed_mode == NO_SEED) {
        if (!grad_map) grad_map = new GradMap(image);
        seeds.push_back(pos);
        redoStack.clear();
        seed_mode = SEED_PLACED;
        undoBtn->setEnabled(true);
        redoBtn->setEnabled(false);
        statusbar->setText("Tip : Place more seeds until a closed loop is created");
    }
    // can draw a permanent path
    else if (seed_mode == SEED_PLACED) {
        seeds.push_back(pos);
        redoStack.clear();
        redoBtn->setEnabled(false);
        // check if clicked on seed/path is closed
        checkPathClosed();
        if (seed_mode == PATH_CLOSED) {
            calcShortPath(seeds[seeds.size()-2], seeds.back());
            statusbar->setText("Tip : Click inside loop to erase outside of loop");
        }
        fullPath.push_back(shortPath);
        drawSeedToSeedPath();
    }
    // path was closed, it is time to create mask
    else if (not fullPath.empty()){ // i.e mask not generated
        getMaskedImage(pos);
        seeds.clear();
        fullPath.clear();
        redraw();
        undoBtn->setEnabled(false);
        statusbar->setText("Tip : Click accept or place more seeds and cut again");
        seed_mode = NO_SEED;
    }
}

void
IScissorDialog:: onMouseMove_iScissor(QPoint pos)
{
    if (seed_mode != SEED_PLACED) return; //Return if mouse is not clicked
    calcShortPath(seeds.back(), pos);
    drawSeedToCursorPath();
}

void
IScissorDialog:: calcShortPath(QPoint from/*seed*/, QPoint to/*cursor*/){
    int x = to.x();
    int y = to.y();
    int seed_x = from.x();
    int seed_y = from.y();
    // Get bounding box of livewire
    int x1 = MIN(x, seed_x); // topleft x
    int y1 = MIN(y, seed_y);
    int x2 = MAX(x, seed_x); // bottom right x
    int y2 = MAX(y, seed_y);

    shortPath.clear();
    if (x1==x2) { // plot vertical line
        for (int i=y1; i<=y2; i++)
            shortPath.push_back(QPoint(x1, i));
    }
    else if (y1==y2) { // plot horizontal line
        for (int i=x1; i<=x2; i++)
            shortPath.push_back(QPoint(i, y1));
    }
    else {
        int extend_w = (x2-x1)*0.2 + 5;
        int extend_h = (y2-y1)*0.2 + 5;
        if (x1==seed_x)
            x2 = MIN(x2+extend_w, image.width()-1);
        else
            x1 = MAX(x1-extend_w, 0);
        if (y1==seed_y)
            y2 = MIN(y2+extend_h, image.height()-1);
        else
            y1 = MAX(y1-extend_h, 0);

        IntBuffer dp_buff(x2-x1+1, y2-y1+1);
        findOptimalPath(grad_map, dp_buff, x1, y1, seed_x, seed_y);
        shortPath = plotShortPath(&dp_buff, x1, y1, x, y);
    }
}

// Draw movable last line (livewire)
void
IScissorDialog:: drawSeedToCursorPath()
{
    QPixmap pm = QPixmap::fromImage(image_scaled.copy());
    painter.begin(&pm);
    QPen pen(Qt::red, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    painter.setPen(pen);
    QPoint pt1 = shortPath[0];
    for (uint i=1; i<shortPath.size(); i++)
    {
        QPoint pt2 = shortPath[i];
        painter.drawLine(pt1*scale, pt2*scale);
        pt1 = pt2;
    }
    painter.setPen(Qt::black);
    painter.setBrush(QBrush(Qt::white));
    painter.drawEllipse(seeds.back()*scale, 4, 4);
    painter.end();
    canvas->setPixmap(pm);
}

// draw last permanent non movable short path
void
IScissorDialog:: drawSeedToSeedPath()
{
    painter.begin(&image_scaled);
    QPen pen(Qt::blue, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    painter.setPen(pen);
    std::vector<QPoint> short_path = fullPath.back();
    QPoint pt1 = short_path[0];
    for (uint i=1; i<short_path.size(); i++)
    {
        QPoint pt2 = short_path[i];
        painter.drawLine(pt1*scale, pt2*scale);
        pt1 = pt2;
    }
    painter.setPen(Qt::black);
    painter.setBrush(QBrush(Qt::white));
    painter.drawEllipse(seeds[seeds.size()-2]*scale, 4, 4);
    painter.end();
    canvas->setImage(image_scaled);
}

// this is called when image is scaled
void
IScissorDialog:: drawFullPath()
{
    if (fullPath.empty())
        return;
    painter.begin(&image_scaled);
    QPen pen(Qt::blue, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    painter.setPen(pen);
    for (auto short_path : fullPath) {
        QPoint pt1 = short_path[0];
        for (uint i=1; i<short_path.size(); i++)
        {
            QPoint pt2 = short_path[i];
            painter.drawLine(pt1*scale, pt2*scale);
            pt1 = pt2;
        }
    }
    painter.setPen(Qt::black);
    painter.setBrush(QBrush(Qt::white));
    for (QPoint seed : seeds) {
        painter.drawEllipse(seed*scale, 4, 4);
    }
    painter.end();
}

void
IScissorDialog:: undo_iScissor()
{
    redoStack.push_back(seeds.back());
    redoBtn->setEnabled(true);
    seeds.pop_back();
    if (seeds.empty()) {
        seed_mode = NO_SEED;
        undoBtn->setEnabled(false);
    }
    else {
        seed_mode = SEED_PLACED;//change from PATH_CLOSED mode
        fullPath.pop_back();
    }
    redraw();
}

void
IScissorDialog:: redo_iScissor()
{
    seeds.push_back(redoStack.back());
    undoBtn->setEnabled(true);
    redoStack.pop_back();
    if (redoStack.empty())
        redoBtn->setEnabled(false);
    if (seed_mode==NO_SEED)
        seed_mode = SEED_PLACED; // first seed placed, nothing to do
    else {
        calcShortPath(seeds[seeds.size()-2], seeds.back());
        fullPath.push_back(shortPath);
    }
    redraw();
}

#define SQR(x) ((x)*(x))

void
IScissorDialog:: checkPathClosed()
{
    if (seeds.size() < 4)
        return;
    int x = seeds.back().x();
    int y = seeds.back().y();
    for (uint i=0; i<seeds.size()-3; ++i) {
        int distance = sqrt(SQR(seeds[i].x()-x) + SQR(seeds[i].y()-y));
        if (distance < (6/scale)) {
            seeds.pop_back();
            seeds.push_back(QPoint(seeds[i]));
            seed_mode = PATH_CLOSED;
            return;
        }
    }
}

// generate mask by floodfill inside path boundary
void
IScissorDialog:: getMaskedImage(QPoint clicked)
{
    QRgb white = qRgb(255,255,255);
    QRgb black = qRgb(0,0,0);
    QImage mask(image.width(), image.height(), QImage::Format_ARGB32);
    mask.fill(white);
    for (auto points : fullPath) {
        for (QPoint pt : points) {
            mask.setPixel(pt, black);
        }
    }
    //mask.save("mask.png");
    floodfill(mask, clicked, white, black);
    for (auto points : fullPath) {
        for (QPoint pt : points) {
            mask.setPixel(pt, white);
        }
    }
    if (smoothEdgesBtn->isChecked())
        gaussianBlur(mask, 3);
    if (image.format() != QImage::Format_ARGB32)
        image = image.convertToFormat(QImage::Format_ARGB32);
    for (int y=0; y<image.height(); y++){
        QRgb *row = (QRgb*)image.scanLine(y);
        QRgb *mask_row = (QRgb*)mask.constScanLine(y);
        for (int x=0; x<image.width(); x++){
            int clr = row[x];
            int alpha = qAlpha(clr) - qRed(mask_row[x]);
            if (alpha<0) alpha = 0;
            row[x] = qRgba(qRed(clr), qGreen(clr), qBlue(clr), alpha);
        }
    }
}


// marker for each of 8 neighbours.
// looks weird order, but helps to easily determine if it is diagonal or edge pixel.
// also, index of opposite side pixel is determined as k+4
/*
 * '---+---+---`
 * | 7 | 5 | 6 |
 * +---+---+---+
 * | 4 |   | 0 |
 * +---+---+---+
 * | 2 | 1 | 3 |
 * `---+---+---'
 */
/* sentinel to mark seed point in cost map */
#define  SEED_POINT     9

// how to reach a neighbour
const int link_offset[8][2] =
{//  x   y
  {  1,  0 },
  {  0,  1 },
  { -1,  1 },
  {  1,  1 },
  { -1,  0 },
  {  0, -1 },
  {  1, -1 },
  { -1, -1 },
};
// we use left 24 bytes to store pixel cost, and rest 4 bytes to store link dir
#define  PIXEL_COST(x)     ((x) >> 4)
#define  PIXEL_DIR(x)      ((x) & 0xf)

#define PACK(x, y)      ((((y) & 0xff) << 8) | ((x) & 0xff))
#define OFFSET(pixel)   ((int8_t)((pixel) & 0xff) + \
                        ((int8_t)(((pixel) & 0xff00) >> 8)) * buff_w)


void findOptimalPath(GradMap *grad_map, IntBuffer &dp_buff,
                    int x1, int y1, int xs, int ys)
{
    uint  pixel[8];
    int  cum_cost[8];
    int  link_cost[8];
    int  pixel_cost[8];
    int link, offset;
    int  buff_w = dp_buff.width;
    int  buff_h = dp_buff.height;

    uint *data = dp_buff.data;

    /*  what directions are we filling the array in according to?  */
    int dirx = (xs == x1) ? 1 : -1;
    int diry = (ys == y1) ? 1 : -1;
    int linkdir = (dirx * diry);

    int y = ys;

    for (int i = 0; i < buff_h; i++)
    {
      int x = xs;

      uint *d = data + (y-y1) * buff_w + (x-x1);

      for (int j = 0; j < buff_w; j++)
        {
          for (int k = 0; k < 8; k++)
            pixel[k] = 0;

          /*  Find the valid neighboring pixels  */
          /*  the previous pixel  */
          if (j)
            pixel[((dirx == 1) ? 4 : 0)] = PACK(-dirx, 0);

          /*  the previous row of pixels  */
          if (i)
            {
              pixel[((diry == 1) ? 5 : 1)] = PACK(0, -diry);

              link = (linkdir == 1) ? 3 : 2;
              if (j)
                pixel[((diry == 1) ? (link + 4) : link)] = PACK(-dirx, -diry);

              link = (linkdir == 1) ? 2 : 3;
              if (j != buff_w - 1)
                pixel[((diry == 1) ? (link + 4) : link)] = PACK(dirx, -diry);
            }

          /*  find the minimum cost of going through each neighbor to reach the
           *  seed point...
           */
          int min_cost = INT32_MAX;
          link = -1;
          for (int k = 0; k < 8; k ++)
            if (pixel[k])
              {
                link_cost[k] = grad_map->linkCost(xs + j*dirx, ys + i*diry, k);
                offset = OFFSET (pixel [k]);
                pixel_cost[k] = PIXEL_COST (d[offset]);
                cum_cost[k] = pixel_cost[k] + link_cost[k];
                if (cum_cost[k] < min_cost)
                  {
                    min_cost = cum_cost[k];
                    link = k;
                  }
              }
          /*  If anything can be done...  */
          if (link >= 0)
            {
              //  set the cumulative cost of this pixel and the new direction
              *d = (cum_cost[link] << 4) + link;

              /*  possibly change the links from the other pixels to this pixel...
               *  these changes occur if a neighboring pixel will receive a lower
               *  cumulative cost by going through this pixel.
               */
              for (int k = 0; k < 8; k ++)
                if (pixel[k] && k != link)
                  {
                    /*  if the cumulative cost at the neighbor is greater than
                     *  the cost through the link to the current pixel, change the
                     *  neighbor's link to point to the current pixel.
                     */
                    int new_cost = link_cost[k] + cum_cost[link];
                    if (pixel_cost[k] > new_cost)
                    {
                      /*  reverse the link direction   /--------------------\ */
                      offset = OFFSET (pixel[k]);
                      d[offset] = (new_cost << 4) + ((k > 3) ? k - 4 : k + 4);
                    }
                  }
            }
          /*  Set the seed point  */
          else if (!i && !j)
            {
              *d = SEED_POINT;
            }
          /*  increment the data pointer and the x counter  */
          d += dirx;
          x += dirx;
        }
      /*  increment the y counter  */
      y += diry;
    }
}

std::vector<QPoint>
            plotShortPath(IntBuffer *dp_buff, int x1, int y1, int target_x, int target_y)
{
    int width = dp_buff->width;

    std::vector<QPoint> shortPath;
    /*  Start the data pointer at the correct location  */
    uint *data = (uint *) dp_buff->data + (target_y - y1)*width + (target_x - x1);

    int x = target_x;
    int y = target_y;

    while (1)
    {
        shortPath.push_back(QPoint(x,y));

        int link = PIXEL_DIR (*data);
        if (link == SEED_POINT)
            return shortPath;

        x += link_offset[link][0];
        y += link_offset[link][1];
        data += link_offset[link][1] * width + link_offset[link][0];
    }
    return shortPath; // never reaches here
}


// Gradient Map
GradMap:: GradMap(QImage img)
{
    width = img.width();
    height = img.height();
    grad_mag = (uchar*)malloc(width*height);
    short int *grad_tmp = (short int*) malloc(width*height*sizeof(short int));
    // create gradient map using sobel filter
    int max_grad = 0;
    QRgb *data = (QRgb*)img.constScanLine(0);
    #pragma omp parallel for
    for (int y=1; y<height-1; y++) {
        QRgb *row1 = data + (y*width);// current row
        QRgb *row0 = row1 - width; // prev row
        QRgb *row2 = row1 + width; // next row
        for (int x=1; x<width-1; x++) {
            int clr00 = row0[x-1];
            int clr01 = row0[x];
            int clr02 = row0[x+1];
            int clr10 = row1[x-1];
            int clr12 = row1[x+1];
            int clr20 = row2[x-1];
            int clr21 = row2[x];
            int clr22 = row2[x+1];

            short int grad_x, grad_y, grad_x_g, grad_y_g, grad_x_b, grad_y_b;

            grad_x = qRed(clr00) + 2*qRed(clr10) + qRed(clr20) -
                    (qRed(clr02) + 2*qRed(clr12) + qRed(clr22));
            grad_y = qRed(clr00) + 2*qRed(clr01) + qRed(clr02) -
                    (qRed(clr20) + 2*qRed(clr21) + qRed(clr22));
            int grad = sqrt(grad_x*grad_x + grad_y*grad_y);
            // green channel
            grad_x_g = qGreen(clr00) + 2*qGreen(clr10) + qGreen(clr20) -
                    (qGreen(clr02) + 2*qGreen(clr12) + qGreen(clr22));
            grad_y_g = qGreen(clr00) + 2*qGreen(clr01) + qGreen(clr02) -
                    (qGreen(clr20) + 2*qGreen(clr21) + qGreen(clr22));
            int grad_g = sqrt(grad_x_g*grad_x_g + grad_y_g*grad_y_g);
            // blue channel
            grad_x_b = qBlue(clr00) + 2*qBlue(clr10) + qBlue(clr20) -
                    (qBlue(clr02) + 2*qBlue(clr12) + qBlue(clr22));
            grad_y_b = qBlue(clr00) + 2*qBlue(clr01) + qBlue(clr02) -
                    (qBlue(clr20) + 2*qBlue(clr21) + qBlue(clr22));
            int grad_b = sqrt(grad_x_b*grad_x_b + grad_y_b*grad_y_b);

            // use maximum value among 3 channels
            grad = MAX(grad, MAX(grad_g, grad_b));
            if (grad > max_grad)
                max_grad = grad;
            grad_tmp[y*width + x] = grad;
        }
    }
    #pragma omp parallel for
    for (int y=1; y<height-1; y++) {
        for (int x=1; x<width-1; x++) {
            grad_mag[y*width + x] = (1 - grad_tmp[y*width + x]/(float)max_grad)*255;
        }
    }
    // duplicate top and bottom border pixels
    memcpy(grad_mag+1, grad_mag+(width+1), width-2);
    memcpy(grad_mag+(width*(height-1)+1), grad_mag+(width*(height-2)+1), width-2);
    // duplicate left and right border
    for (int i=0; i<height; i++){
        uchar *row = grad_mag + (i*width);
        row[0] = row[1];
        row[width-1] = row[width-2];
    }
    free(grad_tmp);
}

#define SQRT2 1.414213562
float link_weight[8] = {1,1,SQRT2,SQRT2,1,1,SQRT2,SQRT2};

int
GradMap:: linkCost(int x, int y, int link)
{
    x += link_offset[link][0];
    y += link_offset[link][1];
    return (link_weight[link] * grad_mag[y*width + x]);
}

GradMap:: ~GradMap()
{
    free(grad_mag);
}

// Integer Buffer
IntBuffer:: IntBuffer(int w, int h) : width(w), height(h)
{
    data = (uint*) malloc(w*h*sizeof(uint));
}

IntBuffer:: ~IntBuffer()
{
    free(data);
}

/* Stack Based Scanline Floodfill
   Source : http://lodev.org/cgtutor/floodfill.html#Scanline_Floodfill_Algorithm_With_Stack
*/
void
floodfill(QImage &img, QPoint pos, QRgb oldColor, QRgb newColor)
{
    int x = pos.x();
    int y = pos.y();
    int w = img.width();
    int h = img.height();

    std::vector<QPoint> q;
    bool spanAbove, spanBelow;
    QRgb *row, *row_prev, *row_next;
    q.push_back(QPoint(x, y));

    while(!q.empty())
    {
        QPoint pt = q.back();
        q.pop_back();
        x = pt.x();
        y = pt.y();
        row = (QRgb*)img.scanLine(y);
        row_prev = (QRgb*)img.constScanLine(y-1);
        row_next = (QRgb*)img.constScanLine(y+1);
        while (x >= 0 && row[x] == oldColor) x--;
        x++;
        spanAbove = spanBelow = 0;
        while (x < w && row[x] == oldColor )
        {
            row[x] = newColor;
            if(!spanAbove && y > 0 && row_prev[x] == oldColor){
                q.push_back(QPoint(x, y - 1));
                spanAbove = 1;
            }
            else if (spanAbove && y > 0 && row_prev[x] != oldColor){
                spanAbove = 0;
            }
            if(!spanBelow && y < h - 1 && row_next[x] == oldColor){
                q.push_back(QPoint(x, y + 1));
                spanBelow = 1;
            }
            else if(spanBelow && y < h - 1 && row_next[x] != oldColor){
                spanBelow = 0;
            }
            x++;
        }
    }
}



BgColorDialog:: BgColorDialog(QWidget *parent) : QDialog(parent)
{
    this->resize(250, 120);
    this->setWindowTitle("Background Color");
    QVBoxLayout *vLayout = new QVBoxLayout(this);
    QLabel *label = new QLabel("Select Background Color :", this);
    QComboBox *combo = new QComboBox(this);
    combo->addItem("Transperant");
    combo->addItem("White Color");
    combo->addItem("Choose Other");
    QDialogButtonBox *btnBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel, Qt::Horizontal, this);
    vLayout->addWidget(label);
    vLayout->addWidget(combo);
    vLayout->addWidget(btnBox);
    connect(combo, SIGNAL(activated(int)), this, SLOT(setBgType(int)));
    connect(btnBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(btnBox, SIGNAL(rejected()), this, SLOT(reject()));
}

void
BgColorDialog:: setBgType(int type)
{
    bg_type = type;
    if (bg_type==2) {
        QColor clr = QColorDialog::getColor(QColor(bg_color), this);
        if (clr.isValid())
            bg_color = clr.rgb();
    }
    emit bgColorSelected(bg_type, bg_color);
}


void updateImageArea(QImage &dst, QImage &src, int pos_x, int pos_y)
{
    #pragma omp parallel for
    for (int y=0; y<src.height(); y++) {
        QRgb *row_dst, *row_src;
        #pragma omp critical
        { row_dst = (QRgb*)dst.scanLine(y+pos_y);
          row_src = (QRgb*)src.constScanLine(y); }
        for (int x=0; x<src.width(); x++) {
            row_dst[x+pos_x] = row_src[x];
        }
    }
}

