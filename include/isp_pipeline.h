#pragma once

#include <opencv2/opencv.hpp>

class ISPPipeline
{
  public:
    static cv::Mat BM3D(cv::Mat &image);
    static cv::Mat GuidedFilter(int radius, cv::Mat &p, cv::Mat &I, float eps);

    static cv::Mat GrayGuidedFilter(int radius, cv::Mat &p, cv::Mat &I,
                                    float eps);

    static cv::Mat ColorGuidedFilter(int radius, cv::Mat &p, cv::Mat &I,
                                     float eps);

    static cv::Mat makePreview(const cv::Mat &bayer16,
                               const cv::Matx33f &rgbCam, int blackLevel,
                               int whiteLevel, double gamma = 2.2,
                               int bayerPattern = 0, float redGain = 1.0f,
                               float greenGain = 1.0f, float blueGain = 1.0f,
                               bool isDenoised = true);
};
