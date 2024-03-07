#include "engine.h"
#include "image.h"
#include <opencv2/core.hpp>
#include <spdlog/spdlog.h>
#include <opencv2/imgproc.hpp>
#include<filesystem>
#include<iostream>

using namespace cv;

int main(int argc, char* argv[])
{
    if(argc <3) throw std::runtime_error("missing arguments. Usage ./app /path/to/image.jpeg 5 (optional)/output/path");
    std::cout<<argv[0]<<" "<<argv[1]<<" "<<argv[2]<<std::endl;
    std::string filename = std::string(argv[1]);
    auto nColors  = stoi(std::string(argv[2]));
    std::string outputPath ="./";
    if(argc>3) outputPath = std::string(argv[3]);

    std::cout<<filename << " "<<nColors<<std::endl;
    Painting painter = Painting(filename);
    painter.generatePBN(nColors);

    Image painting = painter.getQuantizedImage();
    auto r = painting.store(outputPath+"painting.jpeg");

    Image canvas = painter.getContourImage();

    r = canvas.store(outputPath+"canvas.jpeg");

    Image colorPallet = painter.getColorPallet();

    colorPallet.store(outputPath+"colorPallet.jpeg");

    Image opacityImg = painter.getQuantizedImage(10);
    
    opacityImg.store(outputPath+"opacity.jpeg");
    
    return 0;
}
