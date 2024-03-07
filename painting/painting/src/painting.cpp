#include "engine.h"

Image Painting::generateNColorImage(const uint maxColors)
{
    auto image = this->_img;
    auto src = image->getData();
    Mat colors(src.rows * src.cols, 3, CV_32F);
    image->getColorSpace(colors);
    Mat labels, centers;

    kmeans(colors, maxColors, labels, TermCriteria(TermCriteria::MAX_ITER|TermCriteria::EPS, 10000, 0.0001), MAX_KMEANS_ATTEMPTS, KMEANS_PP_CENTERS, centers);
    
    Mat new_image = Mat(src.size(),src.type());
    for( int y = 0; y < src.rows; y++ )
    for( int x = 0; x < src.cols; x++ )
    { 
      int cluster_idx = labels.at<int>(y + x*src.rows,0);
      for(int z=0; z<src.channels(); z++)
      {
        new_image.at<Vec3b>(y,x)[z] = uchar(centers.at<float>(cluster_idx, z));
      }
    }
    
    this->colorMap = std::list<Color8Bit>();
    
    for(int i=0; i<centers.rows;i++)
    {
      auto rgb = centers.row(i);
      #ifdef DEV_VERBOSE
      spdlog::info("primary colors from image: rgb({0},{1},{2} ",uchar(rgb.at<float>(0)),uchar(rgb.at<float>(1)),uchar(rgb.at<float>(2)));
      #endif
      Painting::Color8Bit c = Painting::Color8Bit({uchar(rgb.at<float>(0)),uchar(rgb.at<float>(1)),uchar(rgb.at<float>(2))});
      this->colorMap.push_back(c);
    }
      
    return Image(new_image);
}

Image Painting::getQuantizedImageWithOpacity(uint opacity)
{
  // TODO fix opacity
  if(_quantizedImage.isEmpty()) return Image();
  auto qImg = _quantizedImage.getData();
  cv::cvtColor(qImg,qImg,cv::COLOR_RGB2Lab);
  for(int i=0;i<qImg.rows;i++)
  {
    for(int j=0;j<qImg.cols;j++)
    {
      auto l= qImg.at<Vec3b>(i,j)[0];
      auto newLum = 255;
      
      qImg.at<Vec3b>(i,j)[0] = (uint8_t)(newLum);

    }
  }
  cv::cvtColor(qImg,qImg,cv::COLOR_Lab2RGB);
  return Image(qImg);
}

void Painting::generatePBN(uint nColors)
{

  this->_quantizedImage = generateNColorImage(nColors);
  Painting::ColorRegion color = this->getContours(this->colorMap, this->_quantizedImage);
  
  this->_contourImage = generateContourImage(color);

  this->_colorPallet = generateColorPallet(color);
}

inline void displayColor(Painting::Color8Bit rgb)
{
  int height = 200, width = 200;
  cv::Mat image = cv::Mat(height,width,CV_8UC3);
  for(int h=0;h<height;h++)
  for(int w=0;w<width;w++)
  image.at<Vec3b>(h,w) = {uchar(rgb[0]),uchar(rgb[1]),uchar(rgb[2])};
  cv::imshow("Show Color", image);
  waitKey(0);
}


Painting::ColorRegion Painting::getContours(std::list<Color8Bit> colors, Image quantizedImg)
{
  Painting::ColorRegion regions;
  int colorId=0;
  //For each color create a greyScale image based on the color and find contours
  cv::Mat hsvImage;
  cv::cvtColor(quantizedImg.getData(),hsvImage,COLOR_RGB2HSV);

  for(auto color : colors)
  {
    cv::Mat bgr = cv::Mat(1,1,CV_8UC3);
    bgr.at<Vec3b>(0,0)= color;
    cv::Mat hsv;
    cv::cvtColor(bgr,hsv,COLOR_RGB2HSV);

    auto lower = hsv.at<Vec3b>(0,0) - Vec3b({5,0,0});
    auto higher = hsv.at<Vec3b>(0,0) + Vec3b({5,0,0});

    cv::Mat out = cv::Mat(quantizedImg.getData().rows,quantizedImg.getData().cols,CV_8U);
    cv::inRange(hsvImage,lower,higher,out);
    cv::Mat inverted;
    cv::bitwise_not(out,inverted);

    auto contours = getContours(std::make_shared<Image>(out));
    if(contours.empty())
    {
      spdlog::warn("No contour found for color({0} {1} {2})",color[0],color[1],color[2]);
      continue;
    }
    regions.contours[colorId] = contours;
    regions.colorMap[colorId] = color;
    colorId++;
  }
  return regions;
}

// We are rounding off of the actual centroid to integer coordinates, 
// thus actual centroid might be within 0.5 deviation of the results.
Point findCentroid(std::vector<Point>& area)
{
  cv::Moments m = cv::moments(area);
  auto x = int(m.m10/m.m00);
  auto y = int(m.m01/m.m00);
  return Point(x,y);
}

Image Painting::generateContourImage(ColorRegion& regions)
{
  cv::Mat img = cv::Mat(this->_img->getData().size(),CV_8UC3);
  img.setTo(Color8Bit(255,255,255));
  for(auto [id,color] : regions.colorMap)
  {
    for(auto& area : regions.contours[id])
    {
      for(auto&point : area)
      {
        cv::circle(img,point,0,CONTOUR_COLOR,-1,-1,0);
      }
      //label
      auto center = findCentroid(area);
      cv::putText(img,to_string(id),center,1,0.8,CONTOUR_LABLE_COLOR);
    }
  }
  return Image(img);
}    

std::vector<std::vector<Point>> Painting::getContours(std::shared_ptr<Image> src)
{
  vector<vector<Point> > contours;
  cv::Mat grey;

  cv::findContours(src->getData(),contours,RetrievalModes::RETR_TREE,ContourApproximationModes::CHAIN_APPROX_NONE);
  grey = src->getData();

  std::vector<std::vector<Point>>out;
  #ifdef DEV_GUI
  cv::Mat canvas =cv::Mat::zeros(grey.size(),CV_8UC3);
  #endif
  for(auto &c : contours)
  {
    double area = cv::contourArea(c);
    if(area< MIN_CONTOUR_AREA_IN_PIXELS)
    {
      continue;
    }
    std::vector<Point> tmp;
    for(auto &p :c)
    {
      #ifdef DEV_GUI
      cv::circle(canvas,p,0,CONTOUR_COLOR,-1,-1,0);
      #endif
      tmp.push_back(p);
    }
    out.push_back(tmp);
    #ifdef DEV_GUI
    displayMat("Interative Region",canvas);
    #endif
  }
  if(contours.size()==0)
  {
    auto v = std::vector<Point>();
    auto r = std::vector<decltype(v)>();
    r.push_back(v);
    return r;
  }

  return out;
}

struct Display
  {
    const static uint xoffset = 0, yoffset = 0; 
    const static uint height = 40;
    const static uint width  = 200;
    struct Color
    {
      Color(Painting::Color8Bit c): _color(c){};
      const static uint xoffset = 5, yoffset = 0;
      const static uint height = 15;
      const static uint width  = 15;
      Painting::Color8Bit _color;
    } color;

    struct Lable
    {
      Lable(Painting::Color8Bit c){rgb = to_string(c[0])+","+to_string(c[1])+","+to_string(c[2]);}
      const static uint xoffset = 25, yoffset = 0;
      const static uint height = 15;
      const static uint width  = 25;
      std::string rgb;
    }lable;
  };

Image Painting::generateColorPallet(ColorRegion& colors)
{
  cv::Mat colorPallet;
  uint nColors = colors.colorMap.size();
  uint nColumns = 2;

  uint nrows = uint((nColors+1)/nColumns);
  uint topMargin = 20, leftMargin = 10;
  uint canvasHeight = nrows * Display::height + topMargin;
  uint canvasWidth = nColumns * Display::width + leftMargin;
  colorPallet = cv::Mat(canvasHeight,canvasWidth, CV_8UC3);
  colorPallet.setTo(Color8Bit(255,255,255));

  uint colorId = 0;
  for(int r=0; r<nrows; r++)
  {
    for(int c=0; c<nColumns; c++)
    {
      auto color = colors.colorMap[colorId];
      uint x_pos = leftMargin + c*Display::width;
      uint y_pos = topMargin + r*Display::height;
      //draw square filled with color
      for(int h=0;h<Display::Color::height;h++)
      {
        for(int w=0;w<Display::Color::width;w++)
        {
          colorPallet.at<Color8Bit>(y_pos+Display::Color::yoffset+h,
          x_pos+Display::Color::xoffset+w) = color;
        }
      }
      //draw lable
      std::string lable = to_string(colorId)+" : "+Display::Lable(color).rgb;
      auto lablePos = Point(x_pos+Display::Lable::xoffset, y_pos+Display::Lable::yoffset);
      cv::putText(colorPallet,lable,lablePos,1,0.8,CONTOUR_LABLE_COLOR);
      colorId++;
    }
  }
  return Image(colorPallet);
}


void Painting::drawImage(Image& im, Painting::ColorRegion regions)
{
  for(auto [id,color] : regions.colorMap)
  {
    spdlog::info("Generaing Color contour{0}",id);
    displayColor(color);
    for(auto& area : regions.contours[id])
    {
      for(auto&point : area)
      {
        cv::circle(im.getData(),point,1,CV_RGB(170,255,0),-1,-1,0);
      }
    }
    std::string colorLable = std::to_string(color[0])+","+std::to_string(color[1])+","+std::to_string(color[2]);
    //cv::putText(im.getData(),colorLable,regions.contours[id].at(0),1,0.5,CV_RGB(255,0,0));
    cv::imshow("contours", im.getData());
    waitKey(0);
  }
}