/* This file is a part of photoquick program, which is GPLv3 licensed */
#include "pdfwriter.h"
#include <sstream>
#include <fstream>
#include <clocale>

std::string getPngIdat(const char *rawdata, int rawdata_size);
std::string imgMatrix(float x, float y, float w, float h, int rotation);


PdfDocument:: PdfDocument()
{
    producer = "PhotoQuick by Arindamsoft";
    // this prevents using comma (,) as decimal point in string formatting
    setlocale(LC_NUMERIC, "C");
    // add Info, Catalog, Pages root dictionary
    info = new PdfObject(PDF_OBJ_DICT);
    addObject(info);
    catalog = new PdfObject(PDF_OBJ_DICT);
    addObject(catalog);
    pages_parent = new PdfObject(PDF_OBJ_DICT);
    addObject(pages_parent);
    // set values
    pages = new PdfObject(PDF_OBJ_ARRAY);
    pages_parent->add("/Type", "/Pages");
    pages_parent->add("/Kids", pages);
    catalog->add("/Type", "/Catalog");
    catalog->add("/Pages", pages_parent);
}

PdfPage*
PdfDocument:: newPage(int w, int h)
{
    PdfPage *page = new PdfPage(w, h, pages_parent);
    addObject(page);
    addObject(page->contents);
    pages->append(page);
    return page;
}

void
PdfDocument:: addObject(PdfObject *obj)
{
    obj_table.push_back(obj);
    obj->obj_no = obj_table.size();
}

PdfObject*
PdfDocument:: addImage(const char *buff, int size, int w, int h, PdfImageFormat img_format)
{
    PdfObject *img = new PdfObject(PDF_OBJ_STREAM);
    img->add("/Type", "/XObject");
    img->add("/Subtype", "/Image");
    img->add("/Width", format("%d", w));
    img->add("/Height", format("%d", h));
    if (img_format==PDF_IMG_JPEG){
        img->add("/ColorSpace", "/DeviceRGB");
        img->add("/BitsPerComponent", "8");
        img->add("/Filter", "/DCTDecode"); // jpg = DCTDecode
        img->stream = std::string(buff, size);
    }
    else if (img_format==PDF_IMG_PNG){ // monochrome only
        img->add("/ColorSpace", "[/Indexed /DeviceRGB 1 <ffffff000000>]");
        img->add("/BitsPerComponent", "1");
        img->add("/Filter", "/FlateDecode");// png = FlateDecode
        img->add("/DecodeParms", format("<</Predictor 15 /Columns %d /BitsPerComponent 1 /Colors 1>>", w));
        img->stream = getPngIdat(buff, size);
    }
    addObject(img);
    return img;
}

void
PdfDocument:: save(std::string filename)
{
    // create stream to write
    std::ofstream stream;
    stream.open(filename, std::ios::out|std::ios::binary);
    std::string header = "%%PDF-1.4\n";
    stream << header;
    // set pages count
    info->add("/Producer", format("(%s)",producer.c_str()));
    //info->add("/CreationDate", creation_date);
    pages_parent->add("/Count", format("%d", pages->array.size()));
    // write all indirect objects to file
    for (PdfObject *obj : obj_table){
        obj->offset = stream.tellp();
        stream << format("%d 0 obj\n", obj->obj_no);
        stream << obj->toString() << "\nendobj\n";
    }
    // write the cross reference table.
    /* It starts with xref keyword and followed by one or more sections.
    Each section starts with first obj number and number of entries, followed by entries
    in each line. each entry is in nnnnnnnnnn ggggg n eol format. */
    int xref_offset = stream.tellp();
    int xref_count = obj_table.size()+1;
    stream << format("xref\n0 %d\n", xref_count);
    stream << "0000000000 65535 f \n";// each line is exactly 20 bytes long
    for (PdfObject *obj : obj_table){
        stream << format("%010d 00000 n \n", obj->offset);
    }
    // write trailer dictionary
    PdfObject *trailer = new PdfObject(PDF_OBJ_DICT);
    trailer->add("/Size", format("%d", xref_count));
    trailer->add("/Root", catalog);
    trailer->add("/Info", info);
    stream << "trailer\n" << trailer->toString();
    stream << format("\nstartxref\n%d\n", xref_offset);
    stream << "%%EOF";
    stream.flush();
    stream.close();
    delete trailer;
}

PdfDocument:: ~PdfDocument()
{
    for (PdfObject *obj : obj_table){
        delete obj;
    }
}

/* ----------------- PdfOjject ------------------ */

PdfObject:: PdfObject(ObjectType type)
{
    this->type = type;
    obj_no = 0;
}

bool
PdfObject:: isIndirect()
{
    return obj_no > 0;
}

void
PdfObject:: append(PdfObject *item)
{
    array.push_back(item);
}

void
PdfObject:: add(std::string key, PdfObject *val)
{
    if (dict.count(key)>0 && !dict[key]->isIndirect()){
        delete dict[key];
    }
    dict[key] = val;
}

void
PdfObject:: add(std::string key, std::string val)
{
    PdfObject *val_obj = new PdfObject(PDF_OBJ_STRING);
    val_obj->string = val;
    this->dict[key] = val_obj;
}

std::string
PdfObject:: toString()
{
    std::string str = "";

    switch (type)
    {
    case PDF_OBJ_ARRAY:
        str += "[ ";
        for (PdfObject *obj : this->array) {
            if (obj->isIndirect()) {
                str += format("%d 0 R ", obj->obj_no);
            }
            else {
                str += obj->toString() + " ";
            }
        }
        str += "]";
        break;

    case PDF_OBJ_STREAM:
        this->add("/Length", format("%d", this->stream.size()));
    case PDF_OBJ_DICT:
        str += "<< ";
        for (auto &iter : this->dict){
            str += iter.first + " ";
            PdfObject *obj = iter.second;
            if (obj->isIndirect()) {
                str += format("%d 0 R ", obj->obj_no);
            }
            else {
                str += obj->toString() + " ";
            }
        }
        str += ">>";
        if (type==PDF_OBJ_STREAM){
            str += "\nstream\n";
            str += this->stream;
            str += "\nendstream";
        }
        break;

    case PDF_OBJ_STRING:
        return this->string;
    }
    return str;
}

PdfObject:: ~PdfObject()
{
    switch (type) {
    case PDF_OBJ_ARRAY:
        for (PdfObject *obj : array) {
            if (!obj->isIndirect()) {
                delete obj;
            }
        }
        array.clear();
        break;
    case PDF_OBJ_STREAM:
    case PDF_OBJ_DICT:
        for (auto it : dict) {
            if (!it.second->isIndirect()) {
                delete it.second;
            }
        }
        dict.clear();
        break;
    case PDF_OBJ_STRING:
    default:
        break;
    }
}

/* ************************* Pdf Page ***************************
<<
  /Type /Page
  /Parent 3 0 R
  /MediaBox [0 0 595 842]
  /Resources <</ProcSet [/PDF] /XObject <</img0 4 0 R>> >>
  /Contents 5 0 R
>>
*/
PdfPage:: PdfPage(int w, int h, PdfObject *parent) : PdfObject(PDF_OBJ_DICT)
{
    this->add("/Type", "/Page");
    this->add("/Parent", parent);
    this->add("/MediaBox", format("[0 0 %d %d]",w,h));
    x_objects = new PdfObject(PDF_OBJ_DICT);
    PdfObject *resources = new PdfObject(PDF_OBJ_DICT);
    resources->add("/ProcSet", "[/PDF]");
    resources->add("/XObject", x_objects);
    this->add("/Resources", resources);
    contents = new PdfObject(PDF_OBJ_STREAM);
    this->add("/Contents", contents);
}

void
PdfPage:: drawImage(PdfObject *img, float x, float y, float w, float h, int rotation)
{
    std::string matrix = imgMatrix(x, y, w, h, rotation);
    contents->stream += format("q %s /img%d Do Q\n", matrix.c_str(), x_objects->dict.size());
    x_objects->add(format("/img%d", x_objects->dict.size()), img);
}

void
PdfPage:: setLineColor(int r, int g, int b)
{
    contents->stream += format("/DeviceRGB CS %g %g %g SC\n", r/255.0, g/255.0, b/255.0);
}

void
PdfPage:: setFillColor(int r, int g, int b)
{
    contents->stream += format("/DeviceRGB cs %g %g %g sc\n", r/255.0, g/255.0, b/255.0);
}

const char* paint_cmd(PaintMode mode)
{
    switch (mode){
    case STROKE:
        return "S";
    case FILL:
        return "f";
    case FILL_N_STROKE:
        return "B";
    default:
        return "n";// do nothing
    }
}

void
PdfPage:: drawRect(float x, float y, float w, float h, float line_width, PaintMode mode)
{
    contents->stream += format("q %g w %.4f %.4f %g %g re %s Q\n", line_width, x, y, w, h, paint_cmd(mode));
}


/* ------------------ Parse PNG Image ------------------ */

// read 4 bytes (for network byte order)
#define read32(a, arr) \
do {\
    char aa_=0, bb_=0, cc_=0, dd_=0; \
    aa_= arr[0]; bb_= arr[1]; cc_= arr[2]; dd_= arr[3]; \
    (a) = (aa_<<24) + (bb_<<16) + (cc_<<8) + (dd_); \
    data += 4; \
} while(0)

std::string getPngIdat(const char *rawdata, int rawdata_size)
{
    // the png must be complete and valid png
    std::string idat;

    const char *data = rawdata + 8; // 1st 8 byte is png signature
    int word;
    int size = 0;
    for (int i=8; i<rawdata_size; i+= size+12) { // iterate over each chunk
        read32(size, data); // chunk size
        read32(word, data); // Chunk Header
        //printf("Header : %c%c%c%c\n", word>>24,word>>16,word>>8,word);
        //printf("Data Size : %d\n", size);
        if (word==0x49444154) { // IDAT
            idat += std::string(data, size);
        }
        data += (size+4); // CRC = 4 bytes
    }
    return idat;
}

std::string readFile(std::string filename)
{
    std::ifstream inFile(filename, std::ios::binary);
    std::stringstream strStream;
    strStream << inFile.rdbuf();
    std::string str = strStream.str();
    return str;
}
// transformation order : translate -> rotate -> scale
std::string imgMatrix(float x, float y, float w, float h, int rotation)
{
    int rot = rotation%360;
    std::string trans_str, rot_str;
    switch (rot) {
        case 0:
            trans_str = format("%.4f %.4f", x, y);
            rot_str = "1 0 0 1";
            break;
        case 90:
            trans_str = format("%.4f %.4f", x, y+h);
            rot_str = "0 -1 1 0";
            break;
        case 180:
            trans_str = format("%.4f %.4f", x+w, y+h);
            rot_str = "-1 0 0 -1";
            break;
        case 270:
            trans_str = format("%.4f %.4f", x+w, y);
            rot_str = "0 1 -1 0";
            break;
    }
    std::string matrix = format("%s %s cm ", rot_str.c_str(), trans_str.c_str()); // translate and then rotate
    if (rot==90 or rot==270) {
        float tmp = w;
        w = h;
        h = tmp;
    }
    matrix += format("%.4f 0 0 %.4f 0 0 cm", w, h);      // finally scale image
    return matrix;
}

template<typename... Args>
std::string format(const char* fmt, Args... args)
{
    int len = std::snprintf(nullptr, 0, fmt, args...);
    if (len <0) {
        std::cout << "error formatting string";
        return "";
    }
    //std::cout << len << "\n";
    char buf[len+1];
    std::snprintf(buf, len+1, fmt, args...);
    std::string str(buf);
    return str;
}
