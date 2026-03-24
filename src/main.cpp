#include <iostream>
#include <string>
#include <algorithm>

#include <opencv2/opencv.hpp>

#include "raw_loader.h"
#include "isp_pipeline.h"

namespace {
RawImageData g_raw;
int g_redSlider = 100;
int g_greenSlider = 100;
int g_blueSlider = 100;

void updatePreview(int, void*) {
    float redGain = g_redSlider / 100.0f;
    float greenGain = g_greenSlider / 100.0f;
    float blueGain = g_blueSlider / 100.0f;

    cv::Mat preview = ISPPipeline::makePreview(
        g_raw.bayer16,
        g_raw.blackLevel,
        g_raw.whiteLevel,
        4.0,
        2.2,
        g_raw.bayerPattern,
        redGain,
        greenGain,
        blueGain
    );

    cv::imshow("RAW Preview", preview);
}
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: ./RawRes <path_to_raw_file>" << std::endl;
        return 1;
    }

    std::string rawPath = argv[1];

    try {
        g_raw = RawLoader::load(rawPath);
        g_redSlider = std::clamp(static_cast<int>(g_raw.wbRed * 100.0f), 1, 400);
        g_greenSlider = std::clamp(static_cast<int>(g_raw.wbGreen * 100.0f), 1, 400);
        g_blueSlider = std::clamp(static_cast<int>(g_raw.wbBlue * 100.0f), 1, 400);

        std::cout << "Loaded RAW file: " << rawPath << std::endl;
        std::cout << "Size: " << g_raw.width << " x " << g_raw.height << std::endl;
        std::cout << "Black level: " << g_raw.blackLevel << std::endl;
        std::cout << "White level: " << g_raw.whiteLevel << std::endl;
        std::cout << "As-shot WB (R/G/B): "
                  << g_raw.wbRed << " / "
                  << g_raw.wbGreen << " / "
                  << g_raw.wbBlue << std::endl;

        switch (g_raw.bayerPattern) {
            case 0:
                std::cout << "Bayer pattern : RGGB" << std::endl;
                break;
            case 1:
                std::cout << "Bayer pattern : BGGR" << std::endl;
                break;
            case 2:
                std::cout << "Bayer pattern : GRBG" << std::endl;
                break;
            case 3:
                std::cout << "Bayer pattern : GBRG" << std::endl;
                break;
            default:
                std::cout << "Bayer pattern : Invalid" << std::endl;
                break;
        }

        cv::namedWindow("RAW Preview", cv::WINDOW_AUTOSIZE);

        cv::createTrackbar("R x100", "RAW Preview", &g_redSlider, 400, updatePreview);
        cv::createTrackbar("G x100", "RAW Preview", &g_greenSlider, 400, updatePreview);
        cv::createTrackbar("B x100", "RAW Preview", &g_blueSlider, 400, updatePreview);

        updatePreview(0, nullptr);
        cv::waitKey(0);
    }
    catch (const std::exception& e) {
        std::cerr << "[ERROR] " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
