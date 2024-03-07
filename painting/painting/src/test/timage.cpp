#include <catch2/catch_test_macros.hpp>
#include <opencv2/core/mat.hpp>
#include "image.h"
#include "engine.h"

const length_t imHeight = 200, imWidth = 100;

TEST_CASE( "Create Image" ) 
{
    
    Image im = Image(imHeight,imWidth,CV_8UC3);
    REQUIRE(im.getData().rows == imHeight);
    REQUIRE(im.getData().cols == imWidth);

}

TEST_CASE( "ColorSpace of Image" ) 
{
    Image greyScaleImage = Image(imHeight,imWidth, CV_8UC1);
    Image colorImage = Image(imHeight,imWidth, CV_8UC3);
    cv::Mat color(imHeight * imWidth, 3, CV_32F);
    
    REQUIRE_THROWS(greyScaleImage.getColorSpace(color));
    REQUIRE_NOTHROW(colorImage.getColorSpace(color)); 
}

TEST_CASE("Image lifetime")
{
    Image src = Image(imHeight,imWidth,CV_8U);
    Image dst = Image(std::move(src));
    REQUIRE(dst.isEmpty() == false);
    REQUIRE(src.isEmpty() == true);
    src = std::move(dst);
    REQUIRE(src.isEmpty() == false);
    REQUIRE(dst.isEmpty() == true);
}