
#include "checkARRAY.h"
#include <assert.h>
#include <fstream>
#include <filesystem>

using namespace std;
typedef std::filesystem::path  fs;



check* check::THIS = nullptr;



bool check::valid(string& s)
{
   bool validARRAY = false;

   const char*  pInput = s.c_str(); 

   char* pScanInput = (char*)pInput;
   
   if (pScanInput == nullptr || *pScanInput == '\0')
   {
       return validARRAY;
   }
      
   
   _ARRAY* pB = nullptr;
   while (pScanInput != nullptr && *pScanInput != '\0')
   {
       if (*pScanInput == '=')
       {
           validARRAY = true;
           while (*pScanInput != '\0' && *pScanInput != '{')
           {
               pScanInput++;
           };

           if (*pScanInput == '{')
           {
               validARRAY |= true;
               pB = (_ARRAY*)malloc(sizeof(_ARRAY));

               assert(pB != nullptr);

               char* pScanBal = (char*)pB->content;

               while (*pScanInput != '\0' && *pScanInput != '}')
               {
                   *pScanBal++ = *pScanInput++;
               };

               if (*pScanInput != '}')
               {
                   validARRAY = false;
                   return validARRAY;
               }

               *pScanBal++ = *pScanInput++;

               *pScanBal = '\0';

               ARRAY_infos.push_back(pB);
               return validARRAY;
           }
           else
           {
               validARRAY = false;
               return validARRAY;
           }
       }
       else
       {
           pScanInput++;
       }    
   }

   return validARRAY;
}

check::~check()
{
    for (auto& elem : ARRAY_infos)
    {
        delete elem;

        elem = nullptr;
    }
};

check* check::instance()
{
    if (THIS == nullptr)
    {
        THIS = new check();
    }


    return THIS;
};

bool check::checkTagCompliant(_ARRAY* bal)
{
    auto& tag_open = bal->content;

    char* pScanTagOpen = (char*)tag_open;

    bal->validARRAY = false;

    if (pScanTagOpen != nullptr &&  *pScanTagOpen == '{')
    {
        pScanTagOpen++;

        bal->validARRAY |= true;
    }

    while ( *pScanTagOpen != '\0')
    {
        if (*pScanTagOpen == '}')
        {
            bal->validARRAY &= true;
        }
        pScanTagOpen++;
    }

    return bal->validARRAY;
}

_statis check::statistics(vector <_ARRAY* >& ARRAY_infos)
{
    _statis st;

    return st;
}

bool check::Parse(string& input, _ARRAY*& bal)
{
    bool validARRAY = true;

    const char* pInput = input.c_str();

    char* pScanInput = (char*)pInput;

    if (pScanInput == nullptr || *pScanInput == '\0')
    {
        return false;
    }

    bal = (_ARRAY*)malloc(sizeof(_ARRAY));
    _ARRAY* pB = bal;
    assert(pB != nullptr);

    char* pScanBal = (char*)pB->type;

    while (pScanInput != nullptr && *pScanInput != ' ' && *pScanInput != '\0')
    {     
        while (*pScanInput != ' '  && *pScanInput != '\0')
        {
            *pScanBal++ = *pScanInput++;
        };
        char* pSnapshot = pScanInput;
        *pScanBal++ = *pScanInput++;
        

        int white = 0;
        while (*pScanInput != '[' && *pScanInput != '\0')
        {
            if (*pScanInput == ' ') white++;
            pScanInput++;
        };

        pScanInput = pSnapshot;
        if (*pScanInput != '\0'  && white >=2 )
        {
            while (white && *pScanInput != '\0')
            {
                *pScanBal++ = *pScanInput++;
                if (*pScanInput == ' ') white--;
            };

        }
    }
    
    *pScanBal = '\0';
   
    pScanBal = (char*)pB->name;
   
    while (*pScanInput != '\0' && *pScanInput != '=')
    {
        *pScanBal++ = *pScanInput++;
    };

    if (*pScanInput != '=')
    {
        return false;
    }
    
    *pScanBal = '\0';

    pScanBal = (char*)pB->content;
   
    while (*pScanInput != '\0')
    {
        if (*pScanInput == '{')
        {    
            while (*pScanInput != '\0' && *pScanInput != ';')
            {
                *pScanBal++ = *pScanInput++;
            };

            if (*pScanInput == ';')
            {
                *pScanBal++ = *pScanInput++;

                *pScanBal = '\0';

                ARRAY_infos.push_back(pB);

                return true;
            }
            else
            {
                return false;
            }
            
        }
        else
        {
            pScanInput++;
        }
    }
    return true;
}


void   check::Display(_ARRAY* bal, int num)
{
    assert(bal != nullptr);
    cout << "\n details[" << num <<"]:\n";
    cout << "\ttype    :\t" << bal->type << endl;
    cout << "\tname    :\t" << bal->name << endl;
    cout << "\tcontent :\t" << bal->content << endl;

}

vector<_line> check::Counter(string& input, _statis* st)
{
    st = (_statis*)malloc(sizeof(_statis));
    _statis* pST = st;
    assert(pST != nullptr);

    check* p = check::instance();
    assert(p != nullptr);

    ifstream fic(input.c_str(), ios::in); 
    long num = 0;

    vector<_line> lines;
    vector<_line> filteredLines;

    if (fic)  
    {
        string str;
        while (getline(fic, str) && num < NB_LINES)
        {
            cout << num ++ << "\t" << str << endl;
            _line line;
                
            line.content = str;
            line.num = num;
            line.count= 0;

            lines.push_back(line);
        }
        fic.close();

        auto iter = lines.begin();
        while (iter != lines.end())
        {
            _line l = p->CounterLine(iter , lines); // sometimes concatane line.content 
            
            if ( l.begin > 0 && l.end > 0)
            {
                filteredLines.push_back(l);
            }
            
            if (iter != lines.end()) iter++;
        }

        p->DisplayLines(filteredLines);
    
    }
    
    free(pST);

    return filteredLines;
}

_line check::CounterLine(vector<_line>::iterator&  iter, const vector<_line>& lines)
{
    _line  ref;
    _line& line = *iter;
    
    int pos = find(line.content,'[');
    
    ref.begin =  pos;

    ref.end = 0;

    pos = pos > 0 ? 0 : -1;

    switch (pos)
    {
        case -1: break;

        case  0: pos = line.end = find(line.content, ';'); 
                 ref.content = line.content;
                 while( iter!= lines.end() && pos == -1)
                 { 
                    iter++;

                    if (iter == lines.end())
                        return ref;

                    line = *iter;

                    pos = find(line.content,';');

                    pos = pos > 0 ? 0 : -1;
                 
                    switch (pos)
                    {
                        case -1: {

                            line.end = line.begin = -1;
                            ref.content+=line.content;
                            ref.end += line.count;
                            
                        }
                        break;

                        case  0: {
                            line.end = line.begin = -1;
                            ref.end += find(line.content, ';');
                            ref.content += line.content;
                            
                        }
                        break;
                    }
                 }
                 break;
    }

    return ref;
}

void check::DisplayLines(vector<_line>& lines)
{
    unsigned int num = 0;
    for ( auto& line : lines)
    {
        if (line.begin > 0 && line.end > 0)
        {
            cout << "\tARRAY>\t" <<num++ << "\t" << line.content.c_str() << endl << endl;
            
        }
    }
    
}

int check::find(string& content, char c)
{
    const char* input = content.c_str();
    
    char* pScanInput = (char*)input;

    while (*pScanInput != '\0' && *pScanInput != c) pScanInput++;

    if (*pScanInput == c)
    {
        return pScanInput - input;
    }

    return -1;
}

int check::dirExists(const char* path)
{
    struct stat info;

    if (stat(path, &info) != 0)
        return 0;
    else if (info.st_mode & S_IFDIR)
        return 1;
    else
        return 0;
}

void check::Generator(const fs&  dir, const ARRAY* a)
{
    
    assert(a != nullptr);

    std::string dir_str = dir.string();
    const char* dir_ptr = dir_str.c_str();
    
    assert(dir_ptr!=nullptr);

    if (!dirExists(dir_ptr))
    {
        std::filesystem::create_directory(dir_ptr);
    }

    char name[DIM];

    char* pSnapshot = (char*)name;
    char  file_content_h[1024];
    char  file_content_cpp[1024];

    string dthhmmss = string("////") + string(getTime()) + string("////\n\n");

    strcpy_s(file_content_h, dthhmmss.c_str());

    strcpy_s(file_content_cpp, dthhmmss.c_str());


    char* pScanName = (char*)a->name;
    while (*pScanName != '\0')
    {
        *pSnapshot++ = *pScanName++;
    }
    *pSnapshot = '\0';

    pScanName = (char*)a->name;
    while (*pScanName != '\0' && *pScanName != '[') pScanName++;
    
        
    if (*pScanName == '[')
    {
        *pScanName = '\0';
    }
    
    ///// GENERATE THE HEADER   ///////
    
    std::string path_h = dir_str + string("\\") + a->name + string(".h");

    if (std::filesystem::exists(path_h)) {
        
        return;
    }

    std::ofstream  fic_h(path_h);
    
    if (fic_h.is_open()) {
        
        fic_h << file_content_h;

        // delete keyword 'static'

        char* pScanType = (char*)a->type;
        while (*pScanType != '\0' && *pScanType != 's') ++pScanType;
        if (*pScanType == 's') *pScanType = ' ';
        while (*pScanType != '\0' && *pScanType != 't') ++pScanType;
        if (*pScanType == 't') *pScanType = ' ';
        while (*pScanType != '\0' && *pScanType != 'a') ++pScanType;
        if (*pScanType == 'a') *pScanType = ' ';
        while (*pScanType != '\0' && *pScanType != 't') ++pScanType;
        if (*pScanType == 't') *pScanType = ' ';
        while (*pScanType != '\0' && *pScanType != 'i') ++pScanType;
        if (*pScanType == 'i') *pScanType = ' ';
        while (*pScanType != '\0' && *pScanType != 'c') ++pScanType;
        if (*pScanType == 'c') *pScanType = ' ';

        fic_h << "extern " << a->type <<" " << name << ";";

        fic_h.close();

        cout << "File created: " << path_h << endl;
    }
    else {
         
        cerr << "Failed to create file: " << path_h << endl;
    }

    ///// GENERATE THE .CPP   ///////

    std::string path_cpp = dir_str + string("\\") + a->name + string(".cpp");

    if (std::filesystem::exists(path_cpp)) {

        return;
    }

    std::ofstream  fic_cpp(path_cpp);

    if (fic_cpp.is_open()) {

        fic_cpp << file_content_cpp;

        fs fname(path_h);
        fic_cpp << "#include " << "\\" << "common.h" << "\\" << endl;

        fic_cpp << "#include" <<"\""<< fname.filename().string().c_str() << "\"" << endl << endl;
        fic_cpp << a->type  << name << " = " << a->content;
        fic_cpp.close();

        cout << "File created: " << path_cpp << endl;
    }
    else {

        cerr << "Failed to create file: " << path_cpp << endl;
    }
}

char* check::getTime()
{
    
    time_t now = time(NULL);
    struct tm tm_now;
    localtime_s(&tm_now, &now);
    char* s_now = (char*) malloc(sizeof(char)*strlen("JJ/MM/AAAA HH:MM:SS"));
    
    strftime(s_now, strlen(s_now), "%d/%m/%Y %H:%M:%S", &tm_now);

    return  s_now;
}


void Parse(string& input, _ARRAY*& bal)
{
    check* p = check::instance();
    assert(p != nullptr);
    p->Parse(input, bal);
}

void Display(_ARRAY* bal, int num)
{
    check* p = check::instance();
    assert(p != nullptr);
    p->Display(bal, num);
}

string CheckARRAY(string& str) {

    cout << " **** CHECK ARRAY  ****" << endl;

    check* p = check::instance();
    assert(p != nullptr);

    bool status = p->valid(str);

    ostringstream os;

    os << "Status ARRAY ";

    os << (status == true ? "TRUE" : "FALSE") << endl;

    return os.str().c_str();
};

vector<_line> Counter(string& input)
{
    check* p = check::instance();
    assert(p != nullptr);
    _statis* pST = nullptr;
    return p->Counter(input, pST);
}


void Generator(ARRAY* a)
{
    assert(a != nullptr);

    check* p = check::instance();
    assert(p != nullptr);

    p->Generator(".\\GEN-YOLOV2",a);
}


bool Splitter(fs dir, ifstream& fic_r, string& separator)
{
    std::string dir_str = dir.string();
    const char* dir_ptr = dir_str.c_str();

    assert(dir_ptr != nullptr);

    check* p = check::instance();
    assert(p != nullptr);

    if (!p->dirExists(dir_ptr))
    {
        std::filesystem::create_directory(dir_ptr);
    }

    int num = 0;
    while (p->Splitter(dir, fic_r, ++num, separator));
     
    return true;
}

bool check::Splitter(fs dir, ifstream& fic_r, int num, string& separator)
{
    ostringstream os;

    os << dir.string() << "\\ty-v2-7." << num << ".cpp";

    fs path_s(os.str());
  
    if (std::filesystem::exists(path_s)) {

        return true;
    }

    ofstream fic_w(path_s);

    if (fic_w.is_open())
    {
        
            string str;
            int lignes = 0;
            while (getline(fic_r, str) && str.length() > 0 && ++lignes < NB_LINES)
            {
                fic_w << str << endl;
            }

            char* pScanStr = (char*)str.c_str();
            while (*pScanStr != '\0' && *pScanStr != ';')
            {
                pScanStr++;
            }
                
            if (*(pScanStr-1) == '}' && *pScanStr == ';')
            {
               fic_w.close();
               cout << "File created: " << path_s << endl;
               return true;
            }

            while (getline(fic_r, str) && str.length() > 0)
            {
                fic_w << str << endl;
                pScanStr = (char*)str.c_str();
                while (*pScanStr != '\0' && *pScanStr != ';') pScanStr++;
                if (*(pScanStr - 1) == '}' && *pScanStr == ';')
                {
                    fic_w.close();
                    cout << "File created: " << path_s << endl;
                    return true;
                }
            } 

            fic_w.close();
            cout << "File created: " << path_s << endl;
            return true;
            
    }
    else
    {
        cerr << "Failed to create file: " << path_s << endl;
        return false;
    }

    return true;
}




