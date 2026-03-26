#include "isp_pipeline.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

// 최종 구현 목표 :
// 강의노트에 적힌 ISP를 전부 시뮬레이팅 할 수 있도록 한다.

cv::Mat ISPPipeline::makePreview(
    const cv::Mat& bayer16,
    const cv::Matx33f& CCM,
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
            v = std::min(1.0f, v);
            dst[x] = v;
        }
    }
    
    cv::Mat normalized16;
    normalized.convertTo(normalized16, CV_16U, 65535.0);
    
    cv::Mat rgb16;
    
    // TODO : Implementing demosaicing myself
    if(bayerPattern == 0){
        cv::cvtColor(normalized16, rgb16, cv::COLOR_BayerRG2BGR);    
    }  else if(bayerPattern == 1) {
        cv::cvtColor(normalized16, rgb16, cv::COLOR_BayerBG2BGR);
    } else if(bayerPattern == 2) {
        v = v / denom;
        cv::cvtColor(normalized16, rgb16, cv::COLOR_BayerGR2BGR);
    } else if(bayerPattern == 3) {
        cv::cvtColor(normalized16, rgb16, cv::COLOR_BayerGB2BGR);
    }

    // White balancing
    cv::Mat bgr32;
    rgb16.convertTo(bgr32, CV_32F, 1.0 / 65535.0);


    // Gain이랑 Gamma 적용 한 번에 하면 되는데 일단 연습용

    cv::Mat xyz(bgr32.size(), CV_32FC3);

    for (int y = 0; y < bgr32.rows; ++y) {
        cv::Vec3f* bgrRow = bgr32.ptr<cv::Vec3f>(y);
        cv::Vec3f* xyzRow = xyz.ptr<cv::Vec3f>(y);

        for (int x = 0; x < bgr32.cols; ++x) {

            // WB
           // bgrRow[x][0] = bgrRow[x][0] * blueGain;
           // bgrRow[x][1] = bgrRow[x][1] * greenGain;
           // bgrRow[x][2] = bgrRow[x][2] * redGain;

            // Apply CCM
            cv::Vec3f bgr = bgrRow[x];
            cv::Vec3f rgb(bgr[2], bgr[1], bgr[0]);

            cv::Vec3f corrected = CCM * rgb;

            xyzRow[x][0] = corrected[0];
            xyzRow[x][1] = corrected[1];
            xyzRow[x][2] = corrected[2];
        }
    }

    cv::Mat gammaCorrected;

    cv::cvtColor(xyz, gammaCorrected, cv::COLOR_XYZ2BGR);

    
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

    // TODO : Denoising
    // TODO : Color correction
    // TODO : Tone mapping
    // TODO : Sharpening(Later)

    // TODO : Comparing with original data and restorated data

    cv::Mat result;
    cv::cvtColor(preview8, result, cv::COLOR_BGR2RGB);

    return result;
}