#pragma once
#include "canvas.h"
#include <QPluginLoader>
#include <QAction>

// Action Type
enum {
    ACTION_MENU,
    ACTION_SHORTCUT
};
/*
 In an interface, if a virtual function does not have definition,
 it must be initialized to 0. And those functions must be defined in
 the implementation
*/

class Plugin
{
public:
    Canvas *canvas = 0;   // access current QImage and filename from this
    int max_window_w = 1200; // maximum window width
    int max_window_h = 700;

    virtual void initialize(Canvas *canvas, int max_window_w, int max_window_h) {
        this->canvas = canvas;
        this->max_window_w = max_window_w;
        this->max_window_h = max_window_h;
    }

    // must implement onMenuClick() if this is implemented
    virtual QString     menuItem() { return QString(); }

    // must implement handleAction() if you override this function
    virtual QStringList menuItems() { return QStringList(); }

    // must implement handleAction() if you override this
    virtual QStringList getShortcuts() { return QStringList(); }

    // use QAction->text() to check which menu item is this, and then connect slot
    // use QAction->shortcut()->toString() to check which shortcut is this
    virtual void handleAction(QAction*, int /*Action Type*/) {};

    virtual ~Plugin() {}

//public slots:
    virtual void onMenuClick() {}  // this must be a slot

    // the plugins must declare these signals
signals:
    virtual void imageChanged() = 0;    // show the updated image on canvas
    virtual void optimumSizeRequested() = 0; // scale image and window to fit screen
    virtual void sendNotification(QString title, QString message) = 0;
};


Q_DECLARE_INTERFACE(Plugin, "photoquick.Plugin");
