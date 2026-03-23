#pragma once

#include <string>
#include <opencv2/opencv.hpp>

struct RawImageData {
    cv::Mat bayer16; // CV_16UC1
    int width = 0;
    int height = 0;
    int blackLevel = 0;
    int whiteLevel = 0;
};

class RawLoader {
public:
    static RawImageData load(const std::string& path);
}