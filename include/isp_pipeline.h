#pragma once

#include <opencv2/opencv.hpp>

class ISPPipeline {
public:
    static cv::Mat makePreview(
        const cv::Mat& bayer16,
        int blackLevel,
        int whiteLevel,
        double gain = 1.0,
        double gamma = 2.2
    );
};