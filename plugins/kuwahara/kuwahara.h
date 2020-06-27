#pragma once
#include "plugin.h"

class FilterPlugin : public QObject, Plugin
{
    Q_OBJECT
    Q_INTERFACES(Plugin)

public:
    QStringList menuItems();

    void handleAction(QAction *action, int action_type);

public slots:
    void filterKuwahara();
    void filterPencilSketch();

signals:
    void imageChanged();
    void optimumSizeRequested();
    void sendNotification(QString title, QString message);
};

