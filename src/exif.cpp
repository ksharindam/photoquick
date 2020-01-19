// Based on the specification given in
// https://www.media.mit.edu/pia/Research/deepview/exif.html
#include <stdio.h>

static int intelBA = 0;

#define readbyte(a,f) \
do {\
    if(((a)=getc((f))) == EOF) \
        return 0;\
}\
while (0)

// read 2 bytes
#define read2b(a,b) \
do {\
    int cc_=0,dd_=0; \
    if((cc_=getc((b))) == EOF || (dd_=getc((b))) == EOF)\
        return 0; \
    if (intelBA) (a) = (dd_<<8) + (cc_); \
    else (a) = (cc_<<8) + (dd_); \
} while(0)

// read 4 bytes
#define read4b(a,b) \
do {\
    int aa_=0,bb_=0,cc_=0,dd_=0; \
    if((aa_=getc((b))) == EOF || (bb_=getc((b))) == EOF || (cc_=getc((b))) == EOF || (dd_=getc((b))) == EOF)\
        return 0; \
    if (intelBA) (a) = (dd_<<24) + (cc_<<16) + (bb_<<8) + (aa_); \
    else (a) = (aa_<<24) + (bb_<<16) + (cc_<<8) + (dd_); \
} while(0)


int getOrientation(FILE *f)
{
    if (!f)
        return 0;
    intelBA = 0;
    unsigned int word=0;
    fseek(f, 0, SEEK_SET);
    // Start of Image (SOI) marker
    if ( getc(f) != 0xFF || getc(f) != 0xD8 ){
        //printf("not jpg\n");
        return 0;
    }
    if (getc(f)!=0xFF || getc(f)!= 0xE1){ //App1 marker
        //printf("not exif\n");
        return 0;
    }
    read2b(word,f);// App1 marker size (Motorola byte align)
    //printf("exif data size : %d\n", word);
    if (!(getc(f)=='E' && getc(f)=='x' && getc(f)=='i' && getc(f)=='f'))
        return 0;
    //else printf("Format is exif\n");
    read2b(word, f); // null bytes
    // TIFF header
    read2b(word,f);
    if (word==0x4949){
        intelBA = 1;
        //printf("Align : Intel\n");
    }
    //else printf("Align : Motorola\n");//0x4d4d
    read2b(word, f);//tag mark (0x002a)
    read4b(word, f); // first IFD offset
    // go to first IFD
    fseek(f, word-8, SEEK_CUR);
    read2b(word, f); // word = no. of entries in IFD
    for (int i=0; i<(int)word; ++i){
        read2b(word, f);
        if (word!=0x0112) {
            fseek(f,10,SEEK_CUR);
            continue;
        }
        read2b(word,f);// data format
        read4b(word,f);// no. of components
        read2b(word,f);// data value (or offset to that val if val size > 4bytes)
        return word;
    }
    return 0;
}
