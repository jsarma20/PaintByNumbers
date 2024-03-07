#include "image.h"
#include <map>
#include <list>
#include<vector>
#include<string>
#include <opencv2/highgui.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/imgproc.hpp>

#ifndef ENGINE_H
#define ENGINE_H

using namespace cv;


const uint MAX_KMEANS_ATTEMPTS = 5;
const double MIN_CONTOUR_AREA_IN_PIXELS = 50.0;
const Scalar CONTOUR_COLOR = CV_RGB(220,220,220);
const Scalar CONTOUR_LABLE_COLOR = CV_RGB(170,170,170);

Point findCentroid(std::vector<Point>& area);


class Painting
{

public:
    typedef cv::Vec3b Color8Bit;
    typedef std::map<int, Color8Bit> ColorMap;
    struct ColorRegion
    {
        ColorMap colorMap;
        std::map<int, std::vector<std::vector<Point>>> contours;
    };
    Painting(std::shared_ptr<Image> im){_img=im;};

    Painting(const std::string filename)
    {
        _img=std::make_shared<Image>(filename,cv::ImreadModes::IMREAD_COLOR);
    }

    std::list<Color8Bit> getColorMap(){return colorMap;}

    void drawImage(Image&, ColorRegion);

    Image generateContourImage(ColorRegion& colorRegion);

    Image generateColorPallet(ColorRegion&);

    void generatePBN(uint nColors);

    Image getContourImage(){return _contourImage;}
    Image getColorPallet(){return _colorPallet;}
    Image getQuantizedImage(uint oppacity=0)
    {
        if(oppacity==0)
            return _quantizedImage;
        else return getQuantizedImageWithOpacity(oppacity);
    }

private:

    Image getQuantizedImageWithOpacity(uint oppacity);
    ColorRegion getContours(std::list<Color8Bit> colors, Image quantizedImg);
    Image generateNColorImage(const uint maxColors);
    std::vector<std::vector<cv::Point>> getContours(std::shared_ptr<Image>);
    std::shared_ptr<Image> _img;
    Image _contourImage, _colorPallet;
    std::list<Color8Bit> colorMap;
    Image _quantizedImage;

};
#ifdef DEV_GUI
void displayMat(std::string lable,Mat&m)
{

    cv::imshow(lable,m);
    cv::waitKey();
 
}
#endif

#endif