#include  <parser.h>

float anchors[10] = { 1.08F, 1.19F, 3.42F, 4.41F, 6.63F, 11.38F, 9.42F, 5.11F, 16.62F, 10.52F};

Color classColors[21] = 
{
    /*Khaki*/        Color(240,230,140),//rgb(240,230,140)
    /*Fuchsia*/      Color(255,0,255),  //rgb(255,0,255)
    /*Silver*/       Color(192,192,192),//rgb(192,192,192)
    /*RoyalBlue*/    Color(0,35,102),   //rgb(0,35,102)
    /*Green*/        Color(0,255,0),    //rgb(0,255,0)
    /*DarkOrange*/   Color(255,140,0),  //rgb(255,140,0)
    /*Purple*/       Color(159,0,197),  //rgb(159,0,197)
    /*Gold*/         Color(255,215,0),  //rgb(255,215,0)
    /*Red*/          Color(255,0,0),    //rgb(255,0,0)
    /*Aquamarine*/   Color(127,255,212),//rgb(127,255,212)
    /*Lime*/         Color(208,255,20), //rgb(208,255,20)
    /*AliceBlue*/    Color(240,248,255),//rgb(240,248,255)
    /*Sienna*/       Color(136,45,23),  //rgb(136,45,23)
    /*Orchid*/       Color(218,112,214),//rgb(218,112,214)
    /*Tan*/          Color(210,180,140),//rgb(210,180,140)
    /*LightPink*/    Color(255,182,193),//rgb(255,182,193)
    /*Yellow*/       Color(250,250,55), //rgb(250,250,55)
    /*HotPink*/      Color(255,105,180),//rgb(255,105,180)
    /*OliveDrab*/    Color(142,142,56), //rgb(142,142,56)
    /*SandyBrown*/   Color(0,72,186),   //rgb(0,72,186)
    /*DarkTurquoise*/Color(0,206,209)   //rgb(0,206,209)
};

string labels[] = 
{
    "aeroplane", "bicycle", "bird", "boat", "bottle",
    "bus", "car", "cat", "chair", "cow",
    "diningtable", "dog", "horse", "motorbike", "person",
    "pottedplant", "sheep", "sofa", "train", "tvmonitor"
};

float YoloBoundingBox::IntersectionOverUnion(RectangleF& boundingBoxA, RectangleF& boundingBoxB)
{
    int areaA = boundingBoxA.getWidth() * boundingBoxA.getHeight();

    if (areaA <= 0)
        return 0;

   int areaB = boundingBoxB.getWidth() * boundingBoxB.getHeight();

    if (areaB <= 0)
        return 0;

    int minX = max(boundingBoxA.getLeft(), boundingBoxB.getLeft());
    int minY = max(boundingBoxA.getTop(), boundingBoxB.getTop());
    int maxX = min(boundingBoxA.getRight(), boundingBoxB.getRight());
    int maxY = min(boundingBoxA.getBottom(), boundingBoxB.getBottom());

    int intersectionArea = max(maxY - minY, 0) * max(maxX - minX, 0);
    
    return (float)intersectionArea / (areaA + areaB - intersectionArea);
};

vector<YoloBoundingBox>* YoloBoundingBox::ParseOutputs(float yoloModelOutputs[], float threshold)
{
    auto boxes = new vector<YoloBoundingBox>();

    for (int row = 0; row < ROW_COUNT; row++)
    {
        for (int column = 0; column < COL_COUNT; column++)
        {
            for (int box = 0; box < BOXES_PER_CELL; box++)
            {
                int channel = (box * (CLASS_COUNT + BOX_INFO_FEATURE_COUNT));
                BoundingBoxDimensions* boundingBoxDimensions = ExtractBoundingBoxDimensions(yoloModelOutputs, row, column, channel);
                assert(boundingBoxDimensions!=nullptr);
                float confidence = GetConfidence(yoloModelOutputs, row, column, channel);
                if (confidence < threshold)
                    continue;

                
                CellDimensions& mappedBoundingBox = *MapBoundingBoxToCell(row, column, box, *boundingBoxDimensions);
                
                float* predictedClasses = ExtractClasses(yoloModelOutputs, row, column, channel);
                assert(predictedClasses!=nullptr);
                TopResult top = GetTopResult(predictedClasses);
                int topResultIndex = top.index;
                float topResultScore = top.score;

                float topScore = topResultScore * confidence;
                if (topScore < threshold)
                    continue;
                
                float X = (mappedBoundingBox.getX() - mappedBoundingBox.getWidth() / 2);
                float Y = (mappedBoundingBox.getY() - mappedBoundingBox.getHeight() / 2);
                float Width = mappedBoundingBox.getWidth();
                float Height = mappedBoundingBox.getHeight();

                BoundingBoxDimensions* Dimensions = new BoundingBoxDimensions(X,Y,Width,Height);
                assert(Dimensions!=nullptr);

                float Confidence = topScore;
                string Label = labels[topResultIndex];
                cout << "Label :" << Label << "Confidence Score :" << Confidence << endl;
                Color boxColor = classColors[topResultIndex];

                boxes->push_back(YoloBoundingBox(Dimensions,Confidence,Label,boxColor));
                
            }
        }
    }

    return boxes;
};

vector<YoloBoundingBox>* YoloBoundingBox::FilterBoundingBoxes(vector<YoloBoundingBox>* boxes, int limit, float threshold)
{
    
    assert(boxes != nullptr);

    auto iter = boxes->begin();
    for (; iter != boxes->end(); iter++)
    {
        auto boxA = *iter;
        boxA.setActive();
    }
    int activeCount = boxes->size();
    assert(activeCount != 0);

    vector<YoloBoundingBox>* sortedBoxes = OrderBoundingBoxes(boxes);

    auto results = new vector<YoloBoundingBox>();

    iter = sortedBoxes->begin();
    for (; iter != sortedBoxes->end(); iter++)
    {
        auto boxA = *iter;
        if (boxA.isActive())
        {
            results->push_back(boxA);

            if (results->size() >= limit)
                break;
            
            for (auto iter2 = iter + 1; iter2 != boxes->end(); iter2++)
            {
                auto boxB = *iter2;
                if (boxB.isActive())
                {
                    if (IntersectionOverUnion(*boxA.getRect(), *boxB.getRect()) > threshold)
                    {
                        boxB.inActive();
                        activeCount--;

                        if (activeCount <= 0)
                            break;
                    }
                }
            }
        }
    }

};

void YoloBoundingBox::DrawBoundingBox(string& inputImageLocation, string& outputImageLocation, string& imageName, vector<YoloBoundingBox>* filteredBoundingBoxes,bool show)
{
    assert(filteredBoundingBoxes != nullptr);


    std::string image_path = outputImageLocation + imageName;
    Mat img = imread(image_path, IMREAD_COLOR);
    
    if(img.empty())
    {
        std::cout << "Could not read the image: " << image_path << std::endl;
        return ;
    }
    if (show)
    {
        imshow("Display window", img);
    }
    
    
    cv::Size sz = img.size();
    int imageWidth = sz.width;
    int imageHeight = sz.height;

    int originalImageHeight = imageHeight;
    int originalImageWidth = imageWidth;

    for (auto& box : *filteredBoundingBoxes)
    {
        assert (box.bbd != nullptr);

        // Get Bounding Box Dimensions
        int x = (uint)max(box.getDimensions().getX(), 0);
        int y = (uint)max(box.getDimensions().getY(), 0);
        int width = (uint)min(originalImageWidth - x, box.getDimensions().getWidth());
        int height = (uint)min(originalImageHeight - y, box.getDimensions().getHeight());

        // Resize To Image
        x = (uint)originalImageWidth * x / WIDTH;
        y = (uint)originalImageHeight * y / HEIGHT;
        width = (uint)originalImageWidth * width / WIDTH;
        height = (uint)originalImageHeight * height / HEIGHT;

        // Bounding Box Text
        stringstream ss;
        ss << box.getLabel() <<" ("<< box.getConfidence() * 100.00f <<"%) ";

        string text = ss.str();
        cout << " class :" << text << endl;

        int fontFace = FONT_HERSHEY_SIMPLEX;
        double fontScale = 2;
        int thickness = 3;
        int baseline = 0;
        Size textSize = getTextSize(text, fontFace,
                                    fontScale, thickness, &baseline);
        baseline += thickness;
        
        Point textOrg(x, y - height - 1);
        // draw the box
        rectangle(img, textOrg + Point(0, baseline),
                textOrg + Point(textSize.width, -textSize.height),
                Scalar(0,0,255));
        // ... and the baseline first
        line(img, textOrg + Point(0, thickness),
            textOrg + Point(textSize.width, thickness),
            Scalar(0, 0, 255));
        // then put the text itself
        putText(img, text, textOrg, fontFace, fontScale,
                Scalar::all(255), thickness, 8);
        
    
    }
    outputImageLocation = string("./objects_detected/");
    if (!dirExists(outputImageLocation.c_str()))
    {
        const fs::path outputImage{outputImageLocation.c_str()};
        fs::create_directory(outputImage); 
    }

    if(img.empty())
    {
        std::cerr << "Something is wrong with the "  << imageName << std::endl;
    }
    // Save the img into a file
    imwrite(outputImageLocation + imageName, img); 
}; 
        
