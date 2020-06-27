#include "kuwahara.h"
#include <cmath>
#include <QInputDialog>

Q_EXPORT_PLUGIN2(kuwahara, FilterPlugin);

void kuwaharaFilter(QImage &img, int radius);
void pencilSketch(QImage &img);

QStringList
FilterPlugin:: menuItems()
{
    return QStringList({"Filter/Artistic/Kuwahara Filter", "Filter/Artistic/Pencil Sketch"});
}

void
FilterPlugin:: handleAction(QAction *action, int)
{
    if (action->text() == QString("Kuwahara Filter"))
        connect(action, SIGNAL(triggered()), this, SLOT(filterKuwahara()));
    else if (action->text() == QString("Pencil Sketch"))
        connect(action, SIGNAL(triggered()), this, SLOT(filterPencilSketch()));
    // here you can also set key shortcut to QAction
}

void
FilterPlugin:: filterKuwahara()
{
    bool ok;
    int radius = QInputDialog::getInt(canvas, "Blur Radius", "Enter Blur Radius :",
                                        3/*val*/, 1/*min*/, 50/*max*/, 1/*step*/, &ok);
    if (not ok) return;
    kuwaharaFilter(canvas->image, radius);
    emit imageChanged();
}

void
FilterPlugin:: filterPencilSketch()
{
    pencilSketch(canvas->image);
    emit imageChanged();
}

// takes 4 pixels and a floating point coordinate, returns bilinear interpolated pixel
QRgb interpolateBilinear(float x, float y, QRgb p00, QRgb p01, QRgb p10, QRgb p11)
{
    float offset_x = floorf(x);
    float offset_y = floorf(y);
    float delta_x = (x-offset_x);// w
    float delta_y = (y-offset_y);// h
    float epsilon_x = 1.0-delta_x;// 1-w
    float epsilon_y = 1.0-delta_y;// 1-h
    // Y = A(1-w)(1-h) + B(w)(1-h) + C(h)(1-w) + D(w)(h)
    //   = (1-h){(1-w)A + wB} + h{(1-w)C + wD}
    int r = (epsilon_y*(epsilon_x*qRed(p00)+delta_x*qRed(p01)) +
                delta_y*(epsilon_x*qRed(p10)+delta_x*qRed(p11)));
    int g = (epsilon_y*(epsilon_x*qGreen(p00)+delta_x*qGreen(p01)) +
                delta_y*(epsilon_x*qGreen(p10)+delta_x*qGreen(p11)));
    int b = (epsilon_y*(epsilon_x*qBlue(p00)+delta_x*qBlue(p01)) +
                delta_y*(epsilon_x*qBlue(p10)+delta_x*qBlue(p11)));
    return qRgb(r,g,b);
}

// Calculate Luminance (Y) from an sRGB value
inline float getPixelLuma(QRgb clr)
{
  return (0.212656f*qRed(clr) + 0.715158f*qGreen(clr) + 0.072186f*qBlue(clr));
}

inline double getPixelLuma(double red, double green, double blue)
{
  return (0.212656*red + 0.715158*green + 0.072186*blue);
}


typedef struct {
    int x;
    int y;
    int width;
    int height;
}RectInfo;

// Expand each size of Image by certain amount of border
QImage expandBorder(QImage img, int width)
{
    int w = img.width();
    int h = img.height();
    QImage dst = QImage(w+2*width, h+2*width, img.format());
    // copy all image pixels at the center
    QRgb *row, *dstRow;
    for (int y=0; y<h; y++) {
        row = (QRgb*)img.constScanLine(y);
        dstRow = (QRgb*)dst.scanLine(y+width);
        dstRow += width;
        memcpy(dstRow, row, w*4);
    }
    // duplicate first row
    row = (QRgb*)img.constScanLine(0);
    for (int i=0; i<width; i++) {
        dstRow = (QRgb*)dst.scanLine(i);
        dstRow += width;
        memcpy(dstRow, row, w*4);
    }
    // duplicate last row
    row = (QRgb*)img.constScanLine(h-1);
    for (int i=0; i<width; i++) {
        dstRow = (QRgb*)dst.scanLine(width+h+i);
        dstRow += width;
        memcpy(dstRow, row, w*4);
    }
    // duplicate left and right sides
    QRgb left_clr, right_clr;
    for (int y=0; y<h; y++) {
        dstRow = (QRgb*)dst.scanLine(y+width);
        left_clr = ((QRgb*)img.constScanLine(y))[0];
        right_clr = ((QRgb*)img.constScanLine(y))[w-1];
        for (int x=0; x<width; x++) {
            dstRow[x] = left_clr;
            dstRow[width+w+x] = right_clr;
        }
    }
    // duplicate corner pixels
    row = (QRgb*)img.constScanLine(0);
    for (int y=0; y<width; y++) {
        dstRow = (QRgb*)dst.scanLine(y);
        for (int x=0; x<width; x++) {
            dstRow[x] = row[0];
            dstRow[width+w+x] = row[w-1];
        }
    }
    row = (QRgb*)img.constScanLine(h-1);
    for (int y=0; y<width; y++) {
        dstRow = (QRgb*)dst.scanLine(width+h+y);
        for (int x=0; x<width; x++) {
            dstRow[x] = row[0];
            dstRow[width+w+x] = row[w-1];
        }
    }
    return dst;
}

// convolve a 1D kernel first left to right and then top to bottom
void convolve1D(QImage &img, float kernel[], int width/*of kernel*/)
{
    /* Build normalized kernel */
    float normal_kernel[width]; // = {}; // Throws error in C99 compiler
    memset(normal_kernel, 0, width * sizeof(float));

    float normalize = 0.0;
    for (int i=0; i < width; i++)
        normalize += kernel[i];
    // if (abs(normalize) == 0) normalize=1.0;
    normalize = 1.0/normalize;
    for (int i=0; i < (width); i++) {
        normal_kernel[i] = normalize * kernel[i];
    }

    int radius = width/2;
    int w = img.width();
    int h = img.height();
    /* Convolve image */
    QImage src_img = expandBorder(img, radius);
    int src_w = src_img.width();

    QRgb *data_src = (QRgb*)src_img.constScanLine(0);
    QRgb *data_dst = (QRgb*)img.scanLine(0);

    #pragma omp parallel for
    for (int y=0; y < h; y++)
    {
        QRgb *row_dst = data_dst + (y*w);
        QRgb *row_src = data_src + (src_w*(y+radius));

        for (int x=0; x < w; x++)
        {
            float r=0, g=0, b=0;
            for (int i=0; i < width; i++)
            {
                QRgb clr = row_src[x+i];
                r += normal_kernel[i] * qRed(clr);
                g += normal_kernel[i] * qGreen(clr);
                b += normal_kernel[i] * qBlue(clr);
            }
            row_dst[x] = qRgba(round(r), round(g), round(b), qAlpha(row_dst[x]));
        }
    }
    // Convolve from top to bottom
    src_img = expandBorder(img, radius);
    data_src = (QRgb*)src_img.constScanLine(0);

    #pragma omp parallel for
    for (int y=0; y < h; y++)
    {
        QRgb *row_dst = data_dst + (y*w);

        for (int x=0; x < w; x++)
        {
            float r=0, g=0, b=0;
            for (int i=0; i < width; i++)
            {
                QRgb *row_src = data_src + (src_w*(y+i));
                QRgb clr = row_src[x+radius];
                r += normal_kernel[i] * qRed(clr);
                g += normal_kernel[i] * qGreen(clr);
                b += normal_kernel[i] * qBlue(clr);
            }
            row_dst[x] = qRgba( round(r), round(g), round(b), qAlpha(row_dst[x]) );
        }
    }
}

//*************---------- Gaussian Blur ---------***************//
// 1D Gaussian kernel -> g(x)   = 1/{sqrt(2.pi)*sigma} * e^{-(x^2)/(2.sigma^2)}
// 2D Gaussian kernel -> g(x,y) = 1/(2.pi.sigma^2) * e^{-(x^2 +y^2)/(2.sigma^2)}

void gaussianBlur(QImage &img, int radius, float sigma/*standard deviation*/)
{
    if (sigma==0)  sigma = radius/2.0 ;
    int kernel_width = 2*radius + 1;
    // build 1D gaussian kernel
    float kernel[kernel_width];

    for (int i=0; i<kernel_width; i++)
    {
        int u = i - radius;
        double alpha = exp(-(u*u)/(2.0*sigma*sigma));
        kernel[i] = alpha/(sqrt(2*M_PI)*sigma);
    }
    convolve1D(img, kernel, kernel_width);
}

void kuwaharaFilter(QImage &img, int radius)
{
    int w = img.width();
    int h = img.height();
    QImage gaussImg = img.copy();
    gaussianBlur(gaussImg, radius, 0);
    int width = radius+1;

    QRgb *srcData = (QRgb*)gaussImg.constScanLine(0);
    QRgb *dstData = (QRgb*)img.scanLine(0);
    #pragma omp parallel for
    for (int y=0; y<h; y++)
    {
        for (int x=0; x<w; x++)
        {
            double min_variance = 1.7e308;//maximum for double
            RectInfo quadrant;
            RectInfo target = {0,0,1,1};
            for (int i=0; i<4; i++)
            {
                quadrant.x = x;
                quadrant.y = y;
                quadrant.width=width;
                quadrant.height=width;

                switch (i)
                {
                  case 0:
                  {
                    quadrant.x = x-(width-1);
                    quadrant.y = y-(width-1);
                    break;
                  }
                  case 1:
                  {
                    quadrant.y = y-(width-1);
                    break;
                  }
                  case 2:
                  {
                    quadrant.x = x-(width-1);
                    break;
                  }
                  default:
                    break;
                } // end of switch
                // manage boundary problem
                if (quadrant.x <0) {
                    quadrant.x=0;
                    quadrant.width = x+1;
                }
                else if (quadrant.x+quadrant.width>w)
                    quadrant.width = w-quadrant.x;
                if (quadrant.y <0) {
                    quadrant.y=0;
                    quadrant.height = y+1;
                }
                else if (quadrant.y+quadrant.height>h)
                    quadrant.height = h-quadrant.y;
                // calculate mean of variance
                double mean_r = 0, mean_g = 0, mean_b = 0;
                QRgb *quadRow = srcData + (w*quadrant.y + quadrant.x);
                // for each pixel in quadrant
                for (int m=0; m<quadrant.height; m++)
                {
                    for (int n=0; n<quadrant.width; n++) {
                      mean_r += (double) qRed(quadRow[n]);
                      mean_g += (double) qGreen(quadRow[n]);
                      mean_b += (double) qBlue(quadRow[n]);
                    }
                    quadRow += w;
                }
                mean_r /= (quadrant.width*quadrant.height);
                mean_g /= (quadrant.width*quadrant.height);
                mean_b /= (quadrant.width*quadrant.height);

                double mean_luma = getPixelLuma(mean_r, mean_g, mean_b);
                double variance=0.0;
                quadRow = srcData + (w*quadrant.y + quadrant.x);
                for (int m=0; m<quadrant.height; m++)
                {
                    for (int n=0; n<quadrant.width; n++) {
                      double luma = getPixelLuma(quadRow[n]);
                      variance += (luma-mean_luma)*(luma-mean_luma);
                    }
                    quadRow += w;
                }
                if (variance < min_variance)
                {
                    min_variance=variance;
                    target=quadrant;
                }
            }   // end quadrant loop
            QRgb clr = (srcData + (w*(target.y+target.height/2)))[(target.x+target.width/2)];
            (dstData + w*y)[x] = clr;
        }   // end column loop
    } // end row loop
}

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

//**********----------- Box Blur -----------*************//
// also called mean blur
void boxFilter(QImage &img, int r/*blur radius*/)
{
    int w = img.width();
    int h = img.height();
    int kernel_w = 2*r + 1;

    QImage src_img = expandBorder(img, r);
    int src_w = src_img.width();
    QRgb *data_src = (QRgb*) src_img.constScanLine(0);
    QRgb *data_dst = (QRgb*) img.scanLine(0);

    #pragma omp parallel for
    for (int y=0; y<h; ++y)
    {
        QRgb *row_dst = data_dst + (y*w);
        QRgb *row_src = data_src + ((y+r)*src_w);

        int sum_r = 0, sum_g = 0, sum_b = 0;
        for (int x=0; x<kernel_w; x++) {
            int clr = row_src[x];
            sum_r += qRed(clr); sum_g += qGreen(clr); sum_b += qBlue(clr);
        }
        row_dst[0] = qRgba(sum_r/kernel_w, sum_g/kernel_w, sum_b/kernel_w, qAlpha(row_dst[0]));

        for (int x=1; x<w; x++) {
            int left = row_src[x-1];
            int right = row_src[x+r+r];
            sum_r += qRed(right) - qRed(left);
            sum_g += qGreen(right) - qGreen(left);
            sum_b += qBlue(right) - qBlue(left);
            row_dst[x] = qRgba(sum_r/kernel_w, sum_g/kernel_w, sum_b/kernel_w, qAlpha(row_dst[x]));
        }
    }
    src_img = expandBorder(img, r);
    data_src = (QRgb*) src_img.constScanLine(0);

    #pragma omp parallel for
    for (int x=0; x<w; ++x)
    {
        int sum_r = 0, sum_g = 0, sum_b = 0;

        for (int y=0; y<kernel_w; y++) {
            int clr = (data_src + (y*src_w))[x+r];
            sum_r += qRed(clr); sum_g += qGreen(clr); sum_b += qBlue(clr);
        }
        (data_dst)[x] = qRgba(sum_r/kernel_w, sum_g/kernel_w, sum_b/kernel_w,
                                qAlpha((data_dst)[x]));//first row

        for (int y=1; y<h; y++) {
            int clr_top = (data_src + ((y-1)*src_w))[x+r];
            int clr_btm = (data_src + ((y+r+r)*src_w))[x+r];
            sum_r += qRed(clr_btm) - qRed(clr_top);
            sum_g += qGreen(clr_btm) - qGreen(clr_top);
            sum_b += qBlue(clr_btm) - qBlue(clr_top);
            (data_dst + (y*w))[x] = qRgba(sum_r/kernel_w, sum_g/kernel_w, sum_b/kernel_w,
                                        qAlpha((data_dst + (y*w))[x]));
        }
    }
}

//********* ---------- Invert Colors or Negate --------- ********** //
void invert(QImage &img)
{
    #pragma omp parallel for
    for (int y=0;y<img.height();y++) {
        QRgb* line;
        #pragma omp critical
        { line = ((QRgb*)img.scanLine(y));}
        for (int x=0;x<img.width();x++) {
            line[x] = qRgba(255-qRed(line[x]), 255-qGreen(line[x]), 255-qBlue(line[x]), qAlpha(line[x]));
        }
    }
}

//********** --------- Gray Scale Image --------- ********** //
void grayScale(QImage &img)
{
    #pragma omp parallel for
    for (int y=0;y<img.height();y++) {
        QRgb* line;
        #pragma omp critical
        { line = ((QRgb*)img.scanLine(y));}
        for (int x=0;x<img.width();x++) {
            int val = qGray(line[x]);
            line[x] = qRgba(val,val,val, qAlpha(line[x]));
        }
    }
}

void pencilSketch(QImage &img)
{
    grayScale(img);
    QImage topImg = img.copy();
    invert(topImg);
    boxFilter(topImg, img.width()/20);

    #pragma omp parallel for
    for (int y=0;y<img.height();y++) {
        QRgb *line, *top_line;
        #pragma omp critical
        { line = ((QRgb*)img.scanLine(y));
          top_line = ((QRgb*)topImg.scanLine(y));}
        for (int x=0;x<img.width();x++) {
            int back = qRed(line[x]);
            int top = qRed(top_line[x]);
            if (back==255 || top==0) continue;
            // blend topImg and img using color dodge blend
            // i.e divide the top layer by inverted bottom layer
            int val = MIN(255, (top<<8)/(255-back));
            line[x] = qRgba(val,val,val, qAlpha(line[x]));
        }
    }
}


