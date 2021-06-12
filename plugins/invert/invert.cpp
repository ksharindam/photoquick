#include "invert.h"
#include <QDebug>

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
// first parameter is name of plugin, usually same as the library file name
    Q_EXPORT_PLUGIN2(invert, FilterPlugin);
#endif

void invert(QImage &img);

QString
FilterPlugin:: menuItem()
{
    // if you need / in menu name, use % character. Because / is path separator here
    return QString("Filters/Color/Invert%Negative");
}

void
FilterPlugin:: onMenuClick()
{
    invert(data->image);
    emit imageChanged();
}


//********* ---------- Invert Colors or Negate --------- ********** //
void invert(QImage &img)
{
    for (int y=0;y<img.height();y++) {
        QRgb* line = (QRgb*) img.scanLine(y);
        for (int x=0;x<img.width();x++) {
            line[x] = qRgba(255-qRed(line[x]), 255-qGreen(line[x]), 255-qBlue(line[x]), qAlpha(line[x]));
        }
    }
}


