#pragma once
#include "plugin.h"
#include "ui_text_tool_dialog.h"
#include <QPainter>
#include <QMouseEvent>
#include <QSettings>

class ToolPlugin : public QObject, Plugin
{
    Q_OBJECT
    Q_INTERFACES(Plugin)
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    Q_PLUGIN_METADATA(IID Plugin_iid)
#endif

public:
    QString menuItem();

public slots:
    void onMenuClick();

signals:
    void imageChanged();
    void optimumSizeRequested();
    void sendNotification(QString title, QString message);
};



// It is a drawing area

class PaintCanvas : public QLabel
{
    Q_OBJECT
public:
    PaintCanvas(QWidget *parent);
    void mousePressEvent(QMouseEvent *ev);
    void mouseMoveEvent(QMouseEvent *ev);
    void mouseReleaseEvent(QMouseEvent *ev);
signals:
    void mousePressed(QPoint);
    void mouseMoved(QPoint);
    void mouseReleased(QPoint);
};

class TextBox
{
public:
    QRect rect;
    QString text;
    int font_size=12;
    bool bold=false;
    bool italic=false;
    QRgb text_color=0xff000000;// ARGB black
    QRgb bg_color=0xffffffff;
    int alignment = Qt::AlignCenter;
    QString font;
};

enum {
    MODE_NONE, MODE_MOVE, MODE_RESIZE
};

class TextToolDialog : public QDialog, public Ui_TextToolDialog
{
    Q_OBJECT
public:
    QImage image; // original unchanged image
    QImage image_scaled;
    QPixmap canvas_copy;
    QImage drag_icon;
    QPainter painter;
    PaintCanvas *canvas;
    TextBox tmp_textbox;
    QList<TextBox> textboxes;
    QColor other_color;
    QColor other_bg_color = Qt::white;

    float scale = 1.0;
    bool mouse_pressed = false;
    QPoint click_pos;
    QPoint old_pos;
    bool new_textbox = true;
    int click_mode = MODE_NONE;
    int textbox_entered = -1;// textbox index into which mouse cursor entered, -1=no textbox

    TextToolDialog(QWidget *parent, QImage img);
    void scaleBy(float scale);
    void redraw();
    void updateCurrentTextbox();
    void keyPressEvent(QKeyEvent *ev);
    void accept();
public slots:
    void newTextbox();
    void deleteTextbox();
    void onTextChange();
    void onFontFamilyChange(int index);
    void onFontSizeChange(int val);
    void enableBoldFont(bool enable);
    void enableItalicFont(bool enable);
    void onFontColorChange(int index);
    void onBgColorChange(int index);
    void onTextAlignmentChange(int index);
    void onMousePress(QPoint pos);
    void onMouseRelease(QPoint pos);
    void onMouseMove(QPoint pos);
};
