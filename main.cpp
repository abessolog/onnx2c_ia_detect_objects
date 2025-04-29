
#include <iostream>
#include "checkARRAY.h"
#include <assert.h>
#include <filesystem>

typedef std::filesystem::path fs;
using   std::filesystem::directory_iterator;
using namespace std;

string toParse[] = {
		"ty-v2-7.3.cpp",
		"ty-v2-7.4.cpp",
		"ty-v2-7.5.cpp",
		"ty-v2-7.6.cpp",
		"ty-v2-7.7.cpp",
		"ty-v2-7.8.cpp",
		"ty-v2-7.9.cpp"};

string toForgets[] = {
	"ty-v2-7.10.cpp",
	"ty-v2-7.11.cpp",
	"ty-v2-7.12.cpp",
	"ty-v2-7.13.cpp",
	"ty-v2-7.14.cpp",
	"ty-v2-7.15.cpp",
	"ty-v2-7.16.cpp",
	"ty-v2-7.17.cpp",
	"ty-v2-7.18.cpp",
	"ty-v2-7.19.cpp",
	"ty-v2-7.20.cpp",
	"ty-v2-7.21.cpp",
	"ty-v2-7.22.cpp",
	"ty-v2-7.23.cpp",
	"ty-v2-7.24.cpp",
	"ty-v2-7.25.cpp",
	"ty-v2-7.26.cpp",
	"ty-v2-7.27.cpp",
	"ty-v2-7.28.cpp",
	"ty-v2-7.29.cpp",
	"ty-v2-7.30.cpp",
	"ty-v2-7.31.cpp",
	"ty-v2-7.32.cpp"
};


int main(void) {

	//cout << " .h, .cpp GENERATE from tinyyolov2-7.c provided by onnx2c tool \n ";
	cout << "GENERATE makefile \n ";

	check *p = check::instance();
	assert( p != nullptr);
	cout << "Start time:\t" << p->getTime() << endl;

	/// Source of input files to Parse
	fs src(".\\INPUTS\\src");
	fs dst(".\\GEN-YOLOV2");

	//cout << "List of files to PARSE:\t" << endl;
	cout << "List of files:\t" << endl;
	vector<fs> files;

	for (auto& file : directory_iterator(dst))
	{
		cout << file.path() << endl;
		files.push_back(file.path());
	}

	std::ofstream  makefile("makefile.gen");

	if (makefile.is_open()) {

		makefile << "////" << check::instance()->getTime() << "////" <<endl;
		makefile << "obj+=" <<endl;

	}


	for (auto& file : files)
	{
		fs ext{".o"};

		fs obj = file.replace_extension(ext);
		makefile << obj.filename() << endl;
		cout << obj.filename() << endl;
		
	}
	
	makefile.close();

	/*for (const auto& file : toParse)
	{
		cout << file << endl;
		files.push_back(file);
	}

		
	/// Collect arrays as a pattern and save into a string
	for (const auto& file : files)
	{ 
		string input = string(".\\INPUTS\\") + file;
		vector<_line> lines = Counter( input);

		cout << "From file :" << input << endl;
		cout << "Collect arrays as a pattern and save into a string" << endl;
		cout << "Collect Start time:\t" << p->getTime() << endl;
		
		vector<ARRAY*> arrays;
		int num = 0;
		for (auto& line : lines)
		{
			ARRAY* bal;

			Parse(line.content, bal);
			assert(bal!=nullptr);
			Display(bal, num++);
			arrays.push_back(bal);
		}
		cout << "Collect End time:\t" << p->getTime() << endl;

		//// GENERATE HEADER and CODE for each array
		cout << "From file :" << input << endl;
		cout << "GENERATE HEADER and CODE for each array" << endl;
		cout << "GNERATE Start time:\t" << p->getTime() << endl;
		for (auto& a : arrays)
		{
			Generator(a);
		
			free(a);
		}
		cout << "GNERATE End time:\t" << p->getTime() << endl;

	}*/
	cout << "End time:\t" << p->getTime() << endl;
	
	return 0;
};
