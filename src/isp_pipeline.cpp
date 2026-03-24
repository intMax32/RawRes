#include "isp_pipeline.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

// 최종 구현 목표 :
// 강의노트에 적힌 ISP를 전부 시뮬레이팅 할 수 있도록 한다.

cv::Mat ISPPipeline::makePreview(
    const cv::Mat& bayer16,
    int blackLevel,
    int whiteLevel,
    double gamma,
    int bayerPattern,
    float redGain,
    float greenGain,
    float blueGain
) {
    if (bayer16.empty() || bayer16.type() != CV_16UC1) {
        throw std::runtime_error("Invalid Bayer image.");
    }

    cv::Mat normalized = cv::Mat::zeros(bayer16.size(), CV_32F);

    const float denom = std::max(1, whiteLevel - blackLevel);

    for (int y = 0; y < bayer16.rows; ++y) {
        const ushort* src = bayer16.ptr<ushort>(y);
        float* dst = normalized.ptr<float>(y);

        for (int x = 0; x < bayer16.cols; ++x) {
            float v = static_cast<float>(src[x]) - static_cast<float>(blackLevel);
            v = std::max(0.0f, v);
            v = v / denom;
            v = std::min(1.0f, v);
            dst[x] = v;
        }
    }

    cv::Mat normalized16;
    normalized.convertTo(normalized16, CV_16U, 65535.0);

    cv::Mat rgb16;
    
    if(bayerPattern == 0){
        cv::cvtColor(normalized16, rgb16, cv::COLOR_BayerRG2BGR);    
    }  else if(bayerPattern == 1) {
        cv::cvtColor(normalized16, rgb16, cv::COLOR_BayerBG2BGR);
    } else if(bayerPattern == 2) {
        cv::cvtColor(normalized16, rgb16, cv::COLOR_BayerGR2BGR);
    } else if(bayerPattern == 3) {
        cv::cvtColor(normalized16, rgb16, cv::COLOR_BayerGB2BGR);
    }

    // TODO : OpenCV 내부 디모자이킹 방법 찾아보기
    // TODO : Auto white balancing
    cv::Mat rgb32;
    rgb16.convertTo(rgb32, CV_32F, 1.0 / 65535.0);

    for (int y = 0; y < rgb32.rows; ++y) {
    cv::Vec3f* row = rgb32.ptr<cv::Vec3f>(y);
    for (int x = 0; x < rgb32.cols; ++x) {
        row[x][0] = std::clamp(row[x][0] * blueGain, 0.0f, 1.0f);
        row[x][1] = std::clamp(row[x][1] * greenGain, 0.0f, 1.0f);
        row[x][2] = std::clamp(row[x][2] * redGain, 0.0f, 1.0f);
    }
}

    cv::Mat gammaCorrected = rgb32.clone();
    const float invGamma = 1.0f / static_cast<float>(gamma);

    for (int y = 0; y < gammaCorrected.rows; ++y) {
        cv::Vec3f* row = gammaCorrected.ptr<cv::Vec3f>(y);
        for (int x = 0; x < gammaCorrected.cols; ++x) {
            row[x][0] = std::pow(std::clamp(row[x][0], 0.0f, 1.0f), invGamma);
            row[x][1] = std::pow(std::clamp(row[x][1], 0.0f, 1.0f), invGamma);
            row[x][2] = std::pow(std::clamp(row[x][2], 0.0f, 1.0f), invGamma);
        }
    }

    cv::Mat preview8;
    gammaCorrected.convertTo(preview8, CV_8UC3, 255.0);

    cv::Mat result;
    cv::cvtColor(preview8, result, cv::COLOR_BGR2RGB);

    return result;
}