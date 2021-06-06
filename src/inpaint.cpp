/* This file is a part of photoquick program, which is GPLv3 licensed */

#include "inpaint.h"
#include "common.h"
#include <cmath>
#include <chrono>
#define TIME_START auto start = std::chrono::steady_clock::now();
#define TIME_STOP auto end = std::chrono::steady_clock::now();\
    double elapse = std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count();\
    qDebug() << "Execution Time :" << elapse;

//Explanation -> https://github.com/YuanTingHsieh/Image_Completion


// the maximum value returned by distanceMaskedImage()
#define DSCALE 65535

static double similarity[DSCALE+1];
static int initSim = 0;

/*void initSimilarity()
{
    int i, length;
    double t_halfmax=0.04, t, coef;
    length = (DSCALE+1);
    if (!initSim){
        similarity = (double *) calloc(length, sizeof(double));
        coef = -log(0.5)/pow(t_halfmax,2);
        for (i=0;i<length;i++) {
            t = (double)i/length;
            similarity[i] = exp(-(t*t)*coef);
        }
    }
    initSim = 1;
}*/
Inpaint:: Inpaint()
{
    // initialize similarity if not initialized before
    if (!initSim) {
        double base[11] = {1.0, 0.99, 0.96, 0.83, 0.38, 0.11, 0.02, 0.005, 0.0006, 0.0001, 0};
        int length = (DSCALE+1);
        for (int i=0 ; i<length ; ++i) {
            double t = (double)i/length;
            int j = (int)(100*t);
            int k = j+1;
            double vj = (j<11)?base[j]:0;
            double vk = (k<11)?base[k]:0;
            similarity[i] = vj + (100*t-j)*(vk-vj);
        }
        initSim = 1;
        srand((unsigned)time(0));
    }
}


QImage
Inpaint:: inpaint(QImage input, QImage mask_img, int radius)
{
    // patch radius
    this->radius = radius;

    // working copies
    MaskedImage* source = new MaskedImage(input);
    source->copyMaskFrom(mask_img);
    MaskedImage* target = NULL;

    debug("Build pyramid of images...\n");

    // build pyramid of downscaled images
    pyramid.append(source);
    while (source->width>radius && source->height>radius) {
        source = source->downsample();
        pyramid.append(source);
    }
    int maxlevel = pyramid.size();

    for (int level=maxlevel-1 ; level>0 ; level--) {

        debug( "*** Processing -  Zoom 1:%d ***" , 1<<level );
        // create Nearest-Neighbor Fields (direct and reverse)
        source = this->pyramid.at(level);

        debug("initialize NNF...\n");

        if (level==maxlevel-1) {
            // at first, we use the same image for target and source
            // and use random data as initial guess
            target = source->copy();

            // we consider that the target contains no masked pixels in the firt time
            for (int y = 0 ; y < target->height ; y++ )
                for (int x = 0 ; x < target->width ; x++ )
                    target->setMask(x, y, 0);

            nnf_SourceToTarget = new NNF(source, target, radius);
            nnf_SourceToTarget->randomize();

            nnf_TargetToSource = new NNF(target, source, radius);
            nnf_TargetToSource->randomize();
        }
        else {
            // then, we use the rebuilt (upscaled) target
            // and re-use the previous NNF as initial guess
            NNF* new_nnf = new NNF(source, target, radius);
            new_nnf->initializeNNF(nnf_SourceToTarget);

            NNF* new_nnf_rev = new NNF(target, source, radius);
            new_nnf_rev->initializeNNF(nnf_TargetToSource);

            delete nnf_TargetToSource->input; // delete previous target
            delete nnf_TargetToSource;
            delete nnf_SourceToTarget;

            nnf_SourceToTarget = new_nnf;
            nnf_TargetToSource = new_nnf_rev;
        }
        target = this->ExpectationMaximization(level);
    }
    QImage output = target->image;

    delete target;
    delete nnf_TargetToSource->input;
    delete nnf_TargetToSource;
    delete nnf_SourceToTarget;
    while (!pyramid.isEmpty())
        delete pyramid.takeLast();
    return output;
}

double** allocVote(int w, int h)
{
    w *= 4;
    int len = h * sizeof(double*) + w*h * sizeof(double);
    double **arr = (double**) calloc(1,len);
    if (arr==NULL){
        printf("could not allocate enough memory for vote");
        exit(1);
    }
    double *ptr = (double*)(arr + h);
    for (int i = 0; i < h; i++)
        arr[i] = (ptr + w*i);
    return arr;
}
// EM-Like algorithm (see "PatchMatch" - page 6)
// Returns a double sized target image
MaskedImage*
Inpaint:: ExpectationMaximization(int level)
{
    int emloop, x, y, H, W;
    double **vote;

    int iterEM = 1+2*level;
    int iterNNF = MIN(7,1+level);

    int upscaled;
    MaskedImage* newsource;
    MaskedImage* source = nnf_SourceToTarget->input;
    MaskedImage* newtarget = nnf_SourceToTarget->output;

    //printf("EM loop (em=%d,nnf=%d) : ", iterEM, iterNNF);

    // EM Loop
    for (emloop=1;emloop<=iterEM;emloop++) {
        //printf(" %d", 1+iterEM-emloop);
        // set the new target as current target
        if (emloop!=1) {
            delete nnf_TargetToSource->input; // delete except the last one
            nnf_SourceToTarget->output = newtarget;
            nnf_TargetToSource->input = newtarget;
        }
        // -- add constraint to the NNF
        H = source->height;
        W = source->width;
        // we force the link between unmasked patch in source/target
        for ( y=0 ; y<H ; ++y)
            for ( x=0 ; x<W; ++x)
                if (!source->containsMasked(x, y, this->radius)) {
                    this->nnf_SourceToTarget->field[y][x][0] = x;
                    this->nnf_SourceToTarget->field[y][x][1] = y;
                    this->nnf_SourceToTarget->field[y][x][2] = 0;
                }

        H = newtarget->height;
        W = newtarget->width;
        for ( y=0 ; y<H ; ++y)
            for ( x=0 ; x<W ; ++x)
                if (!source->containsMasked(x, y, this->radius)) {
                    this->nnf_TargetToSource->field[y][x][0] = x;
                    this->nnf_TargetToSource->field[y][x][1] = y;
                    this->nnf_TargetToSource->field[y][x][2] = 0;
                }
        // -- minimize the NNF
        this->nnf_SourceToTarget->minimizeNNF(iterNNF);
        this->nnf_TargetToSource->minimizeNNF(iterNNF);

        // -- Now we rebuild the target using best patches from source
        upscaled = 0;

        // Instead of upsizing the final target, we build the last target from the next level source image
        // So the final target is less blurry (see "Space-Time Video Completion" - page 5)
        if (level>=1 && (emloop==iterEM)) {
            newsource = pyramid.at(level-1);
            newtarget = newtarget->upscale(newsource->width,newsource->height);
            upscaled = 1;
        } else {
            newsource = pyramid.at(level);
            newtarget = newtarget->copy();
        }
        // --- EXPECTATION STEP ---

        // votes for best patch from NNF Source->Target (completeness) and Target->Source (coherence)
        vote = allocVote(newtarget->width, newtarget->height);

        ExpectationStep(this->nnf_SourceToTarget, 1, vote, newsource, upscaled);
        ExpectationStep(this->nnf_TargetToSource, 0, vote, newsource, upscaled);

        // --- MAXIMIZATION STEP ---

        // compile votes and update pixel values
        MaximizationStep(newtarget, vote);

        free(vote);
    }
    //printf("\n");

    return newtarget;
}


// Expectation Step : vote for best estimations of each pixel
void
Inpaint:: ExpectationStep(NNF* nnf, int sourceToTarget, double** vote, MaskedImage* source, int upscale)
{
    int y, x, H, W, xp, yp, dp, dy, dx;
    int xs,ys,xt,yt;
    int*** field = nnf->field;
    int R = nnf->S;
    double w;

    H = nnf->input->height;
    W = nnf->input->width;
    for ( y=0 ; y<H ; ++y) {
        for ( x=0 ; x<W; ++x) { // x,y = center pixel of patch in input

            // xp,yp = center pixel of best corresponding patch in output
            xp=field[y][x][0];
            yp=field[y][x][1];
            dp=field[y][x][2];

            // similarity measure between the two patches
            w = similarity[dp];

            // vote for each pixel inside the input patch
            for ( dy=-R ; dy<=R ; ++dy) {
                for ( dx=-R ; dx<=R; ++dx) {

                    // get corresponding pixel in output patch
                    if (sourceToTarget)
                    { xs=x+dx; ys=y+dy; xt=xp+dx; yt=yp+dy;}
                    else
                    { xs=xp+dx; ys=yp+dy; xt=x+dx; yt=y+dy; }

                    if (xs<0 || xs>=W) continue;
                    if (ys<0 || ys>=H) continue;
                    if (xt<0 || xt>=W) continue;
                    if (yt<0 || yt>=H) continue;

                    // add vote for the value
                    if (upscale) {
                        weightedCopy(source, 2*xs,   2*ys,   vote, 2*xt,   2*yt,   w);
                        weightedCopy(source, 2*xs+1, 2*ys,   vote, 2*xt+1, 2*yt,   w);
                        weightedCopy(source, 2*xs,   2*ys+1, vote, 2*xt,   2*yt+1, w);
                        weightedCopy(source, 2*xs+1, 2*ys+1, vote, 2*xt+1, 2*yt+1, w);
                    } else {
                        weightedCopy(source, xs, ys, vote, xt, yt, w);
                    }
                }
            }
        }
    }
}

void weightedCopy(MaskedImage* src, int xs, int ys, double** vote, int xd,int yd, double w)
{
    if (src->isMasked(xs, ys))
        return;

    vote[yd][ 4*xd ] += w*src->getSample(xs, ys, 0);
    vote[yd][4*xd+1] += w*src->getSample(xs, ys, 1);
    vote[yd][4*xd+2] += w*src->getSample(xs, ys, 2);
    vote[yd][4*xd+3] += w;
}


// Maximization Step : Maximum likelihood of target pixel
void MaximizationStep(MaskedImage* target, double** vote)
{
    int y, x, H, W, r, g, b;
    H = target->height;
    W = target->width;
    for( y=0 ; y<H ; ++y){
        for( x=0 ; x<W ; ++x){
            if (vote[y][4*x+3]>0) {
                r = (int) (vote[y][ 4*x ]/vote[y][4*x+3]);
                g = (int) (vote[y][4*x+1]/vote[y][4*x+3]);
                b = (int) (vote[y][4*x+2]/vote[y][4*x+3]);

                target->setSample(x, y, 0, r );
                target->setSample(x, y, 1, g );
                target->setSample(x, y, 2, b );
                target->setMask(x, y, 0);
            }
        }
    }
}


/**
* Nearest-Neighbor Field (see PatchMatch algorithm)
*  This algorithme uses a version proposed by Xavier Philippeau
*/
NNF:: NNF(MaskedImage* input, MaskedImage* output, int patchsize)
{
    this->input = input;
    this->output= output;
    this->S = patchsize;
    fieldW = input->width;
    fieldH = input->height;
    // allocate field
    field = (int***) malloc(fieldH * sizeof(int**));

    for (int i=0 ; i < fieldH ; i++ ) {
        field[i] = (int**) malloc(fieldW * sizeof(int*));
        for (int j=0 ; j<fieldW ; j++ ) {
            field[i][j] = (int*) malloc(3*sizeof(int));
        }
    }
}

// initialize field with random values
void
NNF:: randomize()
{
    for (int i=0; i<input->height; ++i){
        for (int j=0; j<input->width; ++j){
            field[i][j][0] = rand() % output->width +1;
            field[i][j][1] = rand() % output->height +1;
            field[i][j][2] = DSCALE;
        }
    }
    initializeNNF();
}

// initialize field from an existing (possibily smaller) NNF
void
NNF:: initializeNNF(NNF* otherNnf)
{
    int fx, fy, x, y, xlow, ylow;
    // field
    fx = fieldW/otherNnf->fieldW;
    fy = fieldH/otherNnf->fieldH;
    for (y=0; y<fieldH; ++y) {
        for (x=0; x<fieldW; ++x) {
            xlow = MIN(x/fx, otherNnf->input->width-1);
            ylow = MIN(y/fy, otherNnf->input->height-1);
            field[y][x][0] = otherNnf->field[ylow][xlow][0]*fx;
            field[y][x][1] = otherNnf->field[ylow][xlow][1]*fy;
            field[y][x][2] = DSCALE;
        }
    }
    initializeNNF();
}

// compute initial value of the distance term
void
NNF:: initializeNNF()
{
    int iter=0, maxretry=20;
    for (int y=0;y<this->fieldH;++y) {
        for (int x=0;x<this->fieldW;++x) {

            this->field[y][x][2] = this->distance(x,y,  this->field[y][x][0],this->field[y][x][1]);
            // if the distance is INFINITY (all pixels masked ?), try to find a better link
            iter=0;
            while ( this->field[y][x][2] == DSCALE && iter<maxretry) {
                this->field[y][x][0] = rand() % this->output->width +1;
                this->field[y][x][1] = rand() % this->output->height +1;
                this->field[y][x][2] = this->distance(x,y,  this->field[y][x][0],this->field[y][x][1]);
                iter++;
            }
        }
    }
}

// multi-pass NN-field minimization (see "PatchMatch" - page 4)
void
NNF:: minimizeNNF(int pass)
{
    int min_x=0, min_y=0;
    int max_x=this->input->width-1;
    int max_y=this->input->height-1;
    // multi-pass minimization
    for (int i=0;i<pass;i++) {
        // scanline order
        for (int y=min_y;y<=max_y;++y)
            for (int x=min_x;x<max_x;++x)
                if (this->field[y][x][2]>0)
                    minimizeLinkNNF(x,y,+1);

        // reverse scanline order
        for (int y=max_y;y>=min_y;y--)
            for (int x=max_x;x>=min_x;x--)
                if (this->field[y][x][2]>0)
                    minimizeLinkNNF(x,y,-1);
    }
}

// minimize a single link (see "PatchMatch" - page 4)
void
NNF:: minimizeLinkNNF(int x, int y, int dir)
{
    int xp,yp,dp,wi, xpi, ypi;
    //Propagation Up/Down
    if (y-dir>0 && y-dir<this->input->height) {
        xp = this->field[y-dir][x][0];
        yp = this->field[y-dir][x][1]+dir;
        dp = distance(x,y, xp,yp);
        if (dp<this->field[y][x][2]) {
            this->field[y][x][0] = xp;
            this->field[y][x][1] = yp;
            this->field[y][x][2] = dp;
        }
    }
    //Propagation Left/Right
    if (x-dir>0 && x-dir<this->input->width) {
        xp = this->field[y][x-dir][0]+dir;
        yp = this->field[y][x-dir][1];
        dp = distance(x,y, xp,yp);
        if (dp<this->field[y][x][2]) {
            this->field[y][x][0] = xp;
            this->field[y][x][1] = yp;
            this->field[y][x][2] = dp;
        }
    }
    //Random search
    wi=this->output->width;
    xpi=this->field[y][x][0];
    ypi=this->field[y][x][1];
    int r=0;
    while (wi>0) {
        r=(rand() % (2*wi)) -wi;
        xp = xpi + r;
        r=(rand() % (2*wi)) -wi;
        yp = ypi + r;
        yp = MAX(0, MIN(this->output->height-1, yp ));
        xp = MAX(0, MIN(this->output->width-1, xp ));

        dp = distance(x,y, xp,yp);
        if (dp<this->field[y][x][2]) {
            this->field[y][x][0] = xp;
            this->field[y][x][1] = yp;
            this->field[y][x][2] = dp;
        }
        wi/=2;
    }
}

// compute distance between two patch
int
NNF:: distance(int x,int y, int xp,int yp)
{
    return distanceMaskedImage(this->input,x,y, this->output,xp,yp, this->S);
}

NNF:: ~NNF()
{
    if (field != NULL ){
        for (int i=0 ; i < fieldH ; ++i ){
            for (int j=0 ; j < fieldW ; ++j ){
                free(field[i][j] );
            }
            free(field[i]);
        }
        free(field);
    }
}


// **************** Masked Image ******************

uchar** allocMask(int w, int h)
{
    int len = h * sizeof(uchar*) + w*h * sizeof(uchar);
    uchar **mask = (uchar**) malloc(len);
    if (mask==NULL){
        printf("could not allocate enough memory for mask");
        exit(1);
    }
    uchar *ptr = (uchar*)(mask + h);
    for (int i = 0; i < h; i++)
        mask[i] = (ptr + w*i);
    return mask;
}


MaskedImage:: MaskedImage(int width, int height)
{
    this->width = width;
    this->height = height;
    this->image = QImage(width, height, QImage::Format_RGB888);
    this->mask = allocMask(width, height);
}

//create mask from an image
MaskedImage:: MaskedImage(QImage image)
{
    if (image.format() != QImage::Format_RGB888) {
        image = image.convertToFormat(QImage::Format_RGB888);
    }
    this->image = image;
    this->width = image.width();
    this->height = image.height();
    this->mask = allocMask(width, height);
}

void
MaskedImage:: copyMaskFrom(uchar **oldmask)
{
    for (int y=0; y<height; y++)
        memcpy(mask[y], oldmask[y], width);
}

void
MaskedImage:: copyMaskFrom(QImage mask)
{
    #pragma omp parallel for
    for (int y=0; y<mask.height(); y++) {
        QRgb *row;
        #pragma omp critical
        { row = (QRgb*)mask.constScanLine(y);}
        for (int x=0; x<mask.width(); x++) {
            this->mask[y][x] = qRed(row[x])==0 ? 0 : 1;
        }
    }
}

MaskedImage:: ~MaskedImage()
{
    if (mask != NULL) {
        free(mask);
        mask = NULL;
    }
}


int
MaskedImage:: getSample(int x, int y, int band)
{
    return ((uchar*)image.constScanLine(y))[x*3+band];
}

void
MaskedImage:: setSample(int x, int y, int band, int value)
{
    ((uchar*)image.scanLine(y))[x*3+band] = value;
}

int
MaskedImage:: isMasked(int x, int y)
{
    return this->mask[y][x];
}

void
MaskedImage:: setMask(int x, int y, int value) {
    this->mask[y][x] = 0<value;
}

// return true if the patch contains one (or more) masked pixel
int
MaskedImage:: containsMasked(int x, int y, int S)
{
    int xs, ys;
    for (int dy=-S;dy<=S;dy++) {
        ys=y+dy;
        if (ys<0 || ys>=this->height)
            continue;
        for (int dx=-S;dx<=S;dx++) {
            xs=x+dx;
            if (xs<0 || xs>=this->width)
                continue;
            if (this->mask[ys][xs])
                return 1;
        }
    }
    return 0;
}


// return a copy of the image
MaskedImage*
MaskedImage:: copy()
{
    MaskedImage *newimg = new MaskedImage(image.copy());
    newimg->copyMaskFrom(mask);
    return newimg;
}


// return a downsampled image (factor 1/2)
MaskedImage*
MaskedImage:: downsample()
{
    int kernel[6] = {1,5,10,10,5,1};
    int xk, yk, ky, k;
    int r=0,g=0,b=0,m=0,ksum=0;
    int H = height;
    int W = width;
    int newW=W/2, newH=H/2;

    MaskedImage* newimage = new MaskedImage(newW, newH);
    for (int y=0;y<H-1;y+=2) {
        for (int x=0;x<W-1;x+=2) {
            r=0; g=0; b=0; m=0; ksum=0;

            for (int dy=-2;dy<=3;++dy) {
                yk=y+dy;
                if (yk<0 || yk>=H)
                    continue;
                ky = kernel[2+dy];
                for (int dx=-2;dx<=3;++dx) {
                    xk = x+dx;
                    if (xk<0 || xk>=W)
                        continue;

                    if (this->mask[yk][xk])
                        continue;

                    k = kernel[2+dx]*ky;
                    r+= k*this->getSample(xk, yk, 0);
                    g+= k*this->getSample(xk, yk, 1);
                    b+= k*this->getSample(xk, yk, 2);
                    ksum+=k;
                    m++;
                }
            }
            if (ksum>0) {
                r/=ksum;
                g/=ksum;
                b/=ksum;
            }

            if (m!=0) {
                newimage->setSample(x/2, y/2, 0, r);
                newimage->setSample(x/2, y/2, 1, g);
                newimage->setSample(x/2, y/2, 2, b);
                newimage->setMask(x/2, y/2, 0);
            } else {
                newimage->setMask(x/2, y/2, 1);
            }
        }
    }

    return newimage;
}

// return an upscaled image
MaskedImage*
MaskedImage:: upscale(int newW,int newH)
{
    MaskedImage* newimage = new MaskedImage(newW, newH);

    for (int y=0;y<newH;y++) {
        int ys = (y*height)/newH;
        for (int x=0;x<newW;x++) {
            // original pixel
            int xs = (x*width)/newW;

            // copy to new image
            if (!this->mask[ys][xs]) {
                newimage->setSample(x, y, 0, this->getSample(xs, ys, 0));
                newimage->setSample(x, y, 1, this->getSample(xs, ys, 1));
                newimage->setSample(x, y, 2, this->getSample(xs, ys, 2));
                newimage->setMask(x, y, 0);
            } else {
                newimage->setMask(x, y, 1);
            }
        }
    }
    return newimage;
}

// distance between two patches in two images
int distanceMaskedImage(MaskedImage *source,int xs,int ys, MaskedImage *target,int xt,int yt, int S)
{
    long double distance=0;
    long double wsum=0, ssdmax = 9*255*255;
    int xks, yks;
    int xkt, ykt;
    long double ssd;
    long res;
    int s_value, t_value, s_gx, t_gx, s_gy, t_gy;

    // for each pixel in the source patch
    for (int dy=-S ; dy<=S ; ++dy ) {
        for (int dx=-S ; dx<=S ; ++dx ) {

            xks = xs+dx;
            yks = ys+dy;
            xkt=xt+dx;
            ykt=yt+dy;
            wsum++;

            if ( xks<1 || xks>=source->width-1 ) {distance++; continue;}
            if ( yks<1 || yks>=source->height-1 ) {distance++; continue;}

            // cannot use masked pixels as a valid source of information
            if (source->isMasked(xks, yks)) {distance++; continue;}

            // corresponding pixel in the target patch
            if (xkt<1 || xkt>=target->width-1) {distance++; continue;}
            if (ykt<1 || ykt>=target->height-1) {distance++; continue;}

            // cannot use masked pixels as a valid source of information
            if (target->isMasked(xkt, ykt)) {distance++; continue;}

            ssd=0;
            for (int band=0; band<3; ++band) {
                // pixel values
                s_value = source->getSample(xks, yks, band);
                t_value = source->getSample(xkt, ykt, band);

                // pixel horizontal gradients (Gx)
                s_gx = 128+(source->getSample(xks+1, yks, band) - source->getSample(xks-1, yks, band))/2;
                t_gx = 128+(target->getSample(xkt+1, ykt, band) - target->getSample(xkt-1, ykt, band))/2;

                // pixel vertical gradients (Gy)
                s_gy = 128+(source->getSample(xks, yks+1, band) - source->getSample(xks, yks-1, band))/2;
                t_gy = 128+(target->getSample(xkt, ykt+1, band) - target->getSample(xkt, ykt-1, band))/2;

                ssd += pow((long double)s_value-t_value , 2); // distance between values in [0,255^2]
                ssd += pow((long double)s_gx-t_gx , 2); // distance between Gx in [0,255^2]
                ssd += pow((long double)s_gy-t_gy , 2); // distance between Gy in [0,255^2]
            }

            // add pixel distance to global patch distance
            distance += ssd/ssdmax;
        }
    }

    res = (int)(DSCALE*distance/wsum);
    if (res < 0 || res > DSCALE) return DSCALE;
    return res;
}


// ---------------------------------------------------------------------
//************************ Inpainting GUI *****************************-
// _____________________________________________________________________


// *********************** Inpaint Dialog ************************

InpaintDialog:: InpaintDialog(QImage &img, QWidget *parent) : QDialog(parent)
{
    this->image = img;
    setupUi(this);
    QHBoxLayout *layout = new QHBoxLayout(scrollAreaWidgetContents);
    layout->setContentsMargins(0, 0, 0, 0);
    canvas = new PaintCanvas(this);
    layout->addWidget(canvas);
    undoBtn->setEnabled(false);
    redoBtn->setEnabled(false);

    connect(brushSizeSlider, SIGNAL(valueChanged(int)), this, SLOT(changeBrushSize(int)));
    connect(drawMaskBtn, SIGNAL(toggled(bool)), this, SLOT(changeDrawMode(bool)));
    connect(undoBtn, SIGNAL(clicked()), this, SLOT(undo()));
    connect(redoBtn, SIGNAL(clicked()), this, SLOT(redo()));
    connect(zoomInBtn, SIGNAL(clicked()), this, SLOT(zoomIn()));
    connect(zoomOutBtn, SIGNAL(clicked()), this, SLOT(zoomOut()));
    connect(eraseBtn, SIGNAL(clicked()), this, SLOT(inpaint()));
    connect(acceptBtn, SIGNAL(clicked()), this, SLOT(accept()));
    connect(cancelBtn, SIGNAL(clicked()), this, SLOT(reject()));
    connect(canvas, SIGNAL(mousePressed(QPoint)), this, SLOT(onMousePress(QPoint)));
    connect(canvas, SIGNAL(mouseReleased(QPoint)), this, SLOT(onMouseRelease(QPoint)));
    connect(canvas, SIGNAL(mouseMoved(QPoint)), this, SLOT(onMouseMove(QPoint)));

    brushSizeLabel->setText("Brush Size : 16");

    brush_pen = QPen(Qt::white);
    brush_pen.setCapStyle(Qt::RoundCap);
    eraser_pen = QPen(Qt::black);
    eraser_pen.setCapStyle(Qt::RoundCap);
    setBrushSize(16);
    scaleBy(1.0);
}

void
InpaintDialog:: changeDrawMode(bool checked)
{
    this->draw_mask = checked;
}

void
InpaintDialog:: changeBrushSize(int size)
{
    setBrushSize(size);
    brushSizeLabel->setText(QString("Brush Size : %1").arg(size));
}

void
InpaintDialog:: zoomIn()
{
    scaleBy(2*scale);
    zoomLabel->setText(QString("Zoom : %1x").arg(scale));
}

void
InpaintDialog:: zoomOut()
{
    scaleBy(scale/2);
    zoomLabel->setText(QString("Zoom : %1x").arg(scale));
}

void
InpaintDialog:: undo()
{
    if (undoStack.isEmpty() or !mask.isNull()) return;
    HistoryItem item = undoStack.takeLast();
    HistoryItem redoItem = {item.x, item.y, image.copy(item.x,item.y,item.image.width(),item.image.height())};
    redoStack.append(redoItem);
    redoBtn->setEnabled(true);
    undoBtn->setEnabled(not undoStack.isEmpty());
    updateImageArea(item.x, item.y, item.image);
}

void
InpaintDialog:: redo()
{
    if (redoStack.isEmpty() or !mask.isNull()) return;
    HistoryItem item = redoStack.takeLast();
    HistoryItem undoItem = {item.x, item.y, image.copy(item.x,item.y,item.image.width(),item.image.height())};
    undoStack.append(undoItem);
    undoBtn->setEnabled(true);
    redoBtn->setEnabled(not redoStack.isEmpty());
    updateImageArea(item.x, item.y, item.image);
}

void
InpaintDialog:: keyPressEvent(QKeyEvent *ev)
{
    // prevent closing dialog on accidental Esc key press
    if (ev->key() != Qt::Key_Escape)
        return QDialog::keyPressEvent(ev);
    ev->accept();
}

void
InpaintDialog:: scaleBy(float factor)
{
    if (not mask.isNull())
        return;
    scale = factor;
    if (factor == 1.0)
        this->main_pixmap = QPixmap::fromImage(image);
    else {
        Qt::TransformationMode mode = factor<1.0? Qt::SmoothTransformation: Qt::FastTransformation;
        this->main_pixmap = QPixmap::fromImage(image.scaled(scale*image.width(), scale*image.height(),
                        Qt::IgnoreAspectRatio, mode));
    }
    canvas->setPixmap(main_pixmap);
}

void
InpaintDialog:: setBrushSize(int size)
{
    eraser_pen.setWidth(size);
    brush_pen.setWidth(size);
    // create the circular brush cursor
    QPixmap pm(size, size);
    pm.fill(QColor(0,0,0,0));
    painter.begin(&pm);
    painter.drawEllipse(0,0, size-1, size-1);
    painter.setPen(Qt::white);
    painter.drawEllipse(1,1, size-3, size-3);
    painter.end();
    canvas->setCursor(QCursor(pm));
}

// this is called when mask is null and drawing started
void
InpaintDialog:: initMask()
{
    if (scale==1.0)
        this->image_scaled = image;
    else {
        Qt::TransformationMode mode = scale<1.0? Qt::SmoothTransformation: Qt::FastTransformation;
        this->image_scaled = image.scaled(main_pixmap.width(), main_pixmap.height(),
                            Qt::IgnoreAspectRatio, mode);
    }
    this->mask = QImage(main_pixmap.width(), main_pixmap.height(), QImage::Format_RGB32);
    mask.fill(Qt::black);
    min_x = mask.width()-1;
    min_y = mask.height()-1;
    max_x = max_y = 0;
}

void
InpaintDialog:: onMousePress(QPoint pos)
{
    start_pos = pos;
    mouse_pressed = true;
}

void
InpaintDialog:: onMouseRelease(QPoint)
{
    mouse_pressed = false;
}

void
InpaintDialog:: onMouseMove(QPoint pos)
{
    if (not mouse_pressed) return;
    // create mask and lock zooming
    if (mask.isNull())
        initMask();
    if (draw_mask)
        drawMask(start_pos, pos);
    else
        eraseMask(start_pos, pos);
    start_pos = pos;
}

void
InpaintDialog:: drawMask(QPoint start, QPoint end)
{
    painter.begin(&mask);
    painter.setPen(brush_pen);
    painter.drawLine(start, end);
    painter.end();
    updateMaskedArea(start, end);
    // update mask area range
    min_x = MIN(min_x, MIN(start.x(), end.x())-brush_pen.width()/2);
    min_y = MIN(min_y, MIN(start.y(), end.y())-brush_pen.width()/2);
    max_x = MAX(max_x, MAX(start.x(), end.x())+brush_pen.width()/2);
    max_y = MAX(max_y, MAX(start.y(), end.y())+brush_pen.width()/2);
}

void
InpaintDialog:: eraseMask(QPoint start, QPoint end)
{
    painter.begin(&mask);
    painter.setPen(eraser_pen);
    painter.drawLine(start, end);
    painter.end();
    updateMaskedArea(start, end);
}

void
InpaintDialog:: updateMaskedArea(QPoint start, QPoint end)
{
    int mask_x = MAX(MIN(start.x(), end.x())-brush_pen.width()/2, 0);
    int mask_y = MAX(MIN(start.y(), end.y())-brush_pen.width()/2, 0);
    int max_mask_x = MIN(MAX(start.x(), end.x())+brush_pen.width()/2, mask.width()-1);
    int max_mask_y = MIN(MAX(start.y(), end.y())+brush_pen.width()/2, mask.height()-1);
    int mask_w = max_mask_x-mask_x +1;
    int mask_h = max_mask_y-mask_y +1;
    QImage img = image_scaled.copy(mask_x, mask_y, mask_w, mask_h);
    for (int y=0; y<mask_h; ++y){
        QRgb *img_row = (QRgb*)img.scanLine(y);
        QRgb *mask_row = (QRgb*)mask.constScanLine(y+mask_y);
        for (int x=0; x<mask_w; ++x) {
            int clr = img_row[x];
            if (qRed(mask_row[x+mask_x])) { // mask color = rgba(0,255,0,127)
                int g = 0.5*255 + 0.5*qGreen(clr);
                img_row[x] = qRgb(0.5*qRed(clr), g, 0.5*qBlue(clr));
            }
        }
    }
    painter.begin(&main_pixmap);
    painter.drawImage(QPoint(mask_x, mask_y), img);
    painter.end();
    canvas->setPixmap(main_pixmap);
}

void
InpaintDialog:: inpaint()
{
    if (mask.isNull()) return;
    // check if there is masked pixel
    min_x = MAX(min_x, 0);
    min_y = MAX(min_y, 0);
    max_x = MIN(max_x, mask.width()-1);
    max_y = MIN(max_y, mask.height()-1);
    int mask_w = max_x - min_x + 1;
    int mask_h = max_y - min_y + 1;
    int masked_count = 0;
    for (int i=min_y; i<=max_y; ++i) {
        QRgb *mask_row = (QRgb*)mask.constScanLine(i);
        for (int j=min_x; j<=max_x; ++j)
            if (qRed(mask_row[j])) masked_count++;
    }
    if (masked_count==0) return;

    // narrow masks have smaller fraction of area covered
    int masksize_min = MIN(mask_w, mask_h);
    int masksize_max = MAX(mask_w, mask_h);
    int area = masksize_max*masksize_max;
    float covered = (float)masked_count/area;
    int factor = masksize_max/masksize_min;
    //qDebug()<< masked_count << covered << factor;

    // calculate how much to increase each size
    int inc;
    if (factor>2) // narrow mask vertical or horizontal
        inc = 3*masksize_min;
    else if (covered<0.25) // narrow mask diagonal
        inc = masksize_min/2;
    else    // thick mask
        inc = masksize_min*2;

    int x = MAX(min_x - inc, 0);
    int y = MAX(min_y - inc, 0);
    int w = MIN(max_x + inc, mask.width()-1) - x +1;
    int h = MIN(max_y + inc, mask.height()-1) - y +1;
    x = x/scale;
    y = y/scale;
    w = w/scale;
    h = h/scale;
    // the inpaint function has a bug that causes slight error to the
    // right and bottom boundary if width and height are odd. so make them even
    if (w%2!=0) {
        w -= 1;
        //decrease 1px to left if mask is closer to right boundary of image
        if (min_x > mask.width()-max_x-1) x+=1;
    }
    if (h%2!=0) {
        h -= 1;
        if (min_y > mask.height()-max_y-1) y+=1;
    }
    // draw mask area
    painter.begin(&main_pixmap);
    painter.setPen(Qt::black);
    painter.drawRect(x*scale, y*scale, w*scale-1, h*scale-1);
    painter.end();
    canvas->setPixmap(main_pixmap);
    waitFor(30);
    // get mask and input image for inpaint
    QImage input_img = image.copy(x, y, w, h);
    QImage mask_img = mask.copy(x*scale, y*scale, w*scale, h*scale);
    if (scale!=1.0) mask_img = mask_img.scaled(w, h);
    // clear memory
    mask = QImage();
    image_scaled = QImage();
    //input_img.save("input.png");
    //mask_img.save("mask.png");
    // apply inpaint function
    Inpaint inp;
    QImage output = inp.inpaint(input_img, mask_img, 2);
    // add to undo stack
    redoStack.clear();
    redoBtn->setEnabled(false);
    HistoryItem item = {x, y, image.copy(x, y, output.width(), output.height())};
    undoStack.append(item);
    undoBtn->setEnabled(true);
    if (undoStack.size()>10)
        undoStack.removeFirst();
    drawMaskBtn->setChecked(true);
    updateImageArea(x, y, output);
}

void
InpaintDialog:: updateImageArea(int x, int y, QImage part)
{
    painter.begin(&image);
    painter.drawImage(QPoint(x,y), part);
    painter.end();
    scaleBy(scale);
}
