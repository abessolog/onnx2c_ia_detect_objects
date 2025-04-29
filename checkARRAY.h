#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <string.h>  // <cstring> en C++
#include <filesystem>
#include <fstream>

using namespace std;
typedef std::filesystem::path  fs;

#define DIM  75
#define DIM_CONTENT 3000000


/*
*
*/
typedef struct  ARRAY {

    char  type[DIM];
    char  name[DIM];
    char  content[DIM_CONTENT];
    bool  validARRAY;

} _ARRAY;

typedef struct  statis {

    string pattern;
    uint8_t count;

} _statis;

typedef struct  line {

    string   content;
    unsigned long  num; // line number
    size_t  begin;      // where array starts
    size_t  end;        // where array ends
    uint8_t count;

} _line;


string  CheckARRAY(string& str);

void    Parse(string& input, _ARRAY*& bal);

string  CheckARRAY(string& str);

void Display(_ARRAY* bal, int num);

vector<_line> Counter(string& input);

void Generator(ARRAY* a);

bool Splitter(fs dir,ifstream& fic_r, string& separator);

class check
{
private:

    vector <_ARRAY* > ARRAY_infos;
    vector <_statis* > FILE_infos;

    static check* THIS;
    const unsigned long NB_LINES = 100000; // CRASH LINES =7972438 // TOTAL LINES = 9129095

public:

    check() = default;

    bool valid(string& s);

    virtual ~check();

    static check* instance();

    bool  checkTagCompliant(_ARRAY* bal);

    void   Display(_ARRAY* bal, int num);

    bool Parse(string& input, _ARRAY*& bal);

    _statis statistics(vector <_ARRAY* >& ARRAY_infos );

    vector<_line> Counter(string& input, _statis* st);

    _line  CounterLine(vector<_line>::iterator& iter, const vector<_line>& lines);

    void DisplayLines(vector<_line>& lines);

    int find(string& content, char c);

    void Generator(const fs&  dir, const ARRAY* a);

    int  dirExists(const char* path);

    char* getTime();

    bool Splitter(fs dir, ifstream& fic_r, int num, string& separator);
};
