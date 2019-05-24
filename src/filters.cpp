#include "filters.h"
#include <cmath>
// #include <chrono>
// #include <QDebug>

void grayScale(QImage &img)
{
    for (int y=0;y<img.height();y++) {
        QRgb* line = ((QRgb*)img.scanLine(y));
        for (int x=0;x<img.width();x++) {
            int val = qGray(line[x]);
            line[x] = qRgba(val,val,val, qAlpha(line[x]));
        }
    }
}

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

void globalThresh(QImage &img, int thresh)
{
    for (int y=0;y<img.height();y++) {
        QRgb* line = ((QRgb*)img.scanLine(y));
        for (int x=0;x<img.width();x++) {
            if (qGray(line[x]) > thresh)
                line[x] = qRgb(255,255,255);
            else
                line[x] = qRgb(0,0,0);
        }
    }
}

// Apply Bradley threshold (to get desired output, tune value of T and s)
void adaptiveIntegralThresh(QImage &img)
{
    //auto start = std::chrono::steady_clock::now();
    int w = img.width();
    int h = img.height();
    // Allocate memory for integral image
    int *ptr, **intImg;
    int len = sizeof(int *) * h + sizeof(int) * w * h;
    intImg = (int **)malloc(len);

    ptr = (int*)(intImg + h);
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
    int x1,y1,x2,y2, count, sum;
    float T = 0.15;
    int s = w/32 > 16? w/32: 16;
    int s2 = s/2;
    for (int i=0;i<h;++i)
    {
        y1 = ((i - s2)>0) ? (i - s2) : 0;
        y2 = ((i + s2)<h) ? (i + s2) : h-1;
        QRgb *row = (QRgb*)img.scanLine(i);
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
    //auto end = std::chrono::steady_clock::now();
    //double elapse = std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count();
    //qDebug() << "Time :" << elapse;
}

void boxBlur(QImage &img, int r/*blur radius*/)
{
    // auto start = std::chrono::steady_clock::now();
    int w = img.width();
    int h = img.height();
    QImage tmp = QImage(w,h,QImage::Format_ARGB32);// temporary image
    QRgb *row, *tmp_row;
    int x, y, sum_r,sum_g,sum_b, count;
    // Run blur in horizontal direction
    for (y=0; y<h; ++y)
    {
        row = (QRgb*)img.constScanLine(y);
        tmp_row = (QRgb*)tmp.scanLine(y);
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
    QRgb clr, clr2;
    for (x=0; x<w; ++x)
    {
        sum_r = sum_g = sum_b = 0;
        for (y=0; y<=r; ++y) {
            clr = ((QRgb*)tmp.constScanLine(y))[x];
            sum_r += qRed(clr); sum_g += qGreen(clr); sum_b += qBlue(clr);
        }
        count = r+1;
        ((QRgb*)img.scanLine(0))[x] = qRgb(sum_r/count, sum_g/count, sum_b/count);
        for (y=1; y<=r; y++) {
            clr = ((QRgb*)tmp.constScanLine(y+r))[x];
            sum_r += qRed(clr); sum_g += qGreen(clr); sum_b += qBlue(clr);
            count += 1;
            ((QRgb*)img.scanLine(y))[x] = qRgb(sum_r/count, sum_g/count, sum_b/count);
        }
        for (y=r+1; y<h-r; y++) {
            clr = ((QRgb*)tmp.constScanLine(y+r))[x];
            clr2 = ((QRgb*)tmp.constScanLine(y-r-1))[x];
            sum_r += qRed(clr) - qRed(clr2);
            sum_g += qGreen(clr) - qGreen(clr2);
            sum_b += qBlue(clr) - qBlue(clr2);
            ((QRgb*)img.scanLine(y))[x] = qRgb(sum_r/count, sum_g/count, sum_b/count);
        }
        for (y=h-r; y<h; y++) {
            clr = ((QRgb*)tmp.constScanLine(y-r-1))[x];
            sum_r -= qRed(clr); sum_g -= qGreen(clr); sum_b -= qBlue(clr);
            count -= 1;
            ((QRgb*)img.scanLine(y))[x] = qRgb(sum_r/count, sum_g/count, sum_b/count);
        }
    }
    // auto end = std::chrono::steady_clock::now();
    // double elapse = std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count();
    // qDebug() << "Time :" << elapse;
}

// Using unsharp mask
void sharpen(QImage &img)
{
    QImage mask = img.copy();
    boxBlur(mask, 1);
    int w = img.width();
    int h = img.height();
    for (int y=0; y<h; y++)
    {
        QRgb *row_mask = (QRgb*)mask.constScanLine(y);
        QRgb *row = (QRgb*)img.scanLine(y);
        for (int x=0; x<w; x++)
        {
            int r = qRed(row[x]) + 0.7*(qRed(row[x]) - qRed(row_mask[x]));
            int g = qGreen(row[x]) + 0.7*(qGreen(row[x]) - qGreen(row_mask[x]));
            int b = qBlue(row[x]) + 0.7*(qBlue(row[x]) - qBlue(row_mask[x]));
            r = (r < 0)? 0: (r>255? 255:r);
            g = (g < 0)? 0: (g>255? 255:g);
            b = (b < 0)? 0: (b>255? 255:b);
            row[x] = qRgb(r, g, b);
        }
    }
}

// *******-------- Sigmoidal Contrast ---------**********
// Sigmoidal Contrast Image to enhance low contrast image
#define Sigmoidal(a,b,x) ( tanh((0.5*(a))*((x)-(b))) )

#define ScaledSigmoidal(a,b,x) (                    \
  (Sigmoidal((a),(b),(x))-Sigmoidal((a),(b),0.0)) / \
  (Sigmoidal((a),(b),1.0)-Sigmoidal((a),(b),0.0)) )

// midpoint => range = 0.0 -> 1.0 , default = 0.5
// contrast => range =   1 -> 20,   default = 3
void sigmoidalContrast(QImage &img, float midpoint)
{
    int w = img.width();
    int h = img.height();
    #pragma omp parallel for
    for (int y=0; y<h; y++)
    {
        QRgb *row;
        #pragma omp critical
        {
        row = (QRgb*)img.scanLine(y); // omp critical prevents segfault
        }
        for (int x=0; x<w; x++) {
            int clr = row[x];
            int r = 255*ScaledSigmoidal(3, midpoint, qRed(clr)/255.0);
            int g = 255*ScaledSigmoidal(3, midpoint, qGreen(clr)/255.0);
            int b = 255*ScaledSigmoidal(3, midpoint, qBlue(clr)/255.0);
            row[x] = qRgb(r,g,b);
        }
    }
}
