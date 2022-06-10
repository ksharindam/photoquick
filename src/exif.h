#pragma once
#include <string>
#include <cstdio>
#include <map>


typedef struct {
    unsigned short tag_no;
    short data_format;// original data format in tag
    int comp_count;// component count
    // values
    char *str;
    int integer;
    double real;
    int fraction[2];
} ExifTag;

typedef std::map<int, ExifTag> ExifInfo;

// get jpeg image orientation from exif
int getOrientation(FILE *f);

// read some jpeg exif data as string
int exif_read(ExifInfo &exif, FILE *f);

std::string exif_to_string(ExifInfo &exif);

void exif_free(ExifInfo &exif);

bool write_jpeg_with_exif(const char *jpg, int jpg_size,
                        const char *thumbnail, int thumb_size, ExifInfo exif, FILE *out);

// Known exif tags ids
enum {
    Tag_Compression       = 0x0103,// U_SHORT
    Tag_Make              = 0x010f,// STRING
    Tag_Model             = 0x0110,// STRING
    Tag_Orientation       = 0x0112,// U_SHORT
    Tag_XResolution       = 0x011a,// U_RATIONAL
    Tag_YResolution       = 0x011b,// U_RATIONAL
    Tag_ResolutionUnit    = 0x0128,// U_SHORT
    Tag_Software          = 0x0131,// STRING
    Tag_DateTime          = 0x0132,// STRING * 20
    Tag_JpegIFOffset      = 0x0201,// U_LONG
    Tag_JpegIFByteCount   = 0x0202,// U_LONG
    Tag_ExposureTime      = 0x829a,// U_RATIONAL
    Tag_FNumber           = 0x829d,// U_RATIONAL
    Tag_ExifOffset        = 0x8769,// U_LONG
    Tag_ISOSpeedRatings   = 0x8827,// U_SHORT * 2
    Tag_ExifVersion       = 0x9000,// UNDEFINED * 4
    Tag_DateTimeOriginal  = 0x9003,// STRING * 20
    Tag_BrightnessValue   = 0x9203,// RATIONAL
    Tag_MeteringMode      = 0x9207,// U_SHORT
    Tag_LightSource       = 0x9208,// U_SHORT
    Tag_Flash             = 0x9209,// U_SHORT
    Tag_FocalLength       = 0x920a,// U_RATIONAL
};

// Exif Tag data types according to exif specification (dont modify this)
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
