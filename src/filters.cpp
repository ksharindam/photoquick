// this file is part of photoquick program which is GPLv3 licensed
#include "filters.h"
#include "common.h"
#include <cmath>
#include <chrono>
#include <QDebug>

#define PI 3.141592654
// macros for measuring execution time
#define TIME_START auto start = std::chrono::steady_clock::now();
#define TIME_STOP auto end = std::chrono::steady_clock::now();\
    double elapse = std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count();\
    qDebug() << "Execution Time :" << elapse;

// Byte order is ARGB if big endian else BGRA
inline bool isBigEndian()
{
    int i=1; return ! *((char *)&i);
}

// clamp an integer in 0-255 range
#define Clamp(a) ( (a)&(~0xff) ? (uchar)((~a)>>31) : (a) )

/* HSV colorspace utilities
we are using 32 bit unsigned int to hold a hsv color
first 16 bit is for hue, next 8 bit is for saturation and last
8 bit is for value
h = 0-359, s = 0-255, v = 0-255
*/

typedef unsigned int QHsv;

inline int qHue(QHsv hsv) { return ((hsv>>16) & 0xffff); }
// sat and val has same pos like green and blue i.e last 16 bits
#define qSat qGreen
#define qVal qBlue

inline QHsv qHsv(int h, int s, int v)
{
    return ( (h & 0xffffu)<<16 | (s & 0xffu)<<8 | (v & 0xffu) );
}

inline void rgbToHsv(QRgb rgb, int &h, int &s, int &v)
{
    float r = qRed(rgb)/255.0;
    float g = qGreen(rgb)/255.0;
    float b = qBlue(rgb)/255.0;
    float mx = MAX(MAX(r,g),b);
    float mn = MIN(MIN(r,g),b);
    float df = mx - mn;

    if (mx==mn)
        h = 0;
    else if (mx==r)
        h = (int)roundf(60*(g-b)/df+360) % 360;
    else if (mx==g)
        h = (int)roundf(60*(b-r)/df+120) % 360;
    else if (mx==b)
        h = (int)roundf(60*(r-g)/df+240) % 360;

    s = (mx==0)? 0 : roundf(255*df/mx);
    v = 255*mx;
}
// calculate remainder i.e a%b (works in case of float)
#define mod(a,b) ( (a) - ((int)((a)/(b)))*(b) )
void hsvToRgb(QHsv hsv, int &r, int &g, int &b)
{
    float h = qHue(hsv)/60.0;
    float s = qSat(hsv)/255.0;
    float v = qVal(hsv)/255.0;
    float k;
    k = mod(5 + h, 6);
    r = roundf(255*(v - v*s*MAX(MIN(MIN(k, 4-k), 1),0)));
    k = mod(3 + h, 6);
    g = roundf(255*(v - v*s*MAX(MIN(MIN(k, 4-k), 1),0)));
    k = mod(1 + h, 6);
    b = roundf(255*(v - v*s*MAX(MIN(MIN(k, 4-k), 1),0)));
}

// convert rgb image to hsv image
void hsvImg(QImage &img)
{
    int w = img.width();
    int h = img.height();
    #pragma omp parallel for
    for (int y=0; y<h; y++)
    {
        QHsv *row;
        int h=0,s=0,v=0;
        #pragma omp critical
        { row = (QHsv*)img.scanLine(y);}
        for (int x=0; x<w; x++) {
            rgbToHsv(row[x],h,s,v);
            row[x] = qHsv(h,s,v);
        }
    }
}

// Expand each size of Image by certain amount of border
QImage expandBorder(QImage img, int width)
{
    int w = img.width();
    int h = img.height();
    QImage dst = QImage(w+2*width, h+2*width, QImage::Format_ARGB32);
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


//********** --------- Gray Scale Image --------- ********** //
void grayScale(QImage &img)
{
    expandBorder(img, 3);
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

//********* --------- Global Threshold -------- ***********//
#define HISTOGRAM_SIZE 256

int calcOtsuThresh(QImage img)
{
    // Compute number of pixels
    long int N = img.width()*img.height();

    // Create Histogram
    unsigned int histogram[HISTOGRAM_SIZE];
    memset(histogram, 0, (HISTOGRAM_SIZE) * sizeof(unsigned int));
    int x, y;
    for (y = 0; y < img.height(); ++y)
    {
        QRgb *row = (QRgb*)img.constScanLine(y);
        for (x = 0; x < img.width(); x++)
            ++histogram[qGray(row[x])];
    }

    // Calculate sum
    int sum = 0;
    for (int idx = 0; idx < HISTOGRAM_SIZE; ++idx)
        sum += idx * histogram[idx];

    // Compute threshold
    int threshold = 0;
    int sumB = 0;
    int q1 = 0;
    double max = 0;
    for (int idx = 0; idx < HISTOGRAM_SIZE; ++idx)
    {
        q1 += histogram[idx]; // q1 = Weighted Background
        if (q1 == 0)
            continue;

        const int q2 = N - q1; // q2 = Weighted Forground
        if (q2 == 0)
            break;

        sumB += (idx * histogram[idx]);

        const double m1m2 =
            (double)sumB / q1 -			// Mean Background
            (double)(sum - sumB) / q2;	// Mean Forground

        // Note - There is an insidious casting situation going on here.
        // If one were to multiple by q1 or q2 first, an explicit cast would be required!
        const double between = m1m2 * m1m2 * q1 * q2;

        if (between > max)
        {
            threshold = idx;
            max = between;
        }
    }
    return threshold;
}

void threshold(QImage &img, int thresh)
{
    #pragma omp parallel for
    for (int y=0;y<img.height();y++) {
        QRgb* line;
        #pragma omp critical
        { line = ((QRgb*)img.scanLine(y)); }
        for (int x=0;x<img.width();x++) {
            if (qGray(line[x]) > thresh)
                line[x] = qRgb(255,255,255);
            else
                line[x] = qRgb(0,0,0);
        }
    }
}

//*********---------- Adaptive Threshold ---------**********//
// Apply Bradley threshold (to get desired output, tune value of T and s)
void adaptiveThreshold(QImage &img)
{
    int w = img.width();
    int h = img.height();
    // Allocate memory for integral image
    int len = sizeof(int *) * h + sizeof(int) * w * h;
    unsigned **intImg = (unsigned **)malloc(len);

    unsigned *ptr = (unsigned*)(intImg + h);
    for(int i = 0; i < h; i++)
        intImg[i] = (ptr + w * i);

    // Calculate integral image
    for (int y=0;y<h;++y)
    {
        QRgb *row = (QRgb*)img.constScanLine(y);
        int sum=0;
        for (int x=0;x<w;++x)
        {
            sum += qGray(row[x]);
            if (y==0)
                intImg[y][x] = qGray(row[x]);
            else
                intImg[y][x] = intImg[y-1][x] + sum;
        }
    }
    // Apply Bradley threshold
    float T = 0.15;
    int s = w/32 > 16? w/32: 16;
    int s2 = s/2;
    #pragma omp parallel for
    for (int i=0;i<h;++i)
    {
        int x1,y1,x2,y2, count, sum;
        y1 = ((i - s2)>0) ? (i - s2) : 0;
        y2 = ((i + s2)<h) ? (i + s2) : h-1;
        QRgb *row;
        #pragma omp critical
        { row = (QRgb*)img.scanLine(i); }
        for (int j=0;j<w;++j)
        {
            x1 = ((j - s2)>0) ? (j - s2) : 0;
            x2 = ((j + s2)<w) ? (j + s2) : w-1;

            count = (x2 - x1)*(y2 - y1);
            sum = intImg[y2][x2] - intImg[y2][x1] - intImg[y1][x2] + intImg[y1][x1];

            // threshold = mean*(1 - T) , where mean = sum/count, T = around 0.15
            if ((qGray(row[j]) * count) < (int)(sum*(1.0 - T)) )
                row[j] = qRgb(0,0,0);
            else
                row[j] = qRgb(255,255,255);
        }
    }
    free(intImg);
}


//*********---------- Apply Convolution Matrix ---------**********//
// Kernel Width Must Be An Odd Number
// Image must be larger than Kernel Width
void convolve(QImage &img, float kernel[], int width/*of kernel*/)
{
    int radius = width/2;
    int w = img.width();
    int h = img.height();
    QImage tmp = expandBorder(img, radius);
    int tmp_w = tmp.width();

    /* Build normalized kernel */
    float normal_kernel[width][width]; // = {}; // Throws error in C99 compiler
    memset(normal_kernel, 0, (width*width) * sizeof(float));

    float normalize = 0.0;
    for (int i=0; i < (width*width); i++)
        normalize += kernel[i];
    // if (abs(normalize) == 0) normalize=1.0;
    normalize = 1.0/normalize;
    for (int i=0; i < (width); i++) {
        for (int j=0; j< width; j++)
            normal_kernel[i][j] = normalize*kernel[i*width+j];
    }

    /* Convolve image */
    QRgb *data = (QRgb*)img.scanLine(0);
    QRgb *tmpData = (QRgb*)tmp.constScanLine(0);
    #pragma omp parallel for
    for (int y=0; y < h; y++)
    {
        QRgb *row = data+(y*w);

        for (int x=0; x < w; x++)
        {
            float r=0, g=0, b=0;
		    for (int i=0; i < width; i++)
            {
                QRgb *tmpRow = tmpData+(tmp_w*(y+i));
                for (int j=0; j < width; j++)
                {
                    QRgb clr = tmpRow[x+j];
                    r += normal_kernel[i][j] * qRed(clr);
                    g += normal_kernel[i][j] * qGreen(clr);
                    b += normal_kernel[i][j] * qBlue(clr);
                }
            }
            row[x] = qRgb(round(r), round(g), round(b));
        }
    }
}


//*************---------- Gaussian Blur ---------***************//

void gaussianBlur(QImage &img, int radius, float sigma/*standard deviation*/)
{
    if (sigma==0)  sigma = radius/2.0 ;
    int kernel_width = 2*radius + 1;
    float kernel[kernel_width*kernel_width];
    int i=0;
    for (int v=-radius; v <= radius; v++)
    {
        for (int u=-radius; u <= radius; u++)
        {
            double alpha = exp(-((double) u*u+v*v)/(2.0*sigma*sigma));
            kernel[i]=alpha/(2.0*PI*sigma*sigma);
            i++;
        }
    }
    convolve(img, kernel, kernel_width);
}


//**********----------- Box Blur -----------*************//
// also called mean blur
void boxFilter(QImage &img, int r/*blur radius*/)
{
    int w = img.width();
    int h = img.height();
    QImage tmp = QImage(w,h,QImage::Format_ARGB32);// temporary image
    // Run blur in horizontal direction
    #pragma omp parallel for
    for (int y=0; y<h; ++y)
    {
        int x, sum_r,sum_g,sum_b, count;
        QRgb *row, *tmp_row;
        #pragma omp critical
        { row = (QRgb*)img.constScanLine(y);
          tmp_row = (QRgb*)tmp.scanLine(y); }
        sum_r = sum_g = sum_b = 0;
        for (x=0; x<=r; x++) {
            sum_r += qRed(row[x]); sum_g += qGreen(row[x]); sum_b += qBlue(row[x]);
        }
        count = r+1;
        tmp_row[0] = qRgb(sum_r/count, sum_g/count, sum_b/count);
        for (x=1; x<=r; x++) {
            sum_r += qRed(row[x+r]); sum_g += qGreen(row[x+r]); sum_b += qBlue(row[x+r]);
            count += 1;
            tmp_row[x] = qRgb(sum_r/count, sum_g/count, sum_b/count);
        }
        for (x=r+1; x<w-r; x++) {
            sum_r += qRed(row[x+r]) - qRed(row[x-r-1]);
            sum_g += qGreen(row[x+r]) - qGreen(row[x-r-1]);
            sum_b += qBlue(row[x+r]) - qBlue(row[x-r-1]);
            tmp_row[x] = qRgb(sum_r/count, sum_g/count, sum_b/count);
        }
        for (x=w-r; x<w; x++) {
            sum_r -= qRed(row[x-r-1]); sum_g -= qGreen(row[x-r-1]); sum_b -= qBlue(row[x-r-1]);
            count -= 1;
            tmp_row[x] = qRgb(sum_r/count, sum_g/count, sum_b/count);
        }
    }
    // Run blur in vertical direction
    QRgb *data = (QRgb*)img.scanLine(0);
    QRgb* tmpData = (QRgb*)tmp.constScanLine(0);
    #pragma omp parallel for
    for (int x=0; x<w; ++x)
    {
        QRgb clr, clr2;
        int y, sum_r,sum_g,sum_b, count;
        sum_r = sum_g = sum_b = 0;
        for (y=0; y<=r; ++y) {
            clr = (tmpData + (w*y))[x];
            sum_r += qRed(clr); sum_g += qGreen(clr); sum_b += qBlue(clr);
        }
        count = r+1;
        data[x] = qRgb(sum_r/count, sum_g/count, sum_b/count); // first row
        for (y=1; y<=r; y++) {
            clr = (tmpData + (w*(y+r)))[x];
            sum_r += qRed(clr); sum_g += qGreen(clr); sum_b += qBlue(clr);
            count += 1;
            (data + (w*y))[x] = qRgb(sum_r/count, sum_g/count, sum_b/count);
        }
        for (y=r+1; y<h-r; y++) {
            clr = (tmpData + (w*(y+r)))[x];
            clr2 = (tmpData + (w*(y-r-1)))[x];
            sum_r += qRed(clr) - qRed(clr2);
            sum_g += qGreen(clr) - qGreen(clr2);
            sum_b += qBlue(clr) - qBlue(clr2);
            (data + (w*y))[x] = qRgb(sum_r/count, sum_g/count, sum_b/count);
        }
        for (y=h-r; y<h; y++) {
            clr = (tmpData + (w*(y-r-1)))[x];
            sum_r -= qRed(clr); sum_g -= qGreen(clr); sum_b -= qBlue(clr);
            count -= 1;
            (data + (w*y))[x] = qRgb(sum_r/count, sum_g/count, sum_b/count);
        }
    }
}


//*************------------ Sharpen ------------****************
// Using unsharp masking
// output_image = input_image + (input_image - blur_image)

void sharpen(QImage &img)
{
    QImage mask = img.copy();
    boxFilter(mask, 1);
    int w = img.width();
    int h = img.height();
    #pragma omp parallel for
    for (int y=0; y<h; y++)
    {
        QRgb *row, *row_mask;
        #pragma omp critical
        { row_mask = (QRgb*)mask.constScanLine(y);
          row = (QRgb*)img.scanLine(y); }
        for (int x=0; x<w; x++)
        {
            int r_diff = (qRed(row[x]) - qRed(row_mask[x]));
            int g_diff = (qGreen(row[x]) - qGreen(row_mask[x]));
            int b_diff = (qBlue(row[x]) - qBlue(row_mask[x]));
            // threshold = 5, factor = 1.0
            int r = r_diff > 5? qRed(row[x])   + 1.0*r_diff : qRed(row[x]);
            int g = g_diff > 5? qGreen(row[x]) + 1.0*g_diff : qGreen(row[x]);
            int b = b_diff > 5? qBlue(row[x])  + 1.0*b_diff : qBlue(row[x]);
            row[x] = qRgb(Clamp(r), Clamp(g), Clamp(b));
        }
    }
}

// *******-------- Sigmoidal Contrast ---------**********
// Sigmoidal Contrast Image to enhance low contrast image

#define Sigmoidal(a,b,x) ( tanh((0.5*(a))*((x)-(b))) )

#define ScaledSigmoidal(a,b,x,min,max) (                    \
  (Sigmoidal((a),(b),(x))-Sigmoidal((a),(b),(min))) / \
  (Sigmoidal((a),(b),(max))-Sigmoidal((a),(b),(min))) )

// midpoint => range = 0.0 -> 1.0 , default = 0.5
// contrast => range =   1 -> 20,   default = 3
void sigmoidalContrast(QImage &img, float midpoint)
{
    int w = img.width();
    int h = img.height();
    uchar histogram[256];
    for (int i=0; i<256; i++) {
        histogram[i] = 255*ScaledSigmoidal(3, midpoint, i/255.0, 0.0,1.0);
    }
    #pragma omp parallel for
    for (int y=0; y<h; y++)
    {
        QRgb *row;
        #pragma omp critical
        { row = (QRgb*)img.scanLine(y); }
        for (int x=0; x<w; x++) {
            int clr = row[x];
            int r = histogram[qRed(clr)];
            int g = histogram[qGreen(clr)];
            int b = histogram[qBlue(clr)];
            row[x] = qRgb(r,g,b);
        }
    }
}

/*********** ---------- Stretch Contrast ------------- ***************/
// perc = percentile to calculate (range = 0-100)
// N = total number of samples (i.e number of pixels)
int percentile(unsigned int histogram[], float perc, int N)
{
    int A=0;
    for (unsigned int index = N*perc/100; index > histogram[A]; A++) {
        index -= histogram[A];
    }
    return A;
}

void stretchContrast(QImage &img)
{
    float midpoint = 0.3;
    int w = img.width();
    int h = img.height();
    hsvImg(img);
    // Calculate percentile
    unsigned int histogram[256] = {};
    for (int y=0; y<h; y++)
    {
        QRgb *row;
        row = (QRgb*)img.constScanLine(y);
        for (int x=0; x<w; x++) {
            ++histogram[qVal(row[x])];
        }
    }
    int min = percentile(histogram, 0.5, w*h);
    int max = percentile(histogram, 99.5, w*h);
    for (int i=0; i<256; i++) {
        int val = 255*ScaledSigmoidal(3, midpoint, i/255.0, min/255.0, max/255.0);
        histogram[i] = Clamp(val);
    }
    #pragma omp parallel for
    for (int y=0; y<h; y++)
    {
        int r=0,g=0,b=0;
        QRgb *row;
        #pragma omp critical
        { row = (QRgb*)img.scanLine(y); }
        for (int x=0; x<w; x++) {
            int clr = row[x];
            int hsv = qHsv(qHue(clr), qSat(clr), histogram[qVal(clr)]);
            hsvToRgb(hsv, r,g,b);
            row[x] = qRgb(r,g,b);
        }
    }
}

// ************* ------------ Auto White Balance -------------************
// adopted from a stackoverflow code

void autoWhiteBalance(QImage &img)
{
    int w = img.width();
    int h = img.height();
    // Calculate percentile
    unsigned int histogram_r[256] = {};
    unsigned int histogram_g[256] = {};
    unsigned int histogram_b[256] = {};
    for (int y=0; y<h; y++)
    {
        QRgb *row;
        row = (QRgb*)img.constScanLine(y);
        for (int x=0; x<w; x++) {
            ++histogram_r[qRed(row[x])];
            ++histogram_g[qGreen(row[x])];
            ++histogram_b[qBlue(row[x])];
        }
    }
    int min_r = percentile(histogram_r, 0.5, w*h);
    int min_g = percentile(histogram_g, 0.5, w*h);
    int min_b = percentile(histogram_b, 0.5, w*h);
    int max_r = percentile(histogram_r, 99.5, w*h);
    int max_g = percentile(histogram_g, 99.5, w*h);
    int max_b = percentile(histogram_b, 99.5, w*h);
    #pragma omp parallel for
    for (int y=0; y<h; y++)
    {
        QRgb *row;
        #pragma omp critical
        { row = (QRgb*)img.scanLine(y);}
        for (int x=0; x<w; x++) {
            int r = 255.0*(qRed(row[x]) - min_r)/(max_r-min_r);// stretch contrast
            int g = 255.0*(qGreen(row[x]) - min_g)/(max_g-min_g);
            int b = 255.0*(qBlue(row[x]) - min_b)/(max_b-min_b);
            row[x] = qRgb(Clamp(r), Clamp(g), Clamp(b));
        }
    }
}



// ********* ---------- Despecle ---------- ***********
// Crimmins speckle removal

void Hull(int x_offset, int y_offset, int w, int h, int polarity, uchar *f, uchar *g)
{
    uchar *p, *q, *r, *s, v;
    p = f + w+2;
    q = g + w+2;
    r = p + (y_offset*(w+2)+x_offset);
    for (int y=0; y < h; y++)
    {
        int i=(2*y+1)+y*w;
        if (polarity > 0)
            for (int x=0; x < w; x++)
            {
                v = p[i];
                if (r[i] >= (v+2)) //increase color by 2 unit
                    v+=1;
                q[i] = v;
                i++;
            }
        else
            for (int x=0; x < w; x++)
            {
                v= p[i];
                if ( r[i] <= (v-2))
                    v-=1;
                q[i]= v;
                i++;
            }
    }

    p = f + (w+2);
    q = g + (w+2);
    r = q + (y_offset*(w+2)+x_offset);
    s = q - (y_offset*(w+2)+x_offset);
    for (int y=0; y < h; y++)
    {
        int i=(2*y+1)+y*w;
        if (polarity > 0)
            for (int x=0; x < w; x++)
            {
                v=q[i];
                if ((s[i] >= (v+2)) &&
                    ( r[i] > v))
                    v+=1;
                p[i]=v;
                i++;
            }
        else
            for (int x=0; x < w; x++)
            {
                v=q[i];
                if ((s[i] <= (v-2)) &&
                    ( r[i] < v))
                    v-=1;
                p[i] = v;
                i++;
            }
    }
}

void despeckle(QImage &img)
{
    int w = img.width();
    int h = img.height();
    int X[4] = {0, 1, 1,-1}, Y[4] = {1, 0, 1, 1};
    int length = (w+2)*(h+2); // temp buffers contain 1 pixel border
    #pragma omp parallel for
    for (int i=0; i < 4; i++) // 4 channels ARGB32 image
    {
        if (i==0 and isBigEndian()) continue;  // skip Alpha for ARGB order
        if (i==3 and not isBigEndian()) continue;  // BGRA order
        // allocate memory for pixels array
        uchar *pixels = (uchar*)calloc(1,length);
        uchar *buffer = (uchar*)calloc(1,length);
        // draw image inside pixels array
        int j = w+2;    // leave first row
        for (int y=0; y < h; y++)
        {
            uchar *row;
            #pragma omp critical
            { row = (uchar*)img.constScanLine(y); }

            j++; //leave first column
            for (int x=0; x < w; x++)
            {
                pixels[j++] = row[x*4+i]; // clone image
            }
            j++; // leave last column
        }
        // reduce speckle noise
        for (int k=0; k < 4; k++)
        {
            Hull( X[k], Y[k], w,h, 1,pixels,buffer);
            Hull(-X[k],-Y[k], w,h, 1,pixels,buffer);
            Hull(-X[k],-Y[k], w,h,-1,pixels,buffer);
            Hull( X[k], Y[k], w,h,-1,pixels,buffer);
        }
        // draw pixels array over original image
        j=w+2;
        for (int y=0; y < h; y++)
        {
            uchar *row;
            #pragma omp critical
            { row = (uchar*)img.scanLine(y); }
            j++;
            for (int x=0; x < w; x++)
            {
                row[x*4+i] = pixels[j++];
            }
            j++;
        }
        free(buffer);
        free(pixels);
    }
}


// ******** ---------- Reduce Salt & Pepper noise ---------**********//
// Edge preserving noise reduction filter.
// Algorithm implemented from graphicsmagick source which is based on the
// paper "Skip Lists: A probabilistic Alternative to Balanced Trees"
// by William Pugh (1990)
typedef struct
{
  unsigned int
    next[9],
    count,
    signature;
} MedianListNode;

typedef struct
{
  MedianListNode *nodes;
  int level;
} MedianSkipList;

typedef struct
{
  MedianSkipList list;

  unsigned int
    center,
    seed,
    signature;
} MedianPixelList;


void AddNodeMedianList(MedianPixelList *pixel_list, unsigned int color)
{
    MedianSkipList list = pixel_list->list;
    int level;
    unsigned int search, update[9];

    list.nodes[color].signature=pixel_list->signature;
    list.nodes[color].count=1;
    //  Determine where it belongs in the list.
    //    This loop consumes most of the time.
    search=65536UL;
    for (level=list.level; level >= 0; level--)
    {
        while (list.nodes[search].next[level] < color)
            search=list.nodes[search].next[level];
        update[level]=search;
    }
    // Generate a pseudo-random level for this node.
    for (level=0; ; level++)
    {
        pixel_list->seed=(pixel_list->seed*42893621U)+1U;
        if ((pixel_list->seed & 0x300) != 0x300)
            break;
    }
    if (level > 8)
        level=8;
    if (level > (list.level+2))
        level=list.level+2;
    // If we're raising the list's level, link back to the root node.
    while (level > list.level)
    {
        list.level++;
        update[list.level]=65536U;
    }
    //Link the node into the skip-list.
    do
    {
        list.nodes[color].next[level]=list.nodes[update[level]].next[level];
        list.nodes[update[level]].next[level]=color;
    }
    while (level-- > 0);
}

inline
void InsertMedianList(MedianPixelList *pixel_list, uchar pixel)
{
    unsigned int index = (unsigned short)(pixel*257U);
    if (pixel_list->list.nodes[index].signature == pixel_list->signature)
        pixel_list->list.nodes[index].count++;
    else
        AddNodeMedianList(pixel_list, index);
}

void ResetMedianList(MedianPixelList *pixel_list)
{
    MedianListNode *root;
    MedianSkipList list;

    list=pixel_list->list;
    root=list.nodes+65536UL;
    list.level=0;
    for (int level=0; level < 9; level++)
        root->next[level]=65536UL;

    pixel_list->seed = pixel_list->signature++;
}

void DestroyMedianList(void *pixel_list)
{
    MedianPixelList *skiplist = (MedianPixelList *) pixel_list;

    if (skiplist != (void *) NULL)
        free(skiplist->list.nodes);
    free(skiplist);
}

MedianPixelList* AllocateMedianList(const long width)
{
    MedianPixelList *skiplist;

    skiplist = (MedianPixelList *) calloc(1,sizeof(MedianPixelList));//TODO:align to 64
    if (skiplist == (MedianPixelList *) NULL) return skiplist;

    unsigned int node_list_size = 65537U*sizeof(MedianListNode);

    skiplist->center=width*width/2;
    skiplist->signature = 0xabacadabUL; //MagickSignature;
    skiplist->list.nodes = (MedianListNode*) calloc(1,node_list_size);
    if (skiplist->list.nodes == (MedianListNode *) NULL)
    {
        DestroyMedianList(skiplist);
        skiplist=(MedianPixelList *) NULL;
    }
    return skiplist;
}

QRgb GetMedian(MedianPixelList *pixel_list)
{
    MedianSkipList list=pixel_list->list;

    unsigned long center,color,count;

    // Finds the median value
    center=pixel_list->center;
    color=65536L;
    count=0;
    do
    {
        color=list.nodes[color].next[0];
        count+=list.nodes[color].count;
    }
    while (count <= center);
    return color/257U;
}


void medianFilter(QImage &img, int radius)
{
    int w = img.width();
    int h = img.height();
    QImage tmp = expandBorder(img, radius);
    int tmp_w = tmp.width();

    uchar *data = (uchar*)img.scanLine(0);
    uchar *tmpData = (uchar*)tmp.constScanLine(0);

    #pragma omp parallel for
    for (int channel=0; channel<4; channel++) {
        MedianPixelList *skiplist = AllocateMedianList(2*radius+1);

        for (int y=0; y < h; y++)
        {
            uchar *row = data + (y*w*4);
            for (int x=0; x < w; x++)
            {
                ResetMedianList(skiplist);
                for (int i=y-radius; i <= y+radius; i++)
                {
                    uchar *tmpRow = tmpData + ((i+radius)*tmp_w*4);
                    for (int j=x-radius; j<=x+radius; j++)
                        InsertMedianList(skiplist, tmpRow[4*(j+radius)+channel]);
                }
                row[4*x+channel] = GetMedian(skiplist);
            }
        }
        DestroyMedianList(skiplist);
    }
}

//*********** ------------ Kuwahara Filter ------------ ************* //
#if (0)
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

void kuwaharaFilter(QImage &img, int radius)
{
    int w = img.width();
    int h = img.height();
    QImage gaussImg = img.copy();
    gaussianBlur(gaussImg, radius);
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
#endif


// ************ ------------ Enhance Color ----------- ************* //
#if (0)
void enhanceColor(QImage &img)
{
    int w = img.width();
    int h = img.height();
    hsvImg(img);
    // Calculate percentile
    unsigned int histogram[256] = {};
    for (int y=0; y<h; y++)
    {
        QHsv *row;
        row = (QHsv*)img.constScanLine(y);
        for (int x=0; x<w; x++) {
            ++histogram[qSat(row[x])];
        }
    }
    float min = percentile(histogram, 0.2, w*h);
    float max = percentile(histogram, 99.5, w*h);
    for (int i=0; i<256; i++) {
        int sat = 255*(i-min)/(max-min); // try replacing 255 with 255.0
        histogram[i] = Clamp(sat);
    }
    #pragma omp parallel for
    for (int y=0; y<h; y++)
    {
        int r=0,g=0,b=0;
        QRgb *row;
        #pragma omp critical
        { row = (QRgb*)img.scanLine(y); }
        for (int x=0; x<w; x++) {
            int clr = row[x];
            int hsv = qHsv(qHue(clr), histogram[qSat(clr)], qVal(clr));
            hsvToRgb(hsv, r,g,b);
            row[x] = qRgb(r,g,b);
        }
    }
}
#endif

// ************* ------------ Skin Detection -------------************ /
#if (0)
inline bool isSkin(int r, int g, int b){
    int h=0,s=0,v=0;
    rgbToHsv(qRgb(r,g,b), h,s,v);
    return (r>g) and (r>b) and (r>60) and (g>40) and (b>20) and (abs(r-g)>15) and
    ((h>350) or (h<35)) and (s<170) and (v < 310-s);
}
#endif
