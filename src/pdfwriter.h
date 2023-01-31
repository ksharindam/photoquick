#pragma once
#include <string>
#include <iostream>
#include <list>
#include <map>
#include <sstream>
#include <fstream>
#include <clocale>

#ifndef __PHOTOQUICK_PDFWRITER
#define __PHOTOQUICK_PDFWRITER

/* HOW TO USE
PdfDocument doc;
PdfPage *page = doc.newPage(595, 842);
PdfObject *img = doc.addImage(img_buff, buff_size, 480, 640, PDF_IMG_JPEG);
page->drawImage(img, 0,0,595, 842);
doc.save(filename);
*/

typedef enum {
    PDF_IMG_JPEG,
    PDF_IMG_PNG
} PdfImageFormat;

typedef enum {
    STROKE,
    FILL,
    FILL_N_STROKE
} PaintMode;

/* PDF has 8 basic types of direct objects.
   boolean, number, string, name, array, dictionary, stream and null object.
   In this pdf writer - bool, number, string, name and null objects are represented as
   string object for convenience.
*/

typedef enum {
    PDF_OBJ_STRING,
    PDF_OBJ_ARRAY,
    PDF_OBJ_DICT,
    PDF_OBJ_STREAM
} ObjectType;


class PdfObject
{
public:
    ObjectType type;
    std::string string;
    std::list<PdfObject*> array;
    std::map<std::string, PdfObject*> dict;
    std::string stream;
    // object of any other type can also act as indirect obj.
    // only during writing to file, we consider whether it is indirect, and use the obj_no.
    // obj_no > 0 means it is indirect obj and has been added to obj_table.
    int obj_no;
    int offset;

    PdfObject(ObjectType type);
    // for PDF_OBJ_ARRAY type
    void append(PdfObject *item);
    // for PDF_OBJ_DICT and PDF_OBJ_STREAM type
    void add(std::string key, PdfObject *val);
    void add(std::string key, std::string val);
    // other
    bool isIndirect();// check whether it was added to obj_table
    // if as_direct_obj is true, the obj is considered as direct obj,
    // and does not return as reference.
    std::string toString(bool as_direct_obj=true);
    // free memory recursively
    ~PdfObject();
};

class PdfPage : public PdfObject
{
public:
    PdfObject *x_objects;// a dict of images XObject
    PdfObject *contents;

    PdfPage(int w, int h, PdfObject *parent);
    void setLineColor(int r, int g, int b);
    void setFillColor(int r, int g, int b);
    void drawImage(PdfObject *img, float x, float y, float w, float h, int rotation=0);
    void drawRect(float x, float y, float w, float h, float line_width, PaintMode mode);
};


class PdfDocument
{
public:
    std::string producer;
    PdfObject *info;
    PdfObject *catalog;// Root
    PdfObject *pages_parent;// Pages dictionary
    PdfObject *pages;// Pdf Array of PdfPage
    std::list<PdfObject*> obj_table;

    PdfDocument();
    ~PdfDocument();
    PdfPage*   newPage(int w, int h);
    void       addObject(PdfObject *obj);
    PdfObject* addImage(const char *buf, int size, int w, int h, PdfImageFormat format);
    void       save(std::string filename);
};


// sprintf like string formatting that returns std::string
template<typename... Args>
std::string format(const char* fmt, Args... args);


std::string readFile(std::string filename);

#endif /* __PHOTOQUICK_PDFWRITER */
