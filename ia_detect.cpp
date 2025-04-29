#include <stdint.h>
#include "../include/parser.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string_view>
#include <vector>
#include <time.h>

#define STB_IMAGE_IMPLEMENTATION
#include "../externals/stb_image.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "../externals/stb_image_resize.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../externals/stb_image_write.h"

#ifdef _MSC_VER
#include <windows.h>
#define int64 __int64
#pragma warning(disable:4127)

#define get_milliseconds GetTickCount

#else

#define int64 long long

typedef unsigned int U32;
typedef unsigned long long U64;


static int get_milliseconds()
{
  struct timespec ts;
  clock_gettime( CLOCK_MONOTONIC, &ts );
  return (U32) ( ( ((U64)(U32)ts.tv_sec) * 1000LL ) + (U64)(((U32)ts.tv_nsec+500000)/1000000) );
}

#endif

const int IMG_ROW_COUNT = 416;
const int IMG_COL_COUNT = 416;
const int IMG_CHANNEL_COUNT = 3;
const int GRID_ROW_COUNT = 13;
const int GRID_COL_COUNT = 13;
const int GRID_CHANNEL_COUNT = 125;

namespace program_options {

std::string_view get_option(
    const std::vector<std::string_view>& args, 
    const std::string_view& option_name);

bool has_option(
    const std::vector<std::string_view>& args, 
    const std::string_view& option_name);
	

};  // namespace program_options


void entry(const float tensor_image[1][3][416][416], float tensor_grid[1][125][13][13]);
int run_yolo(string_view& srcDir, string_view& dstDir, string_view& imageName);

int main(int argc, char* argv[])
{

	if (argc > 7) {
	    cout << "usage :" << argv[0] << " -s <src dir>  -s <dest dir> -i <imageName>\n";
	    throw std::runtime_error("too many input parameters!");
		return 0;
	};
	
	if (argc < 7) {
	    cout << "usage :" << argv[0] << " -s <src dir>  -s <dest dir> -i <imageName>\n";
	    throw std::runtime_error("too few input parameters!");
		return 0;
	};
	
	const std::vector<std::string_view> args(argv, argv + argc);
	cout << argv[0] << ": list of arguments\n";
	for (auto it = args.begin(), end = args.end(); it != end; ++it) {
        cout <<"\t" <<(string)*it  ;    
    }

	cout <<"\n";
	
	const bool  hasSrc = program_options::has_option(args, "-s");
	const bool  hasDst = program_options::has_option(args, "-d");
	const bool  hasImg = program_options::has_option(args, "-i");

	if (hasSrc && hasDst && hasImg)
	{
		std::string_view Src = program_options::get_option(args, "-s");
		std::string_view Dst = program_options::get_option(args, "-d");
		std::string_view Img = program_options::get_option(args, "-i");
		 
		run_yolo(Src,Dst,Img); 

	} else
	{	
		cout << "usage :" << argv[0] << " -s <src dir>  -s <dest dir> -i <imageName>\n";
	};

	return 0;
};

std::string_view program_options::get_option(
    const std::vector<std::string_view>& args,
    const std::string_view& option_name) {
	
    for (auto it = args.begin(), end = args.end(); it != end; ++it) {
        if (*it == option_name)
            if (it + 1 != end)
                return *(it + 1);
    }

    return "";
};

bool program_options::has_option(
    const std::vector<std::string_view>& args,
    const std::string_view& option_name) {
    for (auto it = args.begin(), end = args.end(); it != end; ++it) {
        if (*it == option_name)
            return true;
    }

    return false;
};


int run_yolo(string_view& srcDir, string_view& dstDir, string_view& imageName) {
	
	
	string inputImageLocation=(string)srcDir;
	string outputImageLocation=(string)dstDir;
	string img = inputImageLocation;
	img +=(string)imageName;

	int x = 0, y = 0;
	int width = 0, height = 0, channels = 0;
	float tensor_image[1][3][416][416];
	float tensor_grid[1][125][13][13];
    
	char name_resize_img[100];
	char name_gray_img_png[100];

	
	/*
	 float X = (mappedBoundingBox.getX() - mappedBoundingBox.getWidth() / 2);
                float Y = (mappedBoundingBox.getY() - mappedBoundingBox.getHeight() / 2);
                float Width = mappedBoundingBox.getWidth();
                float Height = mappedBoundingBox.getHeight();

                BoundingBoxDimensions* Dimensions = new BoundingBoxDimensions(X,Y,Width,Height);
                
                float Confidence = topScore;
                string Label = labels[topResultIndex];
                cout << "Label :" << Label << "Confidence Score :" << Confidence << endl;
                Color boxColor = classColors[topResultIndex];

                boxes->push_back(YoloBoundingBox(Dimensions,Confidence,Label,boxColor));
	*/ 

	float X = 0.0;
	float Y = 0.0;
	float Width = 416;
	float Height = 416;
	float Confidence = 0.0;

	string Label = labels[0];
    Color boxColor = classColors[0];

	BoundingBoxDimensions* Dimensions = new BoundingBoxDimensions(X,Y,Width,Height);
	YoloBoundingBox* parser = new YoloBoundingBox(Dimensions,Confidence,Label,boxColor); 
	assert(parser != nullptr);

	try
	{
		
		uint8_t*   data = stbi_load(img.c_str(), &width, &height, &channels, 0);
		
		auto *re_data = (unsigned char*)malloc(IMG_ROW_COUNT*IMG_COL_COUNT*channels);
		assert(re_data != nullptr);

		size_t img_size = width * height * channels;
      	int gray_channels = channels == 4 ? 2 : 1;
     	size_t gray_img_size = width * height * gray_channels;
 
     	unsigned char *gray_img = (unsigned char *)malloc(gray_img_size);
		assert(gray_img != nullptr);
     	

		if (data != nullptr) {

			cout << "\nimage :\t" << img.c_str() << " Loaded successfully";
			cout << "\nwidth :\t" << width;
			cout << "\nheight :\t" << height;
			cout << "\nchannels :\t" << channels;		

			stbir_resize(data,width, height,0,re_data,IMG_ROW_COUNT,IMG_COL_COUNT,0,STBIR_TYPE_UINT8,channels,STBIR_ALPHA_CHANNEL_NONE,0,
				  STBIR_EDGE_CLAMP,STBIR_EDGE_CLAMP,
				  STBIR_FILTER_BOX,STBIR_FILTER_BOX,
				  STBIR_COLORSPACE_SRGB,nullptr
			);

			

			img = imageName;
			sprintf(name_resize_img,"%s%s_%dx%d.png",outputImageLocation.c_str(),img.c_str(),IMG_ROW_COUNT,IMG_COL_COUNT);
			cout << "\nresize image :\t" << name_resize_img << " \n";
			stbi_write_png(name_resize_img,IMG_ROW_COUNT, IMG_COL_COUNT,channels,(const void*)&re_data[0],IMG_ROW_COUNT * channels);
			
		}
		else
		{
			cout << "\nimage :\t" << img.c_str() << " Failed loaded \n";
			return 0;
		}
		
		// create the gray image 
		for(unsigned char *p = data, *pg = gray_img; p != data + img_size; p += channels, pg += gray_channels) {
			*pg = (uint8_t)((*p + *(p + 1) + *(p + 2))/3.0);
			if(channels == 4) {
				*(pg + 1) = *(p + 3);
			}
		}
		
		stbi_image_free(data);

		
		img = imageName;
		sprintf(name_gray_img_png,"%s%s_gray_%dx%d.png",outputImageLocation.c_str(),img.c_str(),width,height);
		cout << "\ngray image :\t" << name_gray_img_png;
		stbi_write_png(name_gray_img_png, width, height, channels,gray_img, width * channels);
		
		/*char name_gray_img_jpg[100];
		sprintf(name_gray_img_jpg,"%s//%s_gray_%dx%d.jpg",outputImageLocation.c_str(),img.c_str(),width,height);
		cout << "\njpg image :\t" << name_gray_img_png;
		stbi_write_jpg(name_gray_img_jpg, width, height, channels, gray_img, 100);*/

		
		uint8_t* pScanRgbImage = re_data;

		// populate the input array with image data
		for (int i = 0; i < IMG_ROW_COUNT * IMG_COL_COUNT*channels; ++i) {
			
			if ( *pScanRgbImage != 0)
			{
				uint8_t red   = *pScanRgbImage++;
				uint8_t green = *pScanRgbImage++;
				uint8_t blue  = *pScanRgbImage++;
				
				x = (x == (IMG_COL_COUNT-1) ? 0 : x );

				tensor_image[0][0][x][y] = (float)red;
				tensor_image[0][1][x][y] = (float)green;
				tensor_image[0][2][x][y] = (float)blue;
					
				x++;
				
				y = (x == (IMG_COL_COUNT-1) ? y++ : y );
				y = (y == (IMG_ROW_COUNT-1) ? 0 : y );

			}
			else
			{
				x = (x == (IMG_COL_COUNT-1) ? 0 : x );

				tensor_image[0][0][x][y] = 0.0;
				//tensor_image[0][1][x][y] = 0.0;  /// ?
				//tensor_image[0][2][x][y] = 0.0;  /// ?
					
				x++;
				
				y = (x == (IMG_COL_COUNT-1) ? y++ : y );
				y = (y == (IMG_ROW_COUNT-1) ? 0 : y );
			}
		
		}
			
		stbi_image_free(re_data);    
		stbi_image_free(gray_img);    
		cout << "\nStart inference " << parser->getTime() << endl;
		entry(tensor_image, tensor_grid); // INFERENCE 
		cout << "\nEnd inference " << parser->getTime() << endl;
	}
	catch(...)
    {
        std::exception_ptr p = std::current_exception();
        std::clog <<(p ? p.__cxa_exception_type()->name() : "null") << std::endl;
    }
	
	try
	{
		// populate 1xD array from the tensor grid  ( a 3xD array)

		float* probabilities = (float*)malloc(GRID_CHANNEL_COUNT*GRID_ROW_COUNT * GRID_COL_COUNT*sizeof(float)); 
		assert(probabilities != nullptr);

		x = 0; y = 0;

		int channel = 0;

		for (int i = 0; i < GRID_CHANNEL_COUNT*GRID_ROW_COUNT * GRID_COL_COUNT; ++i) {
			
			// populate the 1xD array with 125 data elements 
			probabilities[i] = tensor_grid[0][channel][x][y];

			// go to the next cell on the same row  every 125 data element paste 	
			x = ((i > 0) && !(i % (GRID_CHANNEL_COUNT-1))) ? x++ : x ;
			// go to the next row of the grid every 13 cell  paste 	
			y = (x == (GRID_COL_COUNT-1) ? y++ : y );
			// go to the first cell on the next row  every 13 cell  paste 		
			x = (x == (GRID_COL_COUNT-1) ? 0 : x );
			// go to the first data element of the next cell every cell  paste 		
			channel = (channel == (GRID_CHANNEL_COUNT-1) ? 0 : channel++);
		}
		
		// post process model output ( tensor grid )
		

		vector<YoloBoundingBox>* sortedBox = parser->ParseOutputs(probabilities,.1F);
		assert(sortedBox != nullptr);
		cout << "SortedBox : " << sortedBox->size() << endl;
		assert(sortedBox->size() !=0 );

		vector<YoloBoundingBox>* filteredBox = parser->FilterBoundingBoxes(sortedBox, 5, .1F);
		assert(filteredBox != nullptr);
		cout << "filteredBox : " << filteredBox->size() << endl;
		assert(sortedBox->size() !=0 );

		free(probabilities);

		// Draw bounding boxes for detected objects in each of the images
		memset(name_resize_img,0,100);
		sprintf(name_resize_img,"%s_%dx%d.png",img.c_str(),IMG_ROW_COUNT,IMG_COL_COUNT);
		string sImgRe(name_resize_img);
		cout << "input image to process drawing box :" << name_resize_img << endl;
		parser->DrawBoundingBox(inputImageLocation, outputImageLocation,sImgRe,filteredBox, false);

		// clean
		//delete sortedBox;
		//delete parser;
		//delete filteredBox;
			
	}
	catch(...)
    {
        std::exception_ptr p = std::current_exception();
        std::clog <<(p ? p.__cxa_exception_type()->name() : "null") << std::endl;
    }


    return 0;
};
