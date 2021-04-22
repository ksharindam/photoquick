// Based on the specification given in
// https://www.media.mit.edu/pia/Research/deepview/exif.html
#include <stdio.h>
#include <map>
#include <list>
#include <sstream>
#include <cmath>

// pos at TIFF header starts
static int tiff_offset = 12;
// byte order : intel = little-endian, motorola = big-endian
static int intelBA = 0;

// read 2 bytes
#define read16(a,b) \
do {\
    int cc_=0,dd_=0; \
    if((cc_=getc((b))) == EOF || (dd_=getc((b))) == EOF)\
        return 0; \
    if (intelBA) (a) = (dd_<<8) + (cc_); \
    else (a) = (cc_<<8) + (dd_); \
} while(0)

// read 4 bytes
#define read32(a,b) \
do {\
    int aa_=0,bb_=0,cc_=0,dd_=0; \
    if((aa_=getc((b))) == EOF || (bb_=getc((b))) == EOF || (cc_=getc((b))) == EOF || (dd_=getc((b))) == EOF)\
        return 0; \
    if (intelBA) (a) = (dd_<<24) + (cc_<<16) + (bb_<<8) + (aa_); \
    else (a) = (aa_<<24) + (bb_<<16) + (cc_<<8) + (dd_); \
} while(0)

#define read64(a,b) \
do {\
    long long aa_=0,bb_=0,cc_=0,dd_=0,ee_=0,ff_=0,gg_=0,hh_=0; \
    if((aa_=getc((b))) == EOF || (bb_=getc((b))) == EOF || (cc_=getc((b))) == EOF ||\
        (dd_=getc((b))) == EOF || (ee_=getc(b))==EOF || (ff_=getc(b))==EOF ||\
        (gg_=getc(b))==EOF || (hh_=getc(b))==EOF)\
        return 0; \
    if (intelBA) (a) = (hh_<<56) + (gg_<<48) + (ff_<<40) + (ee_<<32) + (dd_<<24) + (cc_<<16) + (bb_<<8) + (aa_); \
    else (a) = (aa_<<56) + (bb_<<48) + (cc_<<40) + (dd_<<32) + (ee_<<24) + (ff_<<16) + (gg_<<8) + (hh_); \
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
    read16(word,f);
    // some jpgs contain App0 segment just before App1 segment
    if (word == 0xFFE0) {//App0 Marker (JFIF)
        read16(word,f); // JFIF size
        fseek(f, word-2, SEEK_CUR);
        read16(word,f);// read next header
    }
    if (word != 0xFFE1){// App1 marker (Exif)
        return 0;
    }
    read16(word,f);// App1 marker size (Motorola byte align)
    //printf("exif data size : %d\n", word);
    if (!(getc(f)=='E' && getc(f)=='x' && getc(f)=='i' && getc(f)=='f'))
        return 0;
    //else printf("Format is exif\n");
    read16(word, f); // null bytes
    // TIFF header
    tiff_offset = ftell(f);
    read16(word,f);
    if (word==0x4949){
        intelBA = 1;
        //printf("Align : Intel\n");
    }
    //else printf("Align : Motorola\n");//0x4d4d
    read16(word, f);//tag mark (0x002a)
    read32(word, f); // first IFD offset
    // go to first IFD
    fseek(f, tiff_offset+word, SEEK_SET);
    read16(word, f); // word = no. of entries in IFD
    for (int i=0; i<(int)word; ++i){
        read16(word, f);
        if (word!=0x0112) {// not Orientation tag
            fseek(f,10,SEEK_CUR);
            continue;
        }
        read16(word,f);// data format
        read32(word,f);// no. of components
        read16(word,f);// data value (or offset to that val if val size > 4bytes)
        return word;
    }
    return 0;
}

//------------************ Image Exif Reader ************--------------

enum {
    Tag_Make              = 0x010f,
    Tag_Model             = 0x0110,
    Tag_Orientation       = 0x0112,
    Tag_DateTime          = 0x0132,
    Tag_ExposureTime      = 0x829a,
    Tag_FNumber           = 0x829d,
    Tag_ExifOffset        = 0x8769,
    Tag_ISOSpeedRatings   = 0x8827,
    Tag_DateTimeOriginal  = 0x9003,
    Tag_BrightnessValue   = 0x9203,
    Tag_MeteringMode      = 0x9207,
    Tag_LightSource       = 0x9208,
    Tag_Flash             = 0x9209,
    Tag_FocalLength       = 0x920a,
};

// printable tag types
static std::map<int, std::string> tag_names =
{
    {Tag_Make,              "Make"},
    {Tag_Model,             "Model"},
    {Tag_Orientation,       "Orientation"},
    {Tag_DateTime,          "DateTime"},
    {Tag_ExposureTime,      "Exposure Time"},
    {Tag_FNumber,           "Aperture"},
    {Tag_ExifOffset,        "ExifOffset"},
    {Tag_ISOSpeedRatings,   "ISO"},
    {Tag_DateTimeOriginal,  "DateTimeOriginal"},
    {Tag_BrightnessValue,   "Brightness"},
    {Tag_MeteringMode,      "Metering Mode"},
    {Tag_LightSource,       "Light Source"},
    {Tag_Flash,             "Flash"},
    {Tag_FocalLength,       "Focal Length"}
};

// tag names will be printed in this order
static std::list<int> ordered_tags = {
    Tag_Orientation, Tag_Make, Tag_Model, Tag_DateTime, Tag_DateTimeOriginal,
    Tag_FNumber, Tag_FocalLength,
    Tag_ExposureTime, Tag_ISOSpeedRatings, Tag_BrightnessValue,
    Tag_Flash, Tag_LightSource, Tag_MeteringMode
};

static const char* metering_modes[] = {
    "average", "center weighted", "spot", "multi-spot", "multi-segment"
};

// Exif Tag data types (dont modify this)
enum {
    U_BYTE=1,
    STRING,// 1 byte
    U_SHORT,//2 bytes
    U_LONG,// 4 bytes
    U_RATIONAL,//8 bytes (4 bytes numerator, 4 bytes denominator)
    BYTE,
    UNDEFINED,// 1 byte
    SHORT,
    LONG,
    RATIONAL,
    FLOAT,// 4 bytes
    DOUBLE// 8 bytes
};

static int get_component_size(int data_format)
{
    switch (data_format) {
        case SHORT:
        case U_SHORT:
            return 2;
        case LONG:
        case U_LONG:
        case FLOAT:
            return 4;
        case DOUBLE:
        case RATIONAL:
        case U_RATIONAL:
            return 8;
        default:    // BYTE, U_BYTE, STRING, UNDEFINED = 1
            return 1;
    }
}



enum {
    VAL_UNKNOWN,
    VAL_STRING,
    VAL_INTEGER,
    VAL_REAL,
    VAL_FRACTION,
};

typedef struct {
    int numer;
    int denom;
} Fraction;

typedef struct {
    int type;
    int integer;
    double real;
    Fraction fraction;
    char *str;
} TagValue;

static std::map<int, TagValue> tag_vals;

// tag_no must be in tag_names dict
static void stream_add_tag_info (std::ostringstream &stream, int tag_no, TagValue tag_val)
{
    stream << tag_names[tag_no] << " : ";
    double val;
    switch (tag_no) {
        case Tag_ExposureTime:
            if (tag_val.type==VAL_FRACTION && tag_val.fraction.numer!=0) {
                val = (double)tag_val.fraction.denom / tag_val.fraction.numer;
                stream << "1/" << round(val) << " sec\n";
                return;
            }
            break;
        case Tag_FNumber:
            if (tag_val.type==VAL_FRACTION && tag_val.fraction.denom!=0) {
                val = (double)tag_val.fraction.numer / tag_val.fraction.denom;
                stream << "f/" << val << "\n";
                return;
            }
            break;
        case Tag_BrightnessValue:
            if (tag_val.type==VAL_FRACTION && tag_val.fraction.denom!=0) {
                val = (double)tag_val.fraction.numer / tag_val.fraction.denom;
                stream << (val>0 ? "+" : "") << val << " EV\n";
                return;
            }
            break;
        case Tag_MeteringMode:
            if (tag_val.type==VAL_INTEGER && tag_val.integer>0 && tag_val.integer<6) {
                stream << metering_modes[tag_val.integer-1] << "\n";
                return;
            }
            break;
        case Tag_FocalLength:
            if (tag_val.type==VAL_FRACTION && tag_val.fraction.denom!=0) {
                val = (double)tag_val.fraction.numer / tag_val.fraction.denom;
                stream << val << " cm\n";
                return;
            }
            break;
        default:
            break;
    }
    switch (tag_val.type) {
        case VAL_STRING:
            stream << tag_val.str;
            break;
        case VAL_INTEGER:
            stream << tag_val.integer;
            break;
        case VAL_REAL:
            stream << tag_val.real;
            break;
        case VAL_FRACTION:
            stream << tag_val.fraction.numer << "/" << tag_val.fraction.denom;
    }
    stream << "\n";
}

static int read_tag_val(FILE *f, TagValue *tag_val, int data_type, int comp_size, int comp_count)
{
    int int_num;
    long long long_num;

    if (comp_size==1) {
        tag_val->str = (char*) malloc(comp_count );
        for (int i=0; i<comp_count && (int_num=getc(f))!=EOF; i++) {
            tag_val->str[i] = int_num;
        }
        tag_val->type = VAL_STRING;
    }
    else if (comp_size==2) {
        read16(tag_val->integer, f);
        tag_val->type = VAL_INTEGER;
    }
    else if (comp_size==4) {
        read32(int_num, f);
        switch (data_type) {
            case LONG:
            case U_LONG:
                tag_val->type = VAL_INTEGER;
                tag_val->integer = int_num;
                break;
            case FLOAT:
                tag_val->type = VAL_REAL;
                float *float_val = (float*)&int_num;
                tag_val->real = *float_val ;
        }
    }
    else if (comp_size==8) {
        switch (data_type) {
            case RATIONAL:
            case U_RATIONAL:
                tag_val->type = VAL_FRACTION;
                read32(tag_val->fraction.numer, f);
                read32(tag_val->fraction.denom, f);
                break;
            case DOUBLE:
                read64(long_num, f);
                tag_val->type = VAL_REAL;
                double *double_val = (double*)&long_num;
                tag_val->real = *double_val;
        }
    }
    return 1;
}

// read image file directory
static int read_IFD(FILE *f)
{
    int comp_size, data_size;
    unsigned int word, tags_count, tag_no, data_format, components_count;
    read16(tags_count, f); // no. of entries in IFD
    //printf("\nCount : %d\n", tags_count);
    for (int i=0; i<(int)tags_count; ++i)
    {
        read16(tag_no, f);
        read16(data_format, f);
        read32(components_count, f);
        size_t pos = ftell(f);
        //printf("Tag No. %d\n", tag_no);
        // if we dont know about this tag skip this
        if (tag_names.count(tag_no) < 1)
            goto seek_next;
        // calculate data size
        comp_size = get_component_size(data_format);
        data_size = comp_size * components_count;
        // if data size > 4 , get data offset and go to that pos
        if (data_size > 4) {
            read32(word, f);
            fseek(f, tiff_offset+word, SEEK_SET);
        }
        // data value
        //printf("0x%x,  %d,  %d,  %d\n", tag_no, data_format, comp_size, components_count);
        TagValue tag_val;
        if (!read_tag_val(f, &tag_val, data_format, comp_size, components_count))
            return 0;
        tag_vals[tag_no] = tag_val;

        if (tag_no==Tag_ExifOffset){
            fseek(f, tiff_offset+tag_val.integer, SEEK_SET);
            if (!read_IFD(f))
                return 0;
        }
seek_next:
        fseek(f, pos+4, SEEK_SET);
    }
    return 1;
}

int read_Exif(FILE *f, std::string &exif_str)
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
    read16(word,f);
    // some jpgs contain App0 segment just before App1 segment
    if (word == 0xFFE0) {//App0 Marker (JFIF)
        read16(word,f); // JFIF size
        fseek(f, word-2, SEEK_CUR);
        read16(word,f);// read next header
    }
    if (word != 0xFFE1){// App1 marker (Exif)
        //printf("not exif\n");
        return 0;
    }
    read16(word,f);// App1 data size (Motorola byte align)
    //printf("exif data size : %d\n", word);
    if (!(getc(f)=='E' && getc(f)=='x' && getc(f)=='i' && getc(f)=='f'))// Exif header
        return 0;
    //else printf("Format is exif\n");
    read16(word, f); // null bytes
    // TIFF header
    tiff_offset = ftell(f);
    read16(word,f);
    if (word==0x4949){
        intelBA = 1;//Intel Byte align (Little Endian)
    }// everything from this is byte aligned
    read16(word, f);// tag mark (0x002a)
    read32(word, f); // first IFD offset from tiff header
    // go to first IFD
    fseek(f, tiff_offset+word, SEEK_SET);
    if (!read_IFD(f))
        return 0;
    // create tag string
    std::ostringstream stream;
    for (auto tag_no : ordered_tags) {
        if (tag_vals.count(tag_no)>0)
            stream_add_tag_info(stream, tag_no, tag_vals[tag_no]);
    }
    exif_str += stream.str();
    // free data
    for (auto it : tag_vals) {
        if (it.second.type==VAL_STRING)
            free(it.second.str);
    }
    tag_vals.clear();
    return 1;
}
