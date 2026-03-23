#include <iostream>
#include <string>

#include <opencv2/opencv.hpp>

#include "raw_loader.h"
#include "isp_pipeline.h"

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: ./lowlight_raw_app <path_to_raw_file>" << std::endl;
        return 1;
    }

    std::string rawPath = argv[1];

    try {
        RawImageData raw = RawLoader::load(rawPath);

        std::cout << "Loaded RAW file: " << rawPath << std::endl;
        std::cout << "Size: " << raw.width << " x " << raw.height << std::endl;
        std::cout << "Black level: " << raw.blackLevel << std::endl;
        std::cout << "White level: " << raw.whiteLevel << std::endl;

        cv::Mat preview = ISPPipeline::makePreview(
            raw.bayer16,
            raw.blackLevel,
            raw.whiteLevel,
            4.0,   // 저조도라 임시 gain
            2.2
        );

        cv::imshow("RAW Preview", preview);
        cv::imwrite("outputs/preview.png", preview);

        std::cout << "Saved preview to outputs/preview.png" << std::endl;
        cv::waitKey(0);
    }
    catch (const std::exception& e) {
        std::cerr << "[ERROR] " << e.what() << std::endl;
        return 1;
    }

    return 0;
}