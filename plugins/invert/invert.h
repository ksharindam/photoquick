#pragma once
#include "plugin.h"

// create the plugin class by inheriting both QObject and Plugin
class FilterPlugin : public QObject, Plugin
{
    // must add these macros (no semicolon)
    Q_OBJECT
    Q_INTERFACES(Plugin)
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    Q_PLUGIN_METADATA(IID Plugin_iid)
#endif

public:
    QString menuItem(); // return the menu path for the menu item to be added

public slots:
    void onMenuClick(); // if menuItem() is defined, must define this slot

signals:
    void imageChanged();
    void optimumSizeRequested();
    void sendNotification(QString title, QString message);
};
