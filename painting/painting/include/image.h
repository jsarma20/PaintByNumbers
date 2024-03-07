#include <opencv2/core/mat.hpp>
#include <opencv2/imgcodecs.hpp>
#include <spdlog/spdlog.h>

using namespace std;

#ifndef IMAGE_H
#define IMAGE_H

typedef int length_t;

class Image
{
    public:
    enum Format
    {
        JPEG,
        PNG,
        BMP
    };
    Image(){width=0;height=0;_image=nullptr;}
    Image(Image &im);
    Image(Image&& im);
    Image& operator=(const Image &rhs);
    Image& operator=(Image &&rhs);
    Image(cv::Mat& image);
    Image(cv::Mat&& image);
    Image(length_t height, length_t width, int type);
    Image(const string filename, cv::ImreadModes mode =cv::IMREAD_COLOR);
    

    ~Image()
    {
        if(this->_image != nullptr)
        {
            this->_image = nullptr;
        }

    }

    bool isEmpty() const {return _image==nullptr;}

    bool load(const string file, cv::ImreadModes mode);

    bool store(const string file, Format format = Format::JPEG);

    void getColorSpace(cv::Mat& colors);

    int channels()const {return _image->channels();}

    cv::Mat getData()
    {
        if(_image == nullptr){throw std::runtime_error("No image data found.");}
        
        return *_image;
    }

    private:
    length_t width, height;
    std::shared_ptr<cv::Mat> _image;

};

#endif