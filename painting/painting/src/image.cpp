#include "image.h"
#include<iostream>
#include<vector>

Image::Image(length_t height, length_t width, int type)
{
    std::vector<int>sizes = {height, width};
    this->_image = std::make_shared<cv::Mat>(sizes,type);
    this->width = width;
    this->height = height;
}

Image::Image(Image &im)
{   
    if(!im.isEmpty())
    {
        auto temp = *(im._image);
        this->_image = std::make_shared<cv::Mat>(temp);
    }
    this->height = im.height;
    this->width = im.width;
}

Image& Image::operator=(const Image &rhs)
{
    if(!rhs.isEmpty())
    {
        auto temp = *(rhs._image);
        this->_image = std::make_shared<cv::Mat>(temp);
    }
    this->height = rhs.height;
    this->width = rhs.width;
    return *this;
}

Image& Image::operator=(Image &&rhs)
{
    this->_image = rhs._image;
    this->height = rhs.height;
    this->width = rhs.width;

    rhs._image = nullptr;
    return *this;
}

Image::Image(Image&& im)
{
    this->_image = im._image;
    this->height = im.height;
    this->width = im.width;

    im._image = nullptr;
}


Image::Image(const string filename, cv::ImreadModes mode)
{
    auto success = load(filename,mode);
    if(!success)
    {
        std::cout<<"Failed to laod Image"<<std::endl;
        spdlog::warn("failed to load the image {0}", filename);
    }
    else std::cout<<"laoded Image"<<std::endl;
}

bool Image::load(const string filename, cv::ImreadModes mode)
{
    
    auto im = cv::imread(filename,mode);
    if(im.empty()) return false;
    this->_image = nullptr;//release any old data
    this->_image = std::make_shared<cv::Mat>(std::move(im));// Need to check alternatives

    return true;
}

bool Image::store(const string filename, Format format)
{
    if(_image == nullptr)
    {
        spdlog::error("Can't save the image");
        throw std::runtime_error("no image to write");
    }
    spdlog::info("image saving to file {0}",filename);
    vector<int> compression_params;
    switch (format)
    {
    case Image::Format::JPEG:
        compression_params.push_back(cv::IMWRITE_JPEG_QUALITY);
        compression_params.push_back(100);
        break;
    case Image::Format::PNG:
        break;    
    default:
        break;
    }

    return cv::imwrite(filename, *_image,compression_params);
}

Image::Image(cv::Mat& image)
{
    this->_image = make_shared<cv::Mat>(image);
    this->height = image.rows;
    this->width = image.cols;
}

Image::Image(cv::Mat&& image)
{
    this->_image = make_shared<cv::Mat>(image);
    this->height = image.rows;
    this->width = image.cols;
}

void Image::getColorSpace(cv::Mat& colors)
{
    auto img = this->_image;
    if(img == nullptr || img->channels()<2)
    {
        throw std::runtime_error("invalid  image.");
        spdlog::critical("Couldn't get color space. Invalid image");
    }
    for (int y=0; y<img->rows; y++)
    {
        for(int x=0;x<img->cols;x++)
        {
            for(int z=0; z<img->channels();z++)
            {
                colors.at<float>(y+x*img->rows,z) = img->at<cv::Vec3b>(y,x)[z];
            }
        }
    }
}