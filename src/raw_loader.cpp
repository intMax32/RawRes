#include "raw_loader.h"

#include <algorithm>
#include <iostream>
#include <libraw/libraw.h>
#include <stdexcept>

RawImageData RawLoader::load(const std::string &path)
{
    LibRaw rawProcessor;

    int ret = rawProcessor.open_file(path.c_str());
    if (ret != LIBRAW_SUCCESS)
    {
        throw std::runtime_error("Failed to open RAW file: " + path);
    }

    ret = rawProcessor.unpack();
    if (ret != LIBRAW_SUCCESS)
    {
        throw std::runtime_error("Failed to unpack RAW file: " + path);
    }

    rawProcessor.imgdata.params.use_camera_wb = 1;

    int width = rawProcessor.imgdata.sizes.raw_width;
    int height = rawProcessor.imgdata.sizes.raw_height;

    const char *cdesc = rawProcessor.imgdata.idata.cdesc;
    int c00 = rawProcessor.COLOR(0, 0);
    int c01 = rawProcessor.COLOR(0, 1);
    int bayerPattern;

    if (cdesc[c00] == 'R')
    {
        bayerPattern = 0; // RGGB
    }
    else if (cdesc[c00] == 'B')
    {
        bayerPattern = 1; // BGGR
    }
    else
    {
        if (cdesc[c01] == 'R')
        {
            bayerPattern = 2; // GRBG
        }
        else
        {
            bayerPattern = 3; // GBRG
        }
    }

    cv::Mat bayer(height, width, CV_16UC1);

    ushort *rawData = rawProcessor.imgdata.rawdata.raw_image;
    if (!rawData)
    {
        throw std::runtime_error("RAW image buffer is null.");
    }

    for (int y = 0; y < height; ++y)
    {
        ushort *dst = bayer.ptr<ushort>(y);
        for (int x = 0; x < width; ++x)
        {
            dst[x] = rawData[y * width + x];
        }
    }

    int blackLevel = rawProcessor.imgdata.color.black;
    int whiteLevel = rawProcessor.imgdata.color.maximum;
    const float *camMul = rawProcessor.imgdata.color.cam_mul;

    float wbRed = 1.0f;
    float wbGreen = 1.0f;
    float wbBlue = 1.0f;

    if (camMul[0] > 0.0f && camMul[1] > 0.0f && camMul[2] > 0.0f)
    {
        const float greenRef = camMul[1];
        wbRed = camMul[0] / greenRef;
        wbBlue = camMul[2] / greenRef;
    }

    wbRed = std::clamp(wbRed, 0.1f, 8.0f);
    wbGreen = std::clamp(wbGreen, 0.1f, 8.0f);
    wbBlue = std::clamp(wbBlue, 0.1f, 8.0f);

    RawImageData result;
    result.bayer16 = bayer;
    result.width = width;
    result.height = height;
    result.blackLevel = blackLevel;
    result.whiteLevel = whiteLevel;
    result.bayerPattern = bayerPattern;
    result.wbRed = wbRed;
    result.wbGreen = wbGreen;
    result.wbBlue = wbBlue;

    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            result.CCM(i, j) = rawProcessor.imgdata.color.rgb_cam[i][j];
        }
    }

    rawProcessor.recycle();
    return result;
}
