// Based on the specification given in
// https://www.media.mit.edu/pia/Research/deepview/exif.html
#include "exif.h"

// pos at TIFF header starts
thread_local int tiff_offset = 12;
// byte order : intel = little-endian, motorola = big-endian
thread_local int intelBA = 0;

bool isBigEndian()
{
    int i=1; return ! *((char *)&i);
}
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

// known tags that will be read
static std::map<int, std::string> tag_names =
{
    {Tag_Make,              "Make"},
    {Tag_Model,             "Model"},
    {Tag_Orientation,       "Orientation"},
    {Tag_Software,          "Software"},
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

// IFD0 and SubIFD tags that will be saved
static std::list<int> ifd0_entries = {
    Tag_Make, Tag_Model, Tag_Orientation, Tag_XResolution, Tag_YResolution, Tag_ResolutionUnit,
    Tag_Software, Tag_DateTime/*, Tag_ExifOffset*/
};

static std::list<int> subifd_entries = {
    Tag_ExifVersion, Tag_ExposureTime, Tag_FNumber, Tag_ISOSpeedRatings, Tag_DateTimeOriginal,
    Tag_BrightnessValue, Tag_MeteringMode, Tag_LightSource, Tag_Flash, Tag_FocalLength
};

// tag names will be printed in this order
static std::list<int> tags_to_display = {
    Tag_Orientation, Tag_Make, Tag_Model, Tag_Software, Tag_DateTime, Tag_DateTimeOriginal,
    Tag_FNumber, Tag_FocalLength,
    Tag_ExposureTime, Tag_ISOSpeedRatings, Tag_BrightnessValue,
    Tag_Flash, Tag_LightSource, Tag_MeteringMode
};

static const char* metering_modes[] = {
    "average", "center weighted", "spot", "multi-spot", "multi-segment"
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
        default:    // BYTE, U_BYTE, STRING, UNDEFINED
            return 1;
    }
}



static int read_tag_val(ExifTag *tag, FILE *f)
{
    int int_num;
    long long long_num;
    float *float_val;
    double *double_val;

    switch (tag->data_format) {
        case BYTE:
        case U_BYTE:
        case STRING:
        case UNDEFINED:
            tag->str = (char*) malloc(tag->comp_count);
            for (int i=0; i<tag->comp_count && (int_num=getc(f))!=EOF; i++) {
                tag->str[i] = int_num;
            }
            break;
        case SHORT:
        case U_SHORT:
            read16(tag->integer, f);
            break;
        case LONG:
        case U_LONG:
            read32(tag->integer, f);
            break;
        case FLOAT:
            read32(int_num, f);
            float_val = (float*)&int_num;
            tag->real = *float_val;
            break;
        case DOUBLE:
            read64(long_num, f);
            double_val = (double*)&long_num;
            tag->real = *double_val;
            break;
        case RATIONAL:
        case U_RATIONAL:
            read32(tag->fraction[0], f);
            read32(tag->fraction[1], f);
            break;
        default:
            return 0;
    }
    return 1;
}

// read image file directory
static int read_IFD(ExifInfo &exif, FILE *f)
{
    int data_size;
    unsigned int word, tags_count, tag_no, data_format, components_count;
    read16(tags_count, f); // no. of entries in IFD
    //printf("\nTags Count : %d\n", tags_count);
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
        data_size = components_count * get_component_size(data_format);
        // if data size > 4 , get data offset and go to that pos
        if (data_size > 4) {
            read32(word, f);
            fseek(f, tiff_offset+word, SEEK_SET);
        }
        // data value
        //printf("Tag 0x%x : Data format - %d, Comp Count - %d\n", tag_no, data_format, components_count);
        ExifTag tag;
        tag.tag_no = tag_no;
        tag.data_format = data_format;
        tag.comp_count = components_count;
        if (!read_tag_val(&tag, f))
            return 0;
        exif[tag_no] = tag;

        if (tag_no==Tag_ExifOffset){
            fseek(f, tiff_offset+tag.integer, SEEK_SET);
            if (!read_IFD(exif, f)) {
                printf("Exif : failed to read SubIFD\n");
                return 0;
            }
        }
seek_next:
        fseek(f, pos+4, SEEK_SET);
    }
    return 1;
}

int exif_read(ExifInfo &exif, FILE *f)
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
    if (!read_IFD(exif, f)) {
        printf("Exif : failed to read IFDs\n");
        return 0;
    }
    return 1;
}

// tag_no must be in tag_names dict
static void stream_add_tag_info (std::ostringstream &stream, ExifTag tag)
{
    stream << tag_names[tag.tag_no] << " : ";

    switch (tag.data_format) {
        case BYTE:
        case U_BYTE:
        case STRING:
        case UNDEFINED:
            stream << tag.str;
            break;
        case SHORT:
        case U_SHORT:
        case LONG:
        case U_LONG:
            if (tag.tag_no==Tag_MeteringMode && tag.integer>0 && tag.integer<6) {
                stream << metering_modes[tag.integer-1];
            }
            else
                stream << tag.integer;
            break;
        case FLOAT:
        case DOUBLE:
            stream << tag.real;
            break;
        case RATIONAL:
        case U_RATIONAL:
        {
            double val;
            switch (tag.tag_no) {
            case Tag_ExposureTime:
                if (tag.fraction[0]!=0) {
                    val = (double)tag.fraction[1] / tag.fraction[0];
                    stream << "1/" << round(val) << " sec";
                }
                break;
            case Tag_FNumber:
                if (tag.fraction[1]!=0) {
                    val = (double)tag.fraction[0] / tag.fraction[1];
                    stream << "f/" << val;
                }
                break;
            case Tag_BrightnessValue:
                if (tag.fraction[1]!=0) {
                    val = (double)tag.fraction[0] / tag.fraction[1];
                    stream << (val>0 ? "+" : "") << val << " EV";
                }
                break;
            case Tag_FocalLength:
                if (tag.fraction[1]!=0) {
                    val = (double)tag.fraction[0] / tag.fraction[1];
                    stream << val << " cm";
                }
                break;
            default:
                stream << tag.fraction[0] << "/" << tag.fraction[1];
                break;
            }// end switch tag_no
            break;
        }
        default:
            break;
    }
    stream << "\n";
}

// creates a printable exif info string
std::string exif_to_string(ExifInfo &exif)
{
    std::ostringstream stream;
    for (auto tag_no : tags_to_display) {
        if (exif.count(tag_no)>0)
            stream_add_tag_info(stream, exif[tag_no]);
    }
    return stream.str();
}

// free data
void exif_free(ExifInfo &exif)
{
    for (auto it : exif) {
        switch (it.second.data_format) {
            case BYTE:
            case U_BYTE:
            case STRING:
            case UNDEFINED:
                if (it.second.comp_count!=0)
                    free(it.second.str);
                break;
            default:
                break;
        }
    }
    exif.clear();
}

static bool same_byte_order = false;

static void convert_byte_order(char *src, char *dst, int size)
{
    if (same_byte_order) {
        memcpy(dst, src, size);
        return;
    }
    for (int i=0; i<size; i++) {
        dst[i] = src[size-1-i];
    }
}

short align_short(short num)
{
    short val;
    convert_byte_order((char*)&num, (char*)&val, 2);
    return val;
}

int align_int(int num)
{
    int val;
    convert_byte_order((char*)&num, (char*)&val, 4);
    return val;
}

float align_float(float num)
{
    float val;
    convert_byte_order((char*)&num, (char*)&val, 4);
    return val;
}

double align_double(double num)
{
    double val;
    convert_byte_order((char*)&num, (char*)&val, 8);
    return val;
}

short read_short(const char *ptr, int &pos)
{
    const char *buf = ptr+pos;
    short val = *((short*)buf);
    pos += 2;
    return align_short(val);
}

static void IFD_add_entry(std::string &ifd, std::string &data, int &data_offset, ExifTag tag)
{
    short tag_no = align_short(tag.tag_no);
    short data_format = align_short(tag.data_format);
    int count = align_int(tag.comp_count);
    int data_size = tag.comp_count * get_component_size(tag.data_format);
    int al_offset = align_int(data_offset);
    // [Tag Number] [Data format] [component count] [Data]
    // [2 bytes]    [2 bytes]     [4 bytes]         [4 bytes]
    std::string ent;
    ent.append((char*)&tag_no, 2);
    ent.append((char*)&data_format, 2);
    ent.append((char*)&count, 4);

    short short_val;
    int int_val, numer, denom;
    float float_val;
    double real_val;
    switch (tag.data_format) {
        case BYTE:
        case U_BYTE:
        case STRING:
        case UNDEFINED:
            if (data_size>4) {
                ent.append((char*)&al_offset, 4);
                data.append(tag.str, tag.comp_count);
                data_offset += data_size;
            }
            else {
                ent.append(tag.str, tag.comp_count);
                ent.append(4-data_size, '\0');
            }
            break;
        case SHORT:
        case U_SHORT:
            short_val = align_short((short) tag.integer);
            ent.append((char*)&short_val, 2);
            ent.append(2, '\0');// append two null bytes to make total 4 bytes
            break;
        case LONG:
        case U_LONG:
            int_val = align_int(tag.integer);
            ent.append((char*)&int_val, 4);
            break;
        case FLOAT:
            float_val = (float)align_float(tag.real);
            ent.append((char*)&float_val, 4);
            break;
        case DOUBLE:
            ent.append((char*)&al_offset, 4);
            real_val = align_double(tag.real);
            data.append((char*)&real_val, 8);
            data_offset += 8;
            break;
        case RATIONAL:
        case U_RATIONAL:
            ent.append((char*)&al_offset, 4);
            numer = align_int(tag.fraction[0]);
            denom = align_int(tag.fraction[1]);
            data.append((char*)&numer, 4);
            data.append((char*)&denom, 4);
            data_offset += 8;
            break;
        default:// should not happen
            ent.append(4, '\0');
            break;
    }
    ifd.append(ent);
}


// create data for exif segment (App1)
std::string create_exif_data(ExifInfo exif, const char *thumbnail, int thumb_size)
{
    same_byte_order = isBigEndian();
    // Check if we have known tags
    short ifd0_tags_count=1/*that 1 is ExifOffset*/, subifd_tags_count=0;

    for (int tag_no : ifd0_entries) {
        if (exif.count(tag_no)>0)
            ifd0_tags_count++;
    }
    for (int tag_no : subifd_entries) {
        if (exif.count(tag_no)>0)
            subifd_tags_count++;
    }
    // Change some tag values
    if (exif.count(Tag_Software)==0) {
        ExifTag software = {Tag_Software, STRING, 11, NULL, 0, 0.0, {0,1}};
        software.str = (char*) malloc(11);
        memcpy(software.str, "PhotoQuick", 11);
        exif[Tag_Software] = software;
        ifd0_tags_count++;
    }
    if (exif.count(Tag_ExifOffset)==0) {
        ExifTag exif_offset = {Tag_ExifOffset, U_LONG, 1, NULL, 0, 0.0, {0,1}};
        exif[Tag_ExifOffset] = exif_offset;
    }
    ExifTag exif_version = {Tag_ExifVersion, UNDEFINED, 4, NULL, 0, 0.0, {0,1}};
    exif_version.str = (char*) malloc(4);
    memcpy(exif_version.str, "0220", 4);
    exif[Tag_ExifVersion] = exif_version;
    subifd_tags_count++;

    // Exif Header
    std::string exif_data = "Exif";
    exif_data.append(2, '\0');
    // Tiff Header
    exif_data.append("MM");//motorola byte order
    short tag_mark = align_short((short)0x002A);
    exif_data.append((char*)&tag_mark, 2);

    int ifd0_offset = align_int(8);
    exif_data.append((char*)&ifd0_offset, 4);
    int data_offset = 8;

    // add IFD0

    std::string ifd0;
    std::string ifd0_data;
    data_offset += 2 /*entry count*/ + ifd0_tags_count*12 /*entries*/ + 4/*ifd1 offset*/;

    short al_tags_count = align_short(ifd0_tags_count);
    ifd0.append((char*)&al_tags_count, 2);

    for (int tag_no : ifd0_entries) {
        if (exif.count(tag_no)>0) {
            IFD_add_entry(ifd0, ifd0_data, data_offset, exif[tag_no]);
        }
    }
    exif[Tag_ExifOffset].integer = data_offset;// SubIFD offset
    IFD_add_entry(ifd0, ifd0_data, data_offset, exif[Tag_ExifOffset]);

    // add SubIFD

    std::string ifd;
    std::string ifd_data;
    data_offset += 2 /*entry count*/ + subifd_tags_count*12 /*entries*/ + 4/*next ifd offset*/;

    al_tags_count = align_short(subifd_tags_count);
    ifd.append((char*)&al_tags_count, 2);

    for (int tag_no : subifd_entries) {
        if (exif.count(tag_no)>0)
            IFD_add_entry(ifd, ifd_data, data_offset, exif[tag_no]);
    }
    ifd.append(4, '\0'); // 0 means no next ifd

    if (thumbnail) {
        int al_ifd1_offset = align_int(data_offset);
        ifd0.append((char*)&al_ifd1_offset ,4);// link to next ifd (ifd1)
    }
    else {
        ifd0.append(4, '\0'); // no linked ifd
    }
    exif_data.append(ifd0);
    exif_data.append(ifd0_data);
    exif_data.append(ifd);
    exif_data.append(ifd_data);
    ifd.clear();
    ifd_data.clear();

    // add thumbnail
    if (thumbnail) {
        int ifd1_tags_count = 3;
        data_offset += 2 /*entry count*/ + ifd1_tags_count*12 /*entries*/ + 4/*next ifd offset*/;
        ExifTag compression =     { Tag_Compression, U_SHORT, 1, NULL, 6, 0.0, {0,1}};
        ExifTag jpegIFOffset =    { Tag_JpegIFOffset, U_LONG, 1, NULL, data_offset, 0.0, {0,1}};
        ExifTag jpegIFByteCount = {Tag_JpegIFByteCount, U_LONG, 1, NULL, thumb_size, 0.0, {0,1}};

        ifd_data.append(thumbnail, thumb_size);
        data_offset += thumb_size;

        al_tags_count = align_short(ifd1_tags_count);
        ifd.append((char*)&al_tags_count, 2);
        IFD_add_entry(ifd, ifd_data, data_offset, compression);
        IFD_add_entry(ifd, ifd_data, data_offset, jpegIFOffset);
        IFD_add_entry(ifd, ifd_data, data_offset, jpegIFByteCount);
        ifd.append(4, '\0');
        exif_data.append(ifd);
        exif_data.append(ifd_data);
    }
    return exif_data;
}


bool write_jpeg_with_exif(const char *jpg, int jpg_size,
                        const char *thumbnail, int thumb_size, ExifInfo exif, FILE *out)
{
    std::string exif_data = create_exif_data(exif, thumbnail, thumb_size);

    same_byte_order = isBigEndian();

    const char *ptr = jpg;
    int pos = 2;
    unsigned short size, marker;
    marker = read_short(ptr, pos);

    while (marker == 0xFFE0 || marker == 0xFFE1) {
        size = read_short(ptr, pos);// segment size
        pos += size-2;// skip data area
        marker = read_short(ptr, pos);// next segment header
    }
    pos -= 2;

    short soi = align_short(0xFFD8);
    short app1 = align_short(0xFFE1);
    short app1_size = align_short(2 + exif_data.size());

    if (fwrite((char*)&soi, 2, 1, out) &&
        fwrite((char*)&app1, 2, 1, out) &&
        fwrite((char*)&app1_size, 2, 1, out) &&
        fwrite(exif_data.data(), exif_data.size(), 1, out) &&
        fwrite(ptr+pos, jpg_size-pos, 1, out) )
        return true;
    return false;
}
