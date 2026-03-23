#include "raw_loader.h"

#include <iostream>
#include <stdexcept>
#include <libraw/libraw.h>

RawImageData RawLoader::load(const std::string& path) {
    LibRaw rawProcessor;

    int ret = rawProcessor.open_file(path.c_str());
    if (ret != LIBRAW_SUCCESS) {
        throw std::runtime_error("Failed to open RAW file: " + path);
    }

    ret = rawProcessor.unpack();
    if (ret != LIBRAW_SUCCESS) {
        throw std::runtime_error("Failed to unpack RAW file: " + path);
    }

    int width = rawProcessor.imgdata.sizes.raw_width;
    int height = rawProcessor.imgdata.sizes.raw_height;

    cv::Mat bayer(height, width, CV_16UC1);

    ushort* rawData = rawProcessor.imgdata.rawdata.raw_image;
    if (!rawData) {
        throw std::runtime_error("RAW image buffer is null.");
    }

    for (int y = 0; y < height; ++y) {
        ushort* dst = bayer.ptr<ushort>(y);
        for (int x = 0; x < width; ++x) {
            dst[x] = rawData[y * width + x];
        }
    }

    int blackLevel = rawProcessor.imgdata.color.black;
    int whiteLevel = rawProcessor.imgdata.color.maximum;

    RawImageData result;
    result.bayer16 = bayer;
    result.width = width;
    result.height = height;
    result.blackLevel = blackLevel;
    result.whiteLevel = whiteLevel;

    rawProcessor.recycle();
    return result;
}