#include "pdfwriter.h"
#include <clocale>

PdfWriter:: PdfWriter()
{
    // this prevents using comma (,) as decimal point in string formatting
    setlocale(LC_NUMERIC, "C");
    version = "1.4";
    producer = "PDF Writer by Arindam";
    header = format("%%PDF-%s\n", version.c_str());
}

void
PdfWriter:: begin(std::string filename)
{
    stream.open(filename, std::ios::out|std::ios::binary);
    stream << header;
    offset = stream.tellp();
}

PdfObj
PdfWriter:: createPage(int w, int h, std::string Contents, std::string Resources)
{
    PdfObj page;
    page.set("Type", "/Page");
    page.set("MediaBox", format("[0 0 %d %d]",w,h));
    page.set("Parent", "3 0 R");
    page.set("Resources", Resources);
    page.set("Contents", Contents);
    return page;
}

void
PdfWriter:: addPage(PdfObj &page)
{
    addObj(page);
    pages.push_back(page.id);
}

int
PdfWriter:: addObj(PdfObj &obj, std::string Stream, int id)
{
    if (id) {
        obj.id = id;
        obj_offsets.push_front(offset);
    }
    else {
        obj.id = obj_offsets.size() + 4;
        obj_offsets.push_back(offset);
    }
    std::string strng = obj.toString(Stream);
    stream << strng;
    stream.flush();
    offset = stream.tellp();
    return obj.id;   // object identifier
}

void
PdfWriter:: finish()
{
    //save catalog, xref table and close file
    PdfObj pages_obj;
    pages_obj.set("Type", "/Pages");
    pages_obj.set("Count", format("%d", pages.size()));
    pages_obj.set("Kids", pages);
    addObj(pages_obj, "", 3);
    PdfObj info;
    info.set("Producer", format("(%s)",producer.c_str()));
    // info.set("CreationDate", creation_date);
    addObj(info, "", 2);
    PdfObj catalog;
    catalog.set("Type", "/Catalog");
    catalog.set("Pages", pages_obj);
    addObj(catalog, "", 1);
    // Create the xref table
    int xref_count = obj_offsets.size()+1;
    std::string xref = format("xref\n0 %d\n", xref_count);
    xref += "0000000000 65535 f \n";
    for (int offset : obj_offsets)
        xref += format("%010d 00000 n \n", offset) ;
    PdfDict trailer;
    trailer.set("Size", format("%d", xref_count));
    trailer.set("Root", catalog.byref());
    trailer.set("Info", info.byref());
    xref += "trailer\n" + trailer.toString();
    xref += format("startxref\n%d\n", offset);
    stream << xref;
    stream << "%%EOF";
    stream.close();
}


PdfObj:: PdfObj()
{
}

std::string
PdfObj:: toString(std::string stream)
{
    // must be called after adding obj to writer otherwise id will be 0
    if (not stream.empty())
        content.set("Length", format("%d", stream.size()));
    std::string strng = format("%d 0 obj\n", this->id) + content.toString();
    if (not stream.empty())
        strng += "stream\n" + stream + "\nendstream\n";
    return strng + "endobj\n";
}

void
PdfObj:: set(std::string key, PdfObj value)
{
    content.set(key, value.byref());
}
void
PdfObj:: set(std::string key, PdfDict value)
{
    content.set(key, value);
}
void
PdfObj:: set(std::string key, std::list<int> value)
{
    content.set(key, value);
}
void
PdfObj:: set(std::string key, int value)
{
    content.set(key, value);
}
void
PdfObj:: set(std::string key, std::string value)
{
    content.set(key, value);
}

std::string
PdfObj:: byref()
{
    return format("%d 0 R", this->id);
}

// ************************* PDF Dict Object ***************************

void
PdfDict:: set(std::string key, std::string val)
{
    dict[key] = val;
}
void
PdfDict:: set(std::string key, int val)
{
    dict[key] = format("%d",val);
}

void
PdfDict:: set(std::string key, PdfDict val)
{
    dict[key] = val.toString();
}

void
PdfDict:: set(std::string key, std::list<int> val)
{
    std::string strng;
    strng += "[";
    for (int i : val)
        strng += format("%d 0 R\n", i);
    strng += "]\n";
    dict[key] = strng;
}

std::string
PdfDict:: toString()
{
    std::string strng = "<<\n";
    for (auto &pair : dict)
        strng += format("/%s %s\n", pair.first.c_str(), pair.second.c_str());
    strng += ">>\n";
    return strng;
}


// read 4 bytes (for network byte order)
#define read32(a, arr) \
do {\
    char aa_=0, bb_=0, cc_=0, dd_=0; \
    aa_= arr[0]; bb_= arr[1]; cc_= arr[2]; dd_= arr[3]; \
    (a) = (aa_<<24) + (bb_<<16) + (cc_<<8) + (dd_); \
    data += 4; \
} while(0)

std::string getPngIdat(char *rawdata, int rawdata_size)
{
    // the png must be complete and valid png
    std::string idat;

    char *data = rawdata + 8; // 1st 8 byte is png signature
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
    std::ifstream inFile(filename);
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
    std::string matrix = format("%s %s cm\n", rot_str.c_str(), trans_str.c_str()); // translate and then rotate
    if (rot==90 or rot==270) {
        float tmp = w;
        w = h;
        h = tmp;
    }
    matrix += format("%.4f 0 0 %.4f 0 0 cm\n" ,w, h);      // finally scale image
    return matrix;
}

