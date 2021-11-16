#pragma once
#include "plugin.h"
#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QRunnable>


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


class PhotoOptimizerDialog : public QDialog
{
    Q_OBJECT
public:
    QLabel *selectedPhotosLabel;
    QPushButton *selectDirBtn;
    QLabel *outDirLabel;
    QPushButton *changeOutDirBtn;
    QCheckBox *checkMegaPixel;
    QComboBox *megaPixelCombo;
    QCheckBox *checkResolution;
    QLineEdit *shortEdgeEdit;
    QLineEdit *longEdgeEdit;
    QLabel *statusbar;
    QPushButton *closeBtn;
    QPushButton *optimizeBtn;

    int short_edge;
    int long_edge;
    QStringList selected_files;
    QString target_dir;
    int task_index;
    int finished_count;
    int failed_count;
    bool cancel = false;

    PhotoOptimizerDialog(QWidget *parent);
    void dragEnterEvent(QDragEnterEvent *ev);
    void dropEvent(QDropEvent *ev);
    void setPhotos(QStringList filepaths, QString dir);
    void setDir(QString dir);
    void reject();

public slots:
    void toggleMaxResolution(bool checked);
    void toggleResizeTo(bool checked);
    void selectDir();
    void chooseTargetDir();
    void optimize();
    void onCompressFinish(bool success);
};

class CompressTask : public QObject, public QRunnable
{
    Q_OBJECT
public:
    QString filename;
    QString dst_dir;
    int short_edge;
    int long_edge;

    CompressTask(QString filename, QString dst_dir, int short_edge, int long_edge);
    void run();
signals:
    void compressFinished(bool success);
};
