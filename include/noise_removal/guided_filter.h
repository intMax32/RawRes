#pragma once

#include <opencv2/opencv.hpp>

namespace NoiseRemoval
{
cv::Mat GuidedFilter(int radius, cv::Mat &p, cv::Mat &I, float eps);

cv::Mat GrayGuidedFilter(int radius, cv::Mat &p, cv::Mat &I, float eps);

cv::Mat ColorGuidedFilter(int radius, cv::Mat &p, cv::Mat &I, float eps);
}

