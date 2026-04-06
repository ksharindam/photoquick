/*  This file is a part of PhotoQuick Plugins project, and is GNU GPLv3 licensed
    Copyright (C) 2021-2023 Arindam Chaudhuri <ksharindam@gmail.com>
*/
#include "photo_optimizer.h"
#include "exif.cpp"// bad idea ? but we can not link exif.o from main program
#include <unistd.h> // dup()
#include <QBuffer>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QFileDialog>
#include <QThreadPool>
#include <QMouseEvent>
#include <QMimeData>
#include <QUrl>
#include <QDebug>


#define PLUGIN_NAME "Photo Optimizer"
#define PLUGIN_MENU "Tools/Photos Compressor"
#define PLUGIN_VERSION "1.1"

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    Q_EXPORT_PLUGIN2(photo-optimizer, ToolPlugin);
#endif


// ********************** Photo Optimizer Dialog ******************** //

QImage loadImage(QString fileName);
bool saveJpegWithExif(QImage img, int quality, QString out_filename, QString exif_filename);
bool add_thumbnail_to_jpg(QImage thumbnail, QString filename, QString out_filename);
FILE* qfopen(QString filename, const char *mode);

PhotoOptimizerDialog:: PhotoOptimizerDialog(QWidget *parent) : QDialog(parent)
{
    this->setWindowTitle("Camera Photos Compressor");
    this->resize(454, 316);
    this->setAcceptDrops(true);
    QVBoxLayout *dialogLayout = new QVBoxLayout(this);

    QGroupBox *groupBox1 = new QGroupBox("Drag && Drop Photos or a Folder", this);
    selectedPhotosLabel = new QLabel("0 Photos in current folder", groupBox1);
    selectDirBtn = new QPushButton("Choose Folder", groupBox1);

    QHBoxLayout *hLayout1 = new QHBoxLayout(groupBox1);
    hLayout1->addWidget(selectedPhotosLabel);
    hLayout1->addWidget(selectDirBtn);
    hLayout1->setStretch(0,1);

    QGroupBox *groupBox2 = new QGroupBox("Save to folder", this);
    outDirLabel = new QLabel("Choose a destination folder", groupBox2);
    changeOutDirBtn = new QPushButton("Change", groupBox2);

    QHBoxLayout *hLayout2 = new QHBoxLayout(groupBox2);
    hLayout2->addWidget(outDirLabel);
    hLayout2->addWidget(changeOutDirBtn);
    hLayout2->setStretch(0,1);

    QGroupBox *groupBox3 = new QGroupBox("Resize larger Photos", this);
    checkMegaPixel = new QCheckBox("Use MegaPixel :", groupBox3);
    megaPixelCombo = new QComboBox(groupBox3);
    QStringList resolutions = {"12 MegaPixel", "10 MegaPixel", "8 MegaPixel",
                                "5 MegaPixel", "2 MegaPixel"};
    megaPixelCombo->addItems(resolutions);
    megaPixelCombo->setCurrentIndex(2);
    megaPixelCombo->setEnabled(false);
    checkResolution = new QCheckBox("Use Resolution :", groupBox3);
    shortEdgeEdit = new QLineEdit(groupBox3);
    shortEdgeEdit->setPlaceholderText("Short Edge");
    shortEdgeEdit->setValidator(new QIntValidator(480, 10000, shortEdgeEdit));
    shortEdgeEdit->setEnabled(false);
    longEdgeEdit = new QLineEdit(groupBox3);
    longEdgeEdit->setPlaceholderText("Long Edge");
    shortEdgeEdit->setValidator(new QIntValidator(640, 10000, shortEdgeEdit));
    longEdgeEdit->setEnabled(false);

    QGridLayout *gridLayout = new QGridLayout(groupBox3);
    gridLayout->addWidget(checkMegaPixel, 0, 0, 1, 1);
    gridLayout->addWidget(megaPixelCombo, 0, 1, 1, 1);
    gridLayout->addWidget(checkResolution, 1, 0, 1, 1);
    gridLayout->addWidget(shortEdgeEdit, 1, 1, 1, 1);
    gridLayout->addWidget(longEdgeEdit, 2, 1, 1, 1);
    gridLayout->setColumnStretch(0,1);

    QWidget *widget = new QWidget(this);
    statusbar = new QLabel(widget);
    closeBtn = new QPushButton("Close", widget);
    optimizeBtn = new QPushButton("Compress", widget);

    QHBoxLayout *hLayout3 = new QHBoxLayout(widget);
    hLayout3->setContentsMargins(6,6,6,0);
    hLayout3->addWidget(statusbar);
    hLayout3->addWidget(optimizeBtn);
    hLayout3->addWidget(closeBtn);
    hLayout3->setStretch(0,1);

    dialogLayout->addWidget(groupBox1);
    dialogLayout->addWidget(groupBox2);
    dialogLayout->addWidget(groupBox3);
    dialogLayout->addWidget(widget);

    // connect signals
    connect(selectDirBtn, SIGNAL(clicked()), this, SLOT(selectDir()));
    connect(changeOutDirBtn, SIGNAL(clicked()), this, SLOT(chooseTargetDir()));
    connect(checkMegaPixel, SIGNAL(toggled(bool)), this, SLOT(toggleMaxResolution(bool)));
    connect(checkResolution, SIGNAL(toggled(bool)), this, SLOT(toggleResizeTo(bool)));
    connect(closeBtn, SIGNAL(clicked()), this, SLOT(reject()));
    connect(optimizeBtn, SIGNAL(clicked()), this, SLOT(optimize()));

    setDir(QDir::currentPath());
}

void
PhotoOptimizerDialog:: toggleMaxResolution(bool checked)
{
    megaPixelCombo->setEnabled(checked);
    if (checked && checkResolution->isChecked())
        checkResolution->setChecked(false);
}

void
PhotoOptimizerDialog:: toggleResizeTo(bool checked)
{
    shortEdgeEdit->setEnabled(checked);
    longEdgeEdit->setEnabled(checked);
    if (checked && checkMegaPixel->isChecked())
        checkMegaPixel->setChecked(false);
}

void
PhotoOptimizerDialog:: setPhotos(QStringList files, QString dir)
{
    if (files.isEmpty())
        return;
    selected_files = files;
    if (dir.isEmpty())
        dir = QFileInfo(selected_files[0]).dir().path();
    selectedPhotosLabel->setText(QString("%1 photos in \"%2\"").arg(
                                selected_files.count()).arg(QDir(dir).dirName()));
    target_dir = dir + "-compressed";
    QFontMetrics fontMetrics(outDirLabel->font());
    QString text = fontMetrics.elidedText(target_dir, Qt::ElideMiddle, 350);
    outDirLabel->setText(text);
}

void
PhotoOptimizerDialog:: setDir(QString dir)
{
    QStringList filenames = QDir(dir).entryList({"*.jpg", "*.jpeg"});
    filenames.sort();
    QStringList filepaths;
    for (QString filename : filenames) {
        filepaths.append(dir + "/" + filename);
    }
    setPhotos(filepaths, dir);
}

void
PhotoOptimizerDialog:: selectDir()
{
    QString dir = QFileDialog::getExistingDirectory(this, "Select Folder of Photos", "");
    if (dir.isEmpty())
        return;
    setDir(dir);
}

void
PhotoOptimizerDialog:: chooseTargetDir()
{
    QString dir = QFileDialog::getExistingDirectory(this, "Select Target Folder",
                                                 "", QFileDialog::ShowDirsOnly);
    if (dir.isEmpty())
        return;
    target_dir = dir;
    QFontMetrics fontMetrics(outDirLabel->font());
    dir = fontMetrics.elidedText(dir, Qt::ElideMiddle, outDirLabel->width()-2);
    outDirLabel->setText(dir);
}

void
PhotoOptimizerDialog:: optimize()
{
    if (target_dir.isEmpty())
        return;
    if (not QDir(target_dir).exists()) {
        if (not QDir(target_dir).mkpath(".")){
            statusbar->setText("Failed to create target folder");
            return;
        }
    }
    if (not QFileInfo(target_dir).isWritable()){
        statusbar->setText("Can not save to target folder");
        return;
    }
    short_edge = 0;
    long_edge = 0;
    if (checkMegaPixel->isChecked()){
        switch (megaPixelCombo->currentIndex()) {
        case 0:
            short_edge = 3000;// 12M, Redmi Note 5 pro
            long_edge = 4000;
            break;
        case 1:
            short_edge = 2736;// 10M, Sony DSC W830
            long_edge = 3648;
            break;
        case 2:
            short_edge = 2448;// 8M, Samsung J2 pro
            long_edge = 3264;
            break;
        case 3:
            short_edge = 1920;// 5M, Samsung Galaxy Core 2
            long_edge = 2560;
            break;
        case 4:
            short_edge = 1200;// 2M, Samsung Galaxy Star
            long_edge = 1600;
            break;
        default:
            break;
        }
    }
    else if (checkResolution->isChecked()){
        short_edge = shortEdgeEdit->text().toInt();
        long_edge = longEdgeEdit->text().toInt();
    }
    optimizeBtn->setEnabled(false);
    QThreadPool *pool = QThreadPool::globalInstance();
    finished_count = 0;
    failed_count = 0;
    for (task_index=0; task_index<selected_files.count() &&
                        task_index<pool->maxThreadCount(); task_index++) {
        CompressTask *task = new CompressTask(selected_files[task_index], target_dir,
                                            short_edge, long_edge);
        connect(task, SIGNAL(compressFinished(bool)), this, SLOT(onCompressFinish(bool)));
        pool->start(task);
    }
    statusbar->setText("Compressing...");
}

void
PhotoOptimizerDialog:: onCompressFinish(bool success)
{
    if (cancel)// while closing dialog
        return;
    finished_count++;
    if (not success)
        failed_count++;
    if (finished_count==selected_files.count()){
        statusbar->setText(QString("Finished : %1 successful, %2 failed").arg(
                            finished_count-failed_count).arg(failed_count));
        optimizeBtn->setEnabled(true);
        return;
    }
    statusbar->setText(QString("Compressing... %1/%2").arg(finished_count).arg(selected_files.count()));
    if (task_index<selected_files.count()){
        CompressTask *task = new CompressTask(selected_files[task_index], target_dir,
                                                short_edge, long_edge);
        connect(task, SIGNAL(compressFinished(bool)), this, SLOT(onCompressFinish(bool)));
        QThreadPool::globalInstance()->start(task);
        task_index++;
    }
}

void
PhotoOptimizerDialog:: dragEnterEvent(QDragEnterEvent *ev)
{
    if (ev->mimeData()->hasUrls())
        ev->acceptProposedAction();
    else
        ev->ignore();
}

void
PhotoOptimizerDialog:: dropEvent(QDropEvent *ev)
{
    if (not ev->mimeData()->hasUrls() ) {
        ev->ignore();
        return;
    }
    QStringList files;
    for (const QUrl &url : ev->mimeData()->urls() ) {
        QString path = url.toLocalFile();
        if (not path.isEmpty()) {
            // we allow only one folder to be dropped
            if (QFileInfo(path).isDir()){
                setDir(path);
                ev->ignore();
                return;
            }
            if (path.endsWith(".jpg", Qt::CaseInsensitive) ||
                path.endsWith(".jpeg", Qt::CaseInsensitive) ){
                files.append(path);
            }
        }
    }
    setPhotos(files, "");
    ev->ignore();
}

void
PhotoOptimizerDialog:: reject()
{
    cancel = true;
    QDialog::reject();
}



CompressTask:: CompressTask(QString file_name, QString out_dir,
                            int short_edge_len, int long_edge_len) : QObject()
{
    filename = file_name;
    dst_dir = out_dir;
    short_edge = short_edge_len;
    long_edge = long_edge_len;
}

/* Actually we dont do any compression. As mobile and digital cameras
   save photos unoptimized, so if we load and save photos with default
   quality, file size is greatly reduced (becomes 1/3 or 1/4 of original).
*/
void
CompressTask:: run()
{
    QImage img = loadImage(filename);
    if (img.isNull()) {// failed
        emit compressFinished(false);
        return;
    }
    if (short_edge | long_edge) {// need to resize
        int w = short_edge;// portrait
        int h = long_edge;
        if (img.width()>img.height()) {// landscape
            w = long_edge;
            h = short_edge;
        }
        if (w && !h){
            h = w * img.height()/img.width();
        }
        else if (h && !w) {
            w = h * img.width()/img.height();
        }
        // to prevent quality loss, dont resize if source image is not
        // 20% larger than result image
        if (img.width()*img.height() > 1.2*w*h)
            img = img.scaled(w, h, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
    QString out_filename = dst_dir + "/" + QFileInfo(filename).fileName();
    QString tmp_name = out_filename + ".txt";
    bool ok = saveJpegWithExif(img, -1, tmp_name, filename);
    if (ok) {
        if (QFileInfo(out_filename).exists())// copy/move fails if file already exists
            QFile(out_filename).remove();
        if (QFileInfo(tmp_name).size()<QFileInfo(filename).size()) {
            QFile(tmp_name).rename(out_filename);
        }
        else {// already compressed, add a thumbnail so filemanager can display very fast
            if (img.width()*img.height() >= 1000000) {
                QImage thumb = img.width()>img.height() ? img.scaledToWidth(160) : img.scaledToHeight(160);
                img = QImage();// free memory
                ok = add_thumbnail_to_jpg(thumb, filename, out_filename);
            }
            else QFile(filename).copy(out_filename);
        }
        if (QFileInfo(tmp_name).exists())
            QFile(tmp_name).remove();
    }
    emit compressFinished(ok);
}



// Creates a FILE* from QString filename
FILE* qfopen(QString filename, const char *mode)
{
    QIODevice::OpenMode io_mode = QIODevice::NotOpen;
    if (QString(mode).contains("r"))
        io_mode |= QIODevice::ReadOnly;
    if (QString(mode).contains("w"))
        io_mode |= QIODevice::WriteOnly;

    QFile qf(filename);
    if (!qf.open(io_mode))
        return NULL;
    int fd = dup(qf.handle());
    qf.close();
    FILE *f = fdopen(fd, mode);
    return f;
}

// load an image from file
QImage loadImage(QString fileName)
{
    QImage img(fileName);
    if (img.isNull()) return img;
    // Converted because filters can only be applied to RGB32 or ARGB32 image
    if (img.hasAlphaChannel() && img.format()!=QImage::Format_ARGB32)
        img = img.convertToFormat(QImage::Format_ARGB32);
    else if (!img.hasAlphaChannel() and img.format()!=QImage::Format_RGB32)
        img = img.convertToFormat(QImage::Format_RGB32);
    // Get jpg orientation
    FILE *f = qfopen(fileName, "rb");
    int orientation = getOrientation(f);
    fclose(f);
    // rotate if required
    QTransform transform;
    switch (orientation) {
        case 6:
            return img.transformed(transform.rotate(90));
        case 3:
            return img.transformed(transform.rotate(180));
        case 8:
            return img.transformed(transform.rotate(270));
    }
    return img;
}


bool saveJpegWithExif(QImage img, int quality, QString out_filename, QString exif_filename)
{
    // image too small, do not add thumbnail
    if (img.width()*img.height()<300000)
        return img.save(out_filename, "JPEG", quality);

    FILE *infile = qfopen(exif_filename, "r");
    if (!infile)
        return img.save(out_filename, "JPEG", quality);
    ExifInfo exif;
    exif_read(exif, infile);
    if (exif.count(0x0112)>0) {//fix Tag_Orientation
        exif[0x0112].integer = 1;
    }
    fclose(infile);
    // if image is >1M, even if exif empty, we add exif to add thumbnail
    if (exif.empty() && (img.width()*img.height()<1000000))
        return img.save(out_filename, "JPEG", quality);

    FILE *out = qfopen(out_filename, "w");
    if (!out) {
        exif_free(exif);
        return false;
    }
    QBuffer buff;
    buff.open(QIODevice::WriteOnly);
    img.save(&buff, "JPEG", quality);
    bool ok;
    if (img.width()*img.height()>=1000000){// add a thumbnail
        QBuffer thumb_buff;
        thumb_buff.open(QIODevice::WriteOnly);
        // recommended thumbnail resolution is 160x120
        QImage thumb = img.width()>img.height() ? img.scaledToWidth(160) : img.scaledToHeight(160);
        thumb.save(&thumb_buff, "JPEG");
        ok = write_jpeg_with_exif(buff.buffer().data(), buff.size(),
                                thumb_buff.buffer().data(), thumb_buff.size(), exif, out);
        thumb_buff.buffer().clear();
    }
    else {
        ok = write_jpeg_with_exif(buff.buffer().data(), buff.size(), NULL, 0, exif, out);
    }
    fclose(out);
    buff.buffer().clear();
    exif_free(exif);
    return ok;
}

bool add_thumbnail_to_jpg(QImage thumbnail, QString filename, QString out_filename)
{
    // read exif from infile
    FILE *infile = qfopen(filename, "r");
    if (!infile) {
        return false;
    }
    ExifInfo exif;
    exif_read(exif, infile);
    fclose(infile);
    // create outfile
    FILE *out = qfopen(out_filename, "w");
    if (!out) {
        return false;
    }
    // create thumbnail
    QBuffer thumb_buff;
    thumb_buff.open(QIODevice::WriteOnly);
    thumbnail.save(&thumb_buff, "JPEG");
    // read main image
    QFile file(filename);
    file.open(QIODevice::ReadOnly);
    QByteArray bArr = file.readAll();
    file.close();
    bool ok = write_jpeg_with_exif(bArr.data(), bArr.size(),
                            thumb_buff.buffer().data(), thumb_buff.size(), exif, out);
    thumb_buff.buffer().clear();
    bArr.clear();
    fclose(out);
    exif_free(exif);
    return ok;
}

// ************* ----------- The Plugin Class ---------- ***************

QString ToolPlugin:: menuItem()
{
    return QString(PLUGIN_MENU);
}

void ToolPlugin:: onMenuClick()
{
    PhotoOptimizerDialog *dialog = new PhotoOptimizerDialog(data->window);
    dialog->exec();
}
