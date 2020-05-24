#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <list>
#include <map>

class PdfDict
{
public:
    PdfDict(){};
    void set(std::string key, PdfDict val);
    void set(std::string key, std::list<int> val);
    void set(std::string key, std::string val);
    void set(std::string key, int val);
    std::string toString();
    std::map<std::string, std::string> dict;
};

class PdfObj
{
public:
    PdfObj();
    std::string toString(std::string stream="");
    void set(std::string key, PdfDict val);
    void set(std::string key, PdfObj val);
    void set(std::string key, std::list<int> val);
    void set(std::string key, int val);
    void set(std::string key, std::string val);
    std::string byref();
    PdfDict content;
    int id=0;
};


class PdfWriter
{
public:
    PdfWriter();
    void begin(std::string filename);
    PdfObj createPage(int w=595, int h=842, std::string Contents="[]",
                        std::string Resources="<< /ProcSet [/PDF] >>");
    void addPage(PdfObj &page);
    int  addObj(PdfObj &obj, std::string Stream="", int id=0);
    void finish();
    // member variables
    std::string version;
    std::string producer;
    std::string creation_date;
    std::string header;
    std::list<int> pages;
    int offset;
    std::list<int> obj_offsets;
    std::ofstream stream;
};

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

std::string getPngIdat(char *rawdata, int rawdata_size);

std::string readFile(std::string filename);

std::string imgMatrix(float x, float y, float w, float h, int rotation);
