#include "isp_pipeline.h"
#include "noise_removal/guided_filter.h"
#include <algorithm>
#include <cmath>
#include <stdexcept>

// TODO : HDR 지원하기

namespace
{
float luminanceFromBgr(const cv::Vec3f &bgr)
{
    const float r = std::max(0.0f, bgr[2]);
    const float g = std::max(0.0f, bgr[1]);
    const float b = std::max(0.0f, bgr[0]);

    return 0.2126f * r + 0.7152f * g + 0.0722f * b;
}

float findMaxLuminance(const cv::Mat &linearBgr)
{
    float maxLuminance = 0.0f;

    for (int y = 0; y < linearBgr.rows; ++y)
    {
        const cv::Vec3f *row = linearBgr.ptr<cv::Vec3f>(y);
        for (int x = 0; x < linearBgr.cols; ++x)
        {
            maxLuminance = std::max(maxLuminance, luminanceFromBgr(row[x]));
        }
    }

    return std::max(maxLuminance, 1.0f);
}

cv::Mat applyExtendedReinhardToneMap(const cv::Mat &linearBgr)
{
    if (linearBgr.empty() || linearBgr.type() != CV_32FC3)
    {
        throw std::runtime_error("Invalid linear BGR image for tone mapping.");
    }

    const float whitePoint = findMaxLuminance(linearBgr);
    const float whitePointSquared = whitePoint * whitePoint;

    cv::Mat toneMapped = linearBgr.clone();

    for (int y = 0; y < toneMapped.rows; ++y)
    {
        cv::Vec3f *row = toneMapped.ptr<cv::Vec3f>(y);

        for (int x = 0; x < toneMapped.cols; ++x)
        {
            cv::Vec3f &bgr = row[x];
            const float luminance = luminanceFromBgr(bgr);

            if (luminance <= 0.0f)
            {
                bgr = cv::Vec3f(0.0f, 0.0f, 0.0f);
                continue;
            }

            const float mappedLuminance =
                (luminance * (1.0f + luminance / whitePointSquared)) /
                (1.0f + luminance);
            const float scale = mappedLuminance / luminance;

            bgr[0] = std::clamp(bgr[0] * scale, 0.0f, 1.0f);
            bgr[1] = std::clamp(bgr[1] * scale, 0.0f, 1.0f);
            bgr[2] = std::clamp(bgr[2] * scale, 0.0f, 1.0f);
        }
    }

    return toneMapped;
}

int bayerChannelAt(int y, int x, int bayerPattern)
{
    const bool evenY = (y % 2 == 0);
    const bool evenX = (x % 2 == 0);

    switch (bayerPattern)
    {
    case 0: // RGGB -> R G / G B
        if (evenY && evenX)
        {
            return 2;
        }
        if (!evenY && !evenX)
        {
            return 0;
        }
        return 1;

    case 1: // BGGR -> B G / G R
        if (evenY && evenX)
        {
            return 0;
        }
        if (!evenY && !evenX)
        {
            return 2;
        }
        return 1;

    case 2: // GRBG -> G R / B G
        if (evenY && !evenX)
        {
            return 2;
        }
        if (!evenY && evenX)
        {
            return 0;
        }
        return 1;

    case 3: // GBRG -> G B / R G
        if (evenY && !evenX)
        {
            return 0;
        }
        if (!evenY && evenX)
        {
            return 2;
        }
        return 1;

    default:
        throw std::runtime_error("Invalid Bayer pattern for demosaicing.");
    }
}

float averageBayerChannel(const cv::Mat &wbRaw32, int y, int x,
                          int targetChannel, int bayerPattern)
{
    float sum = 0.0f;
    int count = 0;

    for (int dy = -1; dy <= 1; ++dy)
    {
        const int yy = y + dy;
        if (yy < 0 || yy >= wbRaw32.rows)
        {
            continue;
        }

        const float *row = wbRaw32.ptr<float>(yy);

        for (int dx = -1; dx <= 1; ++dx)
        {
            const int xx = x + dx;
            if (xx < 0 || xx >= wbRaw32.cols)
            {
                continue;
            }

            if (bayerChannelAt(yy, xx, bayerPattern) != targetChannel)
            {
                continue;
            }

            sum += row[xx];
            ++count;
        }
    }

    return count > 0 ? sum / static_cast<float>(count) : 0.0f;
}

cv::Mat Demosaicing(const cv::Mat &wbRaw32, int bayerPattern)
{
    if (wbRaw32.empty() || wbRaw32.type() != CV_32FC1)
    {
        throw std::runtime_error("Invalid 32F Bayer image for demosaicing.");
    }

    cv::Mat bgr32 = cv::Mat::zeros(wbRaw32.size(), CV_32FC3);

    for (int y = 0; y < wbRaw32.rows; ++y)
    {
        cv::Vec3f *dst = bgr32.ptr<cv::Vec3f>(y);

        for (int x = 0; x < wbRaw32.cols; ++x)
        {
            dst[x][0] = averageBayerChannel(wbRaw32, y, x, 0, bayerPattern);
            dst[x][1] = averageBayerChannel(wbRaw32, y, x, 1, bayerPattern);
            dst[x][2] = averageBayerChannel(wbRaw32, y, x, 2, bayerPattern);
        }
    }

    return bgr32;
}
} // namespace

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

    // TODO : Implement active area crop

    cv::Mat normalized = cv::Mat::zeros(bayer16.size(), CV_32FC1);

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

            dst[x] = v;
        }
    }

    // Demosaicing to camera BGR (device BGR).
    cv::Mat bgr32 = Demosaicing(normalized, bayerPattern);

    cv::Mat correctedBgr(bgr32.size(), CV_32FC3);

    for (int y = 0; y < bgr32.rows; ++y)
    {
        cv::Vec3f *bgrRow = bgr32.ptr<cv::Vec3f>(y);
        cv::Vec3f *correctedBgrRow = correctedBgr.ptr<cv::Vec3f>(y);

        // To Linear RGB From Device BGR
        for (int x = 0; x < bgr32.cols; ++x)
        {
            cv::Vec3f bgr = bgrRow[x];
            cv::Vec3f rgb(bgr[2], bgr[1], bgr[0]);

            cv::Vec3f corrected = rgbCam * rgb;

            corrected[0] = std::max(corrected[0], 0.0f);
            corrected[1] = std::max(corrected[1], 0.0f);
            corrected[2] = std::max(corrected[2], 0.0f);

            correctedBgrRow[x][0] = corrected[2];
            correctedBgrRow[x][1] = corrected[1];
            correctedBgrRow[x][2] = corrected[0];
        }
    }

    cv::Mat denoisedImg;

    if (denoiser < 1) // No denoise
    {
        denoisedImg = correctedBgr;
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
    else
    {
        denoisedImg = correctedBgr;
    }

    // Tone mapping before gamma correction.
    denoisedImg *= pow(2.0f, 1);
    cv::Mat toneMapped = applyExtendedReinhardToneMap(denoisedImg);

    cv::Mat gammaCorrected = toneMapped.clone();

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
    // TODO : Comparing with original data and restorated data

    return preview7;
}
