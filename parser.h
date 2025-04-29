#ifndef __PARSER_H__
#define __PARSER_H__

#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <cmath> 
#include <algorithm>
#include <cstdint>
#include <sstream>      // std::stringstream
#include <cassert>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <filesystem>
#include <fstream>

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>


namespace fs = std::filesystem;

using namespace std;
using namespace cv;

const int ROW_COUNT = 13;
const int COL_COUNT = 13;
const int CHANNEL_COUNT = 125;
const int BOXES_PER_CELL = 5;
const int BOX_INFO_FEATURE_COUNT = 5;
const int CLASS_COUNT = 20;
const float CELL_WIDTH = 32;
const float CELL_HEIGHT = 32;
const int WIDTH = 800;
const int HEIGHT = 640;




class DimensionsBase
{
    protected:
        int X;
        int Y;
        int Height;
        int Width;

    public:
        DimensionsBase()
        {
            X=0;
            Y=0;
            Width=416;
            Height=416;
        };
        DimensionsBase(int X, int Y, int w, int h)
        {
            this->X=X;
            this->Y=Y;
            this->Width=w;
            this->Height=h;
        }

        virtual ~DimensionsBase() = default;

        int   getX(void) {return X;} 
        void  setX(int X) { this->X = X;}
        int   getY(void) {return Y;} 
        void  setY(int Y) { this->Y = Y;} 

        int   getHeight(void) {return Height;}  
        void  setHeight(int h) { this->Height=h;}

        int   getWidth(void) { return Width;}
        void  setWidth(int w) { this->Width = w;}
};



class BoundingBoxDimensions : public DimensionsBase
{
    public:
        BoundingBoxDimensions();
        virtual ~BoundingBoxDimensions() = default;
        BoundingBoxDimensions(int X, int Y, int w, int h):DimensionsBase(X,Y,w,h) {}
};


class Color 
{
    protected:

        uint8_t R;
        uint8_t G;
        uint8_t B;

    public:

        Color()=default;
        virtual ~Color()=default;
        
        Color(uint8_t R, uint8_t G, uint8_t B) {this->R = R; this->G = G; this->B = B;}
        
        uint8_t getR() { return R;}
        uint8_t getG() { return G;}
        uint8_t getB() { return B;}

        void setR(uint8_t R) { this->R=R;}
        void setG(uint8_t G) { this->G=G;}
        void setB(uint8_t B) { this->B=B;}

        Color&  operator=(const Color& o)
        {
            this->R = o.R;
            this->G = o.G;
            this->B = o.B;
            
            return *this;
        }

        Color&  operator=(Color& o)
        {
            this->R = o.R;
            this->G = o.G;
            this->B = o.B;
            
            return *this;
        }
};

extern float anchors[10];
extern Color classColors[21];
extern string labels[];

class CellDimensions : public DimensionsBase 
{ 
    public:
    CellDimensions():DimensionsBase (0, 0,416,416) {}
    virtual ~CellDimensions()=default;
    CellDimensions(int X, int Y, int Width, int Height):DimensionsBase (X, Y, Width, Height) {}
};

class RectangleF : public DimensionsBase
{
    protected:

    int Left;
    int Right;
    int Bottom;
    int Top;

    public:
        RectangleF() = delete;
        virtual ~RectangleF() = default;
        RectangleF(float X, float Y, float w, float h):DimensionsBase(X,Y,w,h)
        {
            this->X=X;
            this->Y=Y;
            this->Width=w;
            this->Height=h;

            // the origin is in the middle of the cell

            this->Left   = this->Width/2 ;
            this->Right  = -this->Width/2; 
            this->Top    = this->Height/2;
            this->Bottom = -this->Height/2;
        };

        int getLeft()   {return this->Left;}
        int getRight()  {return this->Right;}
        int getTop()    {return this->Top;}
        int getBottom() {return this->Bottom;}

};

struct TopResult
{
    float  score;
    uint8_t index;
};

class YoloBoundingBox
{
    protected:
        BoundingBoxDimensions* bbd = nullptr;
        string Label;
        float Confidence;
        Color boxColor;

        //int channelStride = ROW_COUNT * COL_COUNT; // 
        int channelStride = 1; // 
        
        bool active = true;

    protected:

        float Sigmoid(float value)
        {
            float k = expf(value);
            return k / (1.0f + k);
        };

        float* Softmax(float values[])
        {
            float maxVal = *max_element(values, values + sizeof(values)/sizeof(values[0]));
            
            for (int i = 0 ; i < sizeof(values)/sizeof(values[0]); i++)
            {
                values[i] = expf(values[i] - maxVal);
            }

            float sumExp = 0.0;
            for (int i = 0 ; i < sizeof(values)/sizeof(values[0]); i++)
            {
                sumExp += values[i];
            }
            
            for (int i = 0 ; i < sizeof(values)/sizeof(values[0]); i++)
            {
                values[i] = values[i]/sumExp;
            }

            return &values[0];
        };

        float GetConfidence(float modelOutput[], int x, int y, int channel)
        {
            return Sigmoid(modelOutput[GetOffset(x, y, channel + 4)]);
        };
 
        TopResult GetTopResult(float predictedClasses[])
        {
            TopResult   top;

            top.score =*max_element(predictedClasses, predictedClasses + sizeof(predictedClasses)/sizeof(predictedClasses[0]));
            top.index = 0;

            for(int i = 0 ; i < sizeof(predictedClasses)/sizeof(predictedClasses[0]); i++)
            {
                if ( top.score == predictedClasses[i] )
                {
                    top.index = i;
                    break;
                }
            }
            return top;
        };

        float IntersectionOverUnion(RectangleF& boundingBoxA, RectangleF& boundingBoxB);
        
        CellDimensions* MapBoundingBoxToCell(int x, int y, int box, BoundingBoxDimensions& boxDimensions)
        {
            float X = ((float)x + Sigmoid(boxDimensions.getX())) * CELL_WIDTH;
            float Y = ((float)y + Sigmoid(boxDimensions.getY())) * CELL_HEIGHT;
            float Width = expf(boxDimensions.getWidth()) * CELL_WIDTH * anchors[box * 2];
            float Height = expf(boxDimensions.getHeight()) * CELL_HEIGHT * anchors[box * 2 + 1];

            return new CellDimensions( X, Y, Width, Height);   
        }

    public :
        YoloBoundingBox()=delete;
        virtual ~YoloBoundingBox() = default;
        YoloBoundingBox(DimensionsBase* Dimensions , float Confidence, string Label,Color  BoxColor)
        {
            assert(Dimensions != nullptr);

            bbd = new BoundingBoxDimensions(Dimensions->getX(),Dimensions->getY(),Dimensions->getWidth(),Dimensions->getHeight());
            assert(bbd != nullptr);
            this->Label = Label;
            this->Confidence = Confidence;
            this->boxColor = BoxColor;
        };   

        BoundingBoxDimensions& getDimensions() {return *bbd;};
        void setDimensions(BoundingBoxDimensions* bbd) { this->bbd = bbd;};

        string getLabel(void) { return Label;};
        void   setLabel(string s) {this->Label=s;};

        float getConfidence(void) { return Confidence;};
        void  setConfidence(float c) { this->Confidence = c;};
        
        RectangleF* getRect()
        {
            assert( bbd != nullptr);
            return new RectangleF(bbd->getX(), bbd->getY(), bbd->getWidth(), bbd->getHeight()); 
        };


        Color getBoxColor() { return boxColor;};
        void setBoxColor(Color& bc) { this->boxColor = bc;};

        int GetOffset(int x, int y, int channel)
        {
            // YOLO outputs a tensor that has a shape of 125x13x13, which 
            // WinML flattens into a 1D array.  To access a specific channel 
            // for a given (x,y) cell position, we need to calculate an offset
            // into the array
            return (channel * this->channelStride) + (y * COL_COUNT) + x;
        };

        BoundingBoxDimensions* ExtractBoundingBoxDimensions(float modelOutput[], int x, int y, int channel)
        {
            
            return new BoundingBoxDimensions
            (
                /*X = */modelOutput[GetOffset(x, y, channel)],
                /*Y = */modelOutput[GetOffset(x, y, channel + 1)],
                /*Width =*/ modelOutput[GetOffset(x, y, channel + 2)],
                /*Height =*/ modelOutput[GetOffset(x, y, channel + 3)]
            );
        };

        float* ExtractClasses(float modelOutput[], int x, int y, int channel)
        {
            float* predictedClasses = new float[CLASS_COUNT];
            int predictedClassOffset = channel + BOX_INFO_FEATURE_COUNT;
            for (int predictedClass = 0; predictedClass < CLASS_COUNT; predictedClass++)
            {
                predictedClasses[predictedClass] = modelOutput[GetOffset(x, y, predictedClass + predictedClassOffset)];
            }
            return Softmax(predictedClasses);
        };
        
        vector<YoloBoundingBox>* ParseOutputs(float yoloModelOutputs[], float threshold = .1F);
        
        static bool DescendingSort(YoloBoundingBox& boxA, YoloBoundingBox& boxB) 
        { 
            return  boxA.getConfidence() > boxB.getConfidence() ; 
        }

        vector<YoloBoundingBox>* OrderBoundingBoxes(vector<YoloBoundingBox>* boxes)
        {
            assert(boxes != nullptr);

            std::sort(boxes->begin(), boxes->end(), DescendingSort); 
            
            return boxes;
        };

        vector<YoloBoundingBox>* FilterBoundingBoxes(vector<YoloBoundingBox>* boxes, int limit, float threshold=0.1F);
        
        void setActive() { active = true;};
        bool isActive()  { return active;};
        void inActive()  { active = false;};

        RectangleF& Rect () { return *getRect();};
        
        /*YoloBoundingBox&  operator=(const YoloBoundingBox& o)
        {
            this->bbd = o.bbd;
            this->Label = o.Label;
            this->Confidence = o.Confidence;
            this->boxColor = o.boxColor;
            this->active = o.active;

            return *this;
        };

        YoloBoundingBox&  operator=(YoloBoundingBox& o)
        {
            this->bbd = o.bbd;
            this->Label = o.Label;
            this->Confidence = o.Confidence;
            this->boxColor = o.boxColor;
            this->active = o.active;

            return *this;
        };*/

        void DrawBoundingBox(string& inputImageLocation, string& outputImageLocation, string& imageName, vector<YoloBoundingBox>* filteredBoundingBoxes, bool show = false);
        
        
        int dirExists(const char *path)
        {
            struct stat info;

            if(stat( path, &info ) != 0)
                return 0;
            else if(info.st_mode & S_IFDIR)
                return 1;
            else
                return 0;
        };

        
        char* getTime()
        {
            
            time_t rawtime;
            struct tm * timeinfo;
            char buffer [80];
          
            time (&rawtime);
            timeinfo = localtime (&rawtime);
          
            strftime (buffer,80,"Now it's %I:%M%p.",timeinfo);
            puts (buffer);
          
            return  buffer;
        }
};
#endif /*  __PARSER_H__ */