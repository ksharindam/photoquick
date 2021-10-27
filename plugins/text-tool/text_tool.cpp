/*  This file is a part of PhotoQuick project, and is GNU GPLv3 licensed
    Copyright (C) 2021 Arindam Chaudhuri <ksharindam@gmail.com>
*/
#include "text_tool.h"
#include <QColorDialog>
#include <QDebug>
#include <cmath>


#define PLUGIN_NAME "Text Tool"
#define PLUGIN_MENU "Tools/Text Tool"
#define PLUGIN_VERSION "1.0"

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    Q_EXPORT_PLUGIN2(text-tool, ToolPlugin);
#endif


static void drawTextBox(QPainter &painter, TextBox textbox, bool border)
{
    if (textbox.bg_color) {
        painter.fillRect(textbox.rect, QColor(textbox.bg_color));
    }
    if (border) {
        QPen pen = QPen(Qt::black);
        pen.setWidth(1);
        painter.setPen(pen);
        painter.drawRect(textbox.rect);
    }

    if (not textbox.text.isEmpty()) {
        painter.setPen(QColor(textbox.text_color));
        QFont font = painter.font();
        font.setFamily(textbox.font);
        font.setPointSize(textbox.font_size);
        painter.setFont(font);
        int padding = textbox.font_size/2;
        QRect text_rect = textbox.rect.adjusted(padding, 0, -padding/2, 0);
        painter.drawText(text_rect, textbox.alignment|Qt::TextWordWrap, textbox.text);
    }
}


TextToolDialog:: TextToolDialog(QWidget *parent, QImage img) : QDialog(parent)
{
    this->image = img;
    drag_icon = QImage(":/drag.png");
    setupUi(this);
    deleteTextboxBtn->setEnabled(false);
    QHBoxLayout *layout = new QHBoxLayout(scrollAreaWidgetContents);
    layout->setContentsMargins(0, 0, 0, 0);
    canvas = new PaintCanvas(this);
    layout->addWidget(canvas);

    connect(acceptBtn, SIGNAL(clicked()), this, SLOT(accept()));
    connect(cancelBtn, SIGNAL(clicked()), this, SLOT(reject()));
    connect(canvas, SIGNAL(mousePressed(QPoint)), this, SLOT(onMousePress(QPoint)));
    connect(canvas, SIGNAL(mouseReleased(QPoint)), this, SLOT(onMouseRelease(QPoint)));
    connect(canvas, SIGNAL(mouseMoved(QPoint)), this, SLOT(onMouseMove(QPoint)));
    connect(newTextboxBtn, SIGNAL(clicked()), this, SLOT(newTextbox()));
    connect(deleteTextboxBtn, SIGNAL(clicked()), this, SLOT(deleteTextbox()));
    connect(plainTextEdit, SIGNAL(textChanged()), this, SLOT(onTextChange()));
    connect(fontComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onFontFamilyChange(int)));
    connect(fontSizeSlider, SIGNAL(valueChanged(int)), this, SLOT(onFontSizeChange(int)));
    connect(fontColorCombo, SIGNAL(activated(int)), this, SLOT(onFontColorChange(int)));
    connect(bgColorCombo, SIGNAL(activated(int)), this, SLOT(onBgColorChange(int)));
    connect(textAlignCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(onTextAlignmentChange(int)));

    // Load settings and init variables
    QSettings settings("photoquick", "plugins", this);
    settings.beginGroup("text-tool");
    fontComboBox->setCurrentIndex(settings.value("FontName", fontComboBox->currentIndex()).toInt());
    fontSizeSlider->setValue(settings.value("FontSize", 12).toInt());
    // font color
    other_color = QColor(settings.value("OtherColor", 0xff000000).toInt());
    int index = settings.value("FontColor", 0).toInt();
    fontColorCombo->setCurrentIndex(index);
    tmp_textbox.text_color = other_color.rgb();
    if (index < fontColorCombo->count()-1) {
        onFontColorChange(index);
    }
    // background color
    other_bg_color = QColor(settings.value("OtherBgColor", 0xffffffff).toInt());
    index = settings.value("BgColor", 1).toInt();
    bgColorCombo->setCurrentIndex(index);
    tmp_textbox.bg_color = other_bg_color.rgb();
    if (index < bgColorCombo->count()-1) {
        onBgColorChange(index);
    }
    textAlignCombo->setCurrentIndex(settings.value("TextAlign", 1).toInt());
    borderBtn->setChecked(settings.value("Border", true).toBool());
    // scale the canvas to fit to scrollarea width
    int available_w = 1020 - frame->width() - 4;
    int img_w = img.width();
    float scale_factor = 1.0;
    while (img_w > available_w) {
        scale_factor /= 1.5;
        img_w = roundf(scale_factor*img.width());
    }
    scaleBy(scale_factor);
    newTextbox();
}

void
TextToolDialog:: scaleBy(float scale)
{
    this->scale = scale;
    if (scale == 1.0)
        image_scaled = image;
    else {
        Qt::TransformationMode tfm_mode = scale<1.0? Qt::SmoothTransformation: Qt::FastTransformation;
        image_scaled = image.scaled(scale*image.width(), scale*image.height(),
                        Qt::IgnoreAspectRatio, tfm_mode);
    }
    redraw();
}

void
TextToolDialog:: redraw()
{
    QPixmap pm = QPixmap::fromImage(image_scaled).copy();
    for (int i=0; i<textboxes.count(); i++) {
        TextBox &textbox = textboxes[i];
        painter.begin(&pm);
        drawTextBox(painter, textbox, true);
        if (i == textbox_entered) {
            painter.drawImage(textbox.rect.bottomRight()-QPoint(15,15), drag_icon);
        }
        else if (i==textboxes.count()-1) {// highlight if last textbox
            QPen pen(Qt::black, 2, Qt::DashDotLine);
            painter.setPen(pen);
            painter.setBrush(Qt::NoBrush);
            painter.drawRect(textbox.rect);
        }
        painter.end();
    }
    canvas->setPixmap(pm);
}

void
TextToolDialog:: updateCurrentTextbox()
{
    if (textboxes.isEmpty())
        return;
    textboxes.last().font = tmp_textbox.font;
    textboxes.last().font_size = tmp_textbox.font_size;
    textboxes.last().text_color = tmp_textbox.text_color;
    textboxes.last().bg_color = tmp_textbox.bg_color;
    textboxes.last().alignment = tmp_textbox.alignment;
    redraw();
}

void
TextToolDialog:: onMousePress(QPoint pos)
{
    click_pos = pos;
    mouse_pressed = true;

    if (new_textbox){
        canvas_copy = canvas->pixmap()->copy();
        tmp_textbox.rect = QRect(pos,pos);
        return;
    }
    for (int i=textboxes.size()-1; i>=0; i--) {
        if (textboxes[i].rect.contains(pos)){
            if (i!=textboxes.size()-1) {// if not top, move to top
                textboxes.move(i, textboxes.size()-1);
                textbox_entered = textboxes.size()-1;
                redraw();
            }
            QRect rect = textboxes.last().rect;
            rect.setTopLeft(rect.bottomRight()-QPoint(16,16));
            click_mode = rect.contains(pos) ? MODE_RESIZE : MODE_MOVE;
            break;
        }
    }
}

void
TextToolDialog:: onMouseRelease(QPoint /*pos*/)
{
    mouse_pressed = false;
    if (new_textbox) {
        // new textbox must have minimum width and height
        if (tmp_textbox.rect.width()<48) {
            tmp_textbox.rect.setWidth(tmp_textbox.font_size*10);
        }
        if (tmp_textbox.rect.height()<16) {
            tmp_textbox.rect.setHeight(tmp_textbox.font_size*2);
        }
        canvas->unsetCursor();
        canvas_copy = QPixmap();// frees memory
        textboxes.append(tmp_textbox);
        deleteTextboxBtn->setEnabled(true);
        new_textbox = false;
        plainTextEdit->clear();
        plainTextEdit->setFocus();
        statusbar->setText("Tip : Type text or drag textbox to move or resize");
    }
    else if (click_mode!=MODE_NONE) {
        // clicking on textbox will focus textedit
        if (not textboxes.isEmpty()) {
            plainTextEdit->setPlainText(textboxes.last().text);
            plainTextEdit->moveCursor(QTextCursor::End);
            plainTextEdit->setFocus();
        }
    }
    click_mode = MODE_NONE;
}

void
TextToolDialog:: onMouseMove(QPoint pos)
{
    if ( !mouse_pressed) {
        if (!textboxes.isEmpty()){
            int i;
            for (i=textboxes.count()-1; i>=0; i--) {
                if (textboxes[i].rect.contains(pos))
                    break;
            }
            if (i!=textbox_entered) {// entered or left textbox
                textbox_entered = i;
                redraw();
            }
        }
        goto end;
    }
    if (new_textbox) {
        tmp_textbox.rect = QRect(click_pos, pos).normalized();
        QPixmap pm = canvas_copy.copy();
        painter.begin(&pm);
        drawTextBox(painter, tmp_textbox, true);
        painter.end();
        canvas->setPixmap(pm);
    }
    else {
        if (click_mode==MODE_MOVE) {
            textboxes.last().rect.translate(pos-old_pos);
            redraw();
        }
        else if (click_mode==MODE_RESIZE){
            textboxes.last().rect.adjust(0,0, pos.x()-old_pos.x(), pos.y()-old_pos.y());
            redraw();
        }
    }
end:
    old_pos = pos;
}

void
TextToolDialog:: newTextbox()
{
    new_textbox = true;
    canvas->setCursor(Qt::CrossCursor);
    statusbar->setText("Tip : Click and drag to create new textbox");
}

void
TextToolDialog:: deleteTextbox()
{
    textboxes.pop_back();
    if (textboxes.isEmpty()) {
        deleteTextboxBtn->setEnabled(false);
        newTextbox();
    }
    redraw();
}

void
TextToolDialog:: onTextChange()
{
    if (textboxes.isEmpty()) return;
    textboxes.last().text = plainTextEdit->toPlainText();
    redraw();
}

void
TextToolDialog:: onTextAlignmentChange(int index)
{
    uint alignments[3] = { Qt::AlignLeft|Qt::AlignVCenter, Qt::AlignCenter, Qt::AlignRight|Qt::AlignVCenter};
    tmp_textbox.alignment = alignments[index];
    updateCurrentTextbox();
}

void
TextToolDialog:: onFontFamilyChange(int index)
{
    tmp_textbox.font = fontComboBox->itemText(index);
    updateCurrentTextbox();
}

void
TextToolDialog:: onFontSizeChange(int val)
{
    fontSizeLabel->setText(QString("Size : %1").arg(val));
    tmp_textbox.font_size = val;
    updateCurrentTextbox();
}

void
TextToolDialog:: onFontColorChange(int index)
{
    if (index>4) {
        other_color = QColorDialog::getColor(other_color, this, "Choose Text Color");
        tmp_textbox.text_color = other_color.rgb();
    }
    else {
        QRgb font_colors[6] = {0xff000000, 0xffffffff, 0xffff0000, 0xff0000ff, 0xff00ff00};
        tmp_textbox.text_color = font_colors[index];
    }
    updateCurrentTextbox();
}

void
TextToolDialog:: onBgColorChange(int index)
{
    if (index>2) {
        other_bg_color = QColorDialog::getColor(other_bg_color, this, "Choose Text Color");
        tmp_textbox.bg_color = other_bg_color.rgb();
    }
    else {
        QRgb bg_colors[4] = {0x00000000, 0xffffffff, 0xff000000};
        tmp_textbox.bg_color = bg_colors[index];
    }
    updateCurrentTextbox();
}


void
TextToolDialog:: accept()
{
    painter.begin(&image);
    painter.scale(1.0/scale, 1.0/scale);
    for (TextBox &textbox : textboxes) {
        drawTextBox(painter, textbox, borderBtn->isChecked());
    }
    painter.end();

    QSettings settings("photoquick", "plugins", this);
    settings.beginGroup("text-tool");
    settings.setValue("FontName", fontComboBox->currentIndex());
    settings.setValue("FontSize", tmp_textbox.font_size);
    settings.setValue("FontColor", fontColorCombo->currentIndex());
    settings.setValue("OtherColor", other_color.rgb());
    settings.setValue("BgColor", bgColorCombo->currentIndex());
    settings.setValue("OtherBgColor", other_bg_color.rgb());
    settings.setValue("TextAlign", textAlignCombo->currentIndex());
    settings.setValue("Border", borderBtn->isChecked());

    QDialog::accept();
}

void
TextToolDialog:: keyPressEvent(QKeyEvent *ev)
{
    // prevent closing dialog on accidental Esc key press
    if (ev->key() != Qt::Key_Escape)
        return QDialog::keyPressEvent(ev);
    ev->accept();
}


// ******************* Paint Canvas ******************
PaintCanvas:: PaintCanvas(QWidget *parent) : QLabel(parent)
{
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setMouseTracking(true);
}

void
PaintCanvas:: mousePressEvent(QMouseEvent *ev)
{
    emit mousePressed(ev->pos());
}

void
PaintCanvas:: mouseMoveEvent(QMouseEvent *ev)
{
    emit mouseMoved(ev->pos());
}

void
PaintCanvas:: mouseReleaseEvent(QMouseEvent *ev)
{
    emit mouseReleased(ev->pos());
}


// ************* ----------- The Plugin Class ---------- ***************

QString ToolPlugin:: menuItem()
{
    return QString(PLUGIN_MENU);
}

void ToolPlugin:: onMenuClick()
{
    TextToolDialog *dialog = new TextToolDialog(data->window, data->image);
    dialog->resize(1020, data->max_window_h);
    if (dialog->exec()==QDialog::Accepted) {
        data->image = dialog->image;
        emit imageChanged();
    }
}
