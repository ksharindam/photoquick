#include "filters.h"

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
        for (x = 0; x < img.width(); x++)
            ++histogram[qGray(img.pixel(x,y))];
    }

    // Calculate sum
    int sum = 0;
    for (int idx = 0; idx < HISTOGRAM_SIZE; ++idx) 
    {
        sum += idx * histogram[idx];
    }

    // Compute threshold
    int threshold = 0;
    int sumB = 0;
    int q1 = 0;
    double max = 0;
    for (int idx = 0; idx < HISTOGRAM_SIZE; ++idx) 
    {
        // q1 = Weighted Background
        q1 += histogram[idx];
        if (q1 == 0)
            continue;

        // q2 = Weighted Forground
        const int q2 = N - q1;
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

void applyThresh(QImage &img, int thresh)
{
    for (int y=0;y<img.height();y++) {
        QRgb* line = ((QRgb*)img.scanLine(y));
        for (int x=0;x<img.width();x++) {
            int val = qRed(line[x]);
            if (val>thresh)
                line[x] = qRgba(255,255,255, 255);
            else
                line[x] = qRgba(0,0,0, 255);
        }
    }
}

void adaptiveIntegralThresh(QImage &img)
{
    int w = img.width();
    int h = img.height();
    // Allocate memory for imtegral image
    int *ptr, **intImg;
    int len = sizeof(int *) * h + sizeof(int) * w * h;
    intImg = (int **)malloc(len);

    ptr = (int*)(intImg + h);
    for(int i = 0; i < h; i++)
        intImg[i] = (ptr + w * i);

    // Calculate integral image
    for (int y=0;y<h;++y)
    {
        int sum=0;
        for (int x=0;x<w;++x)
        {
            sum += qRed(img.pixel(x,y));
            if (y==0)
                intImg[y][x] = qRed(img.pixel(x,y));
            else
                intImg[y][x] = intImg[y-1][x] + sum;
        }
    }
    // Apply Bradley threshold
    int x1,y1,x2,y2, count, sum;
    float T = 0.15;
    int s = w/16;
    int s2 = s/2;
    for (int i=0;i<h;++i)
    {
        y1 = ((i - s2)>0) ? (i - s2) : 0;
        y2 = ((i + s2)<h) ? (i + s2) : h-1;
        for (int j=0;j<w;++j)
        {
            x1 = ((j - s2)>0) ? (j - s2) : 0;
            x2 = ((j + s2)<w) ? (j + s2) : w-1;

            count = (x2 - x1)*(y2 - y1);
            sum = intImg[y2][x2] - intImg[y2][x1] - intImg[y1][x2] + intImg[y1][x1];

            if ((qRed(img.pixel(j,i)) * count) < (int)(sum*(1.0 - T)))
                img.setPixel(j,i,qRgb(0,0,0));
            else
                img.setPixel(j,i,qRgb(255,255,255));
        }
    }
    free(intImg);
}
