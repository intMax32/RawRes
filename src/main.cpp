#include <iostream>
#include <string>
#include <algorithm>

#include <opencv2/opencv.hpp>

#include "raw_loader.h"
#include "isp_pipeline.h"

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: ./RawRes <path_to_raw_file>" << std::endl;
        return 1;
    }

    std::string rawPath = argv[1];

    try {
        RawImageData raw = RawLoader::load(rawPath);

        std::cout << "Loaded RAW file: " << rawPath << std::endl;
        std::cout << "Size: " << raw.width << " x " << raw.height << std::endl;
        std::cout << "Black level: " << raw.blackLevel << std::endl;
        std::cout << "White level: " << raw.whiteLevel << std::endl;
        std::cout << "As-shot WB (R/G/B): "
                  << raw.wbRed << " / "
                  << raw.wbGreen << " / "
                  << raw.wbBlue << std::endl;

        switch (raw.bayerPattern) {
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

        cv::Mat preview = ISPPipeline::makePreview(
            raw.bayer16,
            raw.blackLevel,
            raw.whiteLevel,
            2.2,
            raw.bayerPattern,
            raw.wbRed,
            raw.wbGreen,
            raw.wbBlue
        );

        cv::imshow("RAW Preview", preview);
        cv::waitKey(0);
    }
    catch (const std::exception& e) {
        std::cerr << "[ERROR] " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
