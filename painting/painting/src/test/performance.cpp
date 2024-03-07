#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>
#include <opencv2/core/mat.hpp>
#include "image.h"
#include "engine.h"

Image generateColorImage()
{
    uint height=1000, width=1000;
    cv::Mat colorImg = cv::Mat::zeros(height,width,CV_8UC3);
    for(int row=1;row<height-1;row++)
    {
        for(int col=1;col<width-1;col++)
        {
            uint var = col/20;
            var %= 255;
            colorImg.at<cv::Vec3b>(row,col) = cv::Vec3b(var,255-var,0);
        }
    }
    return Image(colorImg);
}

bool generatePBN(Image& img, uint nColors)
{

    Painting painter = Painting(std::make_shared<Image>(img));
    painter.generatePBN(nColors);

    Image painting = painter.getQuantizedImage();
    if(painting.isEmpty()) return false;

    Image canvas = painter.getContourImage();
    if(canvas.isEmpty()) return false;

    Image colorPallet = painter.getColorPallet();
    if(colorPallet.isEmpty()) return false;
    
    return true;
}

TEST_CASE("GET PBN")
{

    auto img = generateColorImage();

    BENCHMARK("Benchmark generatePBN")
    {
        bool success = generatePBN(img,5);
        CHECK(success == true);
    };

}


