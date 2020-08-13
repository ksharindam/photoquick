#pragma once
/* This file is part of photoquick project, which is GPLv3 licensed */
#include <QtPlugin>
#include <QPluginLoader>
#include <QAction>
#include <QImage>
#include <QWidget>

typedef struct {
    QImage image;
    bool animation;
    QString filename;
    QWidget *window;
    int max_window_w;
    int max_window_h;
} ImageData;

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
    ImageData *data;   // access current QImage and filename from this

    virtual void initialize(ImageData *data) {
        this->data = data;
    }

    /* returns a single menu to be created (e.g return "Filter/Color/Invert").
    the menu signal is connected to onMenuClick() slot. handleAction() does not get called.
    Must implement onMenuClick() if this is implemented */
    virtual QString     menuItem() { return QString(); }

    /* returns list of one or more menus to be created. handleAction() gets called each
    time when a menu item is added. Must implement handleAction() to connect slots, or
    to set shortcut using QAction->setShortcut() */
    virtual QStringList menuItems() { return QStringList(); }

    /* returns list of shortcuts to be added. handleAction() gets called each time when
    a shortcut action is created. Must implement handleAction() if you override this */
    virtual QStringList getShortcuts() { return QStringList(); }

    /* it gets called each time a menu or shortcut is added, use it to connect slots
    use QAction->text() to check which menu item is this, and then connect slot
    use QAction->shortcut()->toString() to check which shortcut is this */
    virtual void handleAction(QAction*, int /*Action Type*/) {};

    virtual ~Plugin() {}

//public slots:
    virtual void onMenuClick() {}  // this must be a slot

    // the plugins must declare these signals
signals:
    virtual void imageChanged() = 0;    // show the updated image on canvas
    virtual void optimumSizeRequested() = 0; // scale image and window to fit to screen
    virtual void sendNotification(QString title, QString message) = 0;
};


Q_DECLARE_INTERFACE(Plugin, "photoquick.Plugin");
