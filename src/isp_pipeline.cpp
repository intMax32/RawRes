#include "isp_pipeline.h"
#include "noise_removal/guided_filter.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

// TODO : HDR 지원하기

cv::Mat ISPPipeline::makePreview(const cv::Mat &bayer16,
                                 const cv::Matx33f &rgbCam, int blackLevel,
                                 int whiteLevel, double gamma, int bayerPattern,
                                 float redGain, float greenGain, float blueGain,
                                 int denoiser)
{
    if (bayer16.empty() || bayer16.type() != CV_16UC1)
    {
        throw std::runtime_error("Invalid Bayer image.");
    }

    cv::Mat normalized = cv::Mat::zeros(bayer16.size(), CV_32F);

    const float denom = std::max(1, whiteLevel - blackLevel);
    // Normalise and white balance in Bayer space.
    for (int y = 0; y < bayer16.rows; ++y)
    {
        const ushort *src = bayer16.ptr<ushort>(y);
        float *dst = normalized.ptr<float>(y);

        for (int x = 0; x < bayer16.cols; ++x)
        {
            const bool evenY = (y % 2 == 0);
            const bool evenX = (x % 2 == 0);

            float v =
                static_cast<float>(src[x]) - static_cast<float>(blackLevel);
            v = std::max(0.0f, v);
            v /= denom;

            float channelGain = greenGain;

            switch (bayerPattern)
            {
            case 0: // RGGB -> R G / G B
                if (evenY && evenX)
                {
                    channelGain = redGain;
                }
                else if (!evenY && !evenX)
                {
                    channelGain = blueGain;
                }
                break;

            case 1: // BGGR -> B G / G R
                if (evenY && evenX)
                {
                    channelGain = blueGain;
                }
                else if (!evenY && !evenX)
                {
                    channelGain = redGain;
                }
                break;

            case 2: // GRBG -> G R / B G
                if (evenY && !evenX)
                {
                    channelGain = redGain;
                }
                else if (!evenY && evenX)
                {
                    channelGain = blueGain;
                }
                break;

            case 3: // GBRG -> G B / R G
                if (evenY && !evenX)
                {
                    channelGain = blueGain;
                }
                else if (!evenY && evenX)
                {
                    channelGain = redGain;
                }
                break;

            default:
                break;
            }

            v *= channelGain;
            v = std::min(1.0f, v);
            dst[x] = v;
        }
    }

    cv::Mat normalized16;
    normalized.convertTo(normalized16, CV_16U, 65535.0);

    cv::Mat bgr16;

    // Demosaicing
    if (bayerPattern == 0)
    { // RGGB
        cv::cvtColor(normalized16, bgr16, cv::COLOR_BayerRG2BGR_EA);
    }
    else if (bayerPattern == 1)
    {
        cv::cvtColor(normalized16, bgr16, cv::COLOR_BayerBG2BGR);
    }
    else if (bayerPattern == 2)
    {
        cv::cvtColor(normalized16, bgr16, cv::COLOR_BayerGR2BGR);
    }
    else if (bayerPattern == 3)
    {
        cv::cvtColor(normalized16, bgr16, cv::COLOR_BayerGB2BGR);
    }

    cv::Mat bgr32;
    bgr16.convertTo(bgr32, CV_32F, 1.0 / 65535.0);

    cv::Mat correctedBgr(bgr32.size(), CV_32FC3);

    for (int y = 0; y < bgr32.rows; ++y)
    {
        cv::Vec3f *bgrRow = bgr32.ptr<cv::Vec3f>(y);
        cv::Vec3f *correctedBgrRow = correctedBgr.ptr<cv::Vec3f>(y);

        for (int x = 0; x < bgr32.cols; ++x)
        {
            cv::Vec3f bgr = bgrRow[x];
            cv::Vec3f rgb(bgr[2], bgr[1], bgr[0]);

            cv::Vec3f corrected = rgbCam * rgb;

            correctedBgrRow[x][0] = corrected[2];
            correctedBgrRow[x][1] = corrected[1];
            correctedBgrRow[x][2] = corrected[0];
        }
    }

    cv::Mat denoisedImg;

    if (denoiser < 1) // No denoise
    {
        cv::Mat gammaCorrected = correctedBgr.clone();

        const float invGamma = 1.0f / static_cast<float>(gamma);

        for (int y = 0; y < gammaCorrected.rows; ++y)
        {
            cv::Vec3f *row = gammaCorrected.ptr<cv::Vec3f>(y);
            for (int x = 0; x < gammaCorrected.cols; ++x)
            {
                row[x][0] =
                    std::pow(std::clamp(row[x][0], 0.0f, 1.0f), invGamma);
                row[x][1] =
                    std::pow(std::clamp(row[x][1], 0.0f, 1.0f), invGamma);
                row[x][2] =
                    std::pow(std::clamp(row[x][2], 0.0f, 1.0f), invGamma);
            }
        }

        cv::Mat preview7;
        gammaCorrected.convertTo(preview7, CV_8UC3, 255.0);
        cv::Mat preview8;

        cv::cvtColor(preview7, preview8, cv::COLOR_BGR2RGB);
        // TODO : Comparing with original data and restorated data

        return preview8;
    }
    else if (denoiser == 1) // guided filter
    {
        // Test code

        cv::Mat temp;
        cv::cvtColor(correctedBgr, temp, cv::COLOR_BGR2GRAY);
        denoisedImg = NoiseRemoval::GuidedFilter(3, correctedBgr, temp, 0.001f);
    }
    else if (denoiser == 2) // BM3D
    {
        denoisedImg = correctedBgr;
    }

    // Gamma correction

    cv::Mat gammaCorrected = denoisedImg.clone();

    const float invGamma = 1.0f / static_cast<float>(gamma);

    for (int y = 0; y < gammaCorrected.rows; ++y)
    {
        cv::Vec3f *row = gammaCorrected.ptr<cv::Vec3f>(y);
        for (int x = 0; x < gammaCorrected.cols; ++x)
        {
            row[x][0] = std::pow(std::clamp(row[x][0], 0.0f, 1.0f), invGamma);
            row[x][1] = std::pow(std::clamp(row[x][1], 0.0f, 1.0f), invGamma);
            row[x][2] = std::pow(std::clamp(row[x][2], 0.0f, 1.0f), invGamma);
        }
    }

    cv::Mat preview7;
    gammaCorrected.convertTo(preview7, CV_8UC3, 255.0);
    cv::Mat preview8;

    cv::cvtColor(preview7, preview8, cv::COLOR_BGR2RGB);
    // TODO : Comparing with original data and restorated data

    return preview8;
}
