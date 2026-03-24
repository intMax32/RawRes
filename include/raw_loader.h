#pragma once

#include <string>
#include <opencv2/opencv.hpp>

struct RawImageData {
    cv::Mat bayer16; // CV_16UC1
    int width = 0;
    int height = 0;
    int blackLevel = 0;
    int whiteLevel = 0;
    int bayerPattern = 0;
    float wbRed = 1.0f;
    float wbGreen = 1.0f;
    float wbBlue = 1.0f;
};

class RawLoader {
public:
    static RawImageData load(const std::string& path);
};
