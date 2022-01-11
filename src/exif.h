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
