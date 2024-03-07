#include <vector>
#define private public
#include<iostream>
#include <catch2/catch_test_macros.hpp>
#include <opencv2/core/mat.hpp>
#include "image.h"
#include "engine.h"



Image generateImage(uint Size)
{
    // Generate a size x size image with size/2 x size/2 red square in the middle.
    auto im = cv::Mat(Size,Size,CV_8UC3);
    for(int h=0;h<Size;h++)
    {
        for(int w=0;w<Size;w++)
        {
            if((h>Size/4 && h <= 3*Size/4) && (w>Size/4 && w <= 3*Size/4))
                im.at<Vec3b>(h,w) = Vec3b(0,0,255);
            else
                im.at<Vec3b>(h,w) = Vec3b(0,0,0);
        }
    }
    return Image(im);
}

TEST_CASE( "Create quantized image" ) 
{
    uint nColor = 2;
    auto imgptr = std::make_shared<Image>(generateImage(8));
    auto painting = Painting(imgptr);
    auto quantizedImg = painting.generateNColorImage(nColor);
    auto colors = painting.getColorMap();

    REQUIRE(colors.size() == nColor);

}

bool checkIfPointInColorRegion(cv::Point p, Painting::ColorRegion& region)
{
    for(auto [id,color] : region.colorMap)
    {
        for(auto& areaContour : region.contours[id])
        {
            auto pos = std::find(areaContour.begin(),areaContour.end(),p);
            if(pos != areaContour.end()) return true;
        }
    }
    return false;
}

TEST_CASE( "Contours")
{
    uint nColor = 2;
    uint size = 100;
    auto imgptr = std::make_shared<Image>(generateImage(size));

    auto painting = Painting(imgptr);
    auto quantizedImg = painting.generateNColorImage(nColor);
    auto colors = painting.getColorMap();
    Painting::ColorRegion colorRegions = painting.getContours(colors,quantizedImg);

    // make sure corner points of inner square are near the contour
    bool status = true;
    for(auto&p : {cv::Point(size/4,size/4), cv::Point(size/4,3*size/4), cv::Point(3*size/4,size/4), cv::Point(3*size/4,3*size/4)} )
    {
        bool near = false;
        // check if one of the neighboring pixels going through the contour
        auto p1 = p - Point(0,1);
        auto p2 = p - Point(1,0);
        auto p3 = p + Point(0,1);
        auto p4 = p + Point(1,0);
        near |= checkIfPointInColorRegion(p1,colorRegions);
        near |= checkIfPointInColorRegion(p2,colorRegions);
        near |= checkIfPointInColorRegion(p3,colorRegions);
        near |= checkIfPointInColorRegion(p4,colorRegions);
        status &= near;
    }
    REQUIRE(status==true);
}

 cv::Mat drawSquareStartingAt(uint dim, cv::Point starting,std::vector<Point>& points)
{
    uint hEnd=starting.y+dim, wEnd=starting.x+dim;
    uint hBegin=starting.y, wBegin=starting.x;

    //Align the image size with the multiples of 2 for the single channel image. 
    //Otherwise opencv memory allocation for the image will result in SEGERR
    uint ImageH = (hEnd%2==1)? hEnd+1 : hEnd;
    uint ImageW = (wEnd%2==1)? wEnd+1 : wEnd;
    cv::Mat square3c = cv::Mat::zeros(ImageH,ImageW,CV_8UC3);
    for(int c=wBegin; c<wEnd;c++)
    {
        points.push_back(Point(c,hBegin));
        square3c.at<Vec3b>(c,hBegin) = Vec3b(255,255,255);
    }
    for(int r=hBegin;r<hEnd;r++)
    {
        points.push_back(Point(wEnd-1,r));
        square3c.at<Vec3b>(wEnd-1,r) = Vec3b(255,255,255);
    }
    for(int c=wEnd-1;c>=wBegin;c--)
    {
        points.push_back(Point(c,hEnd-1));
        square3c.at<Vec3b>(c,hEnd-1) = Vec3b(255,255,255);
    }
    for(int r=hEnd-1;r>=hBegin;r--)
    {
        points.push_back(Point(wBegin,r));
        square3c.at<Vec3b>(wBegin,r) = Vec3b(255,255,255);
    }
    
    // for(int r=hBegin; r<hEnd; r++)
    // {
    //     for(int c=wBegin;c<wEnd;c++)
    //     {
    //         if(r==hBegin||r==hEnd-1||c==wBegin||c==wEnd-1)
    //         {
    //             square1d.at<uint>(r,c)=255;
    //         }
    //     }
    // }
    //vector<vector<Point> > contours;
    //cv::findContours(square1d,contours,RetrievalModes::RETR_TREE,ContourApproximationModes::CHAIN_APPROX_NONE);
    //if(contours.size() == 0) spdlog::error("cv::findContours couldn't find points");
    //points = contours.at(0);
    return square3c;
}


TEST_CASE( "Centroid")
{
    uint size=11;
    uint offset = 1;
    auto starting = cv::Point(offset,offset);
    cv::Mat sq;
    std::vector<cv::Point> square;
    sq = drawSquareStartingAt(size,starting,square);
    cv::Point foundCenter = findCentroid(square);

    double trueCenterX = offset + size/2.0 -1; //offset + half of the length - 1px for starting pixel at 0 instead 1
    double trueCenterY = offset + size/2.0 -1; 

    /**
     * True center coordinates might be within 0.5 pixel deviation due to 
     * integer rounding off of the findCentroid algorithm
     * 
     * OpenCV typically assumes that the top and left boundary of the rectangle are 
     * inclusive, while the right and bottom boundaries are not.
     * So make the bottom bondaries longer by setting heigh and width to 1.1 instead 1.
     */
    auto boundingRect = cv::Rect2d(trueCenterX -0.5,trueCenterY-0.5,1.1,1.1);
    std::cout<<boundingRect.x<<","<< boundingRect.y<<" 1x1, looking for"<<foundCenter.x<<","<<foundCenter.y<<endl;
    REQUIRE(boundingRect.contains(foundCenter));
}