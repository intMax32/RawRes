#include <iostream>
#include <string>

#include <opencv2/opencv.hpp>

#include "isp_pipeline.h"
#include "raw_loader.h"

int main()
{
    std::string rawPath;
    std::cout << "Enter a path of a photo" << std::endl;
    std::cin >> rawPath;

    try
    {
        RawImageData raw = RawLoader::load(rawPath);

        std::cout << "Loaded RAW file: " << rawPath << std::endl;
        std::cout << "Size: " << raw.width << " x " << raw.height << std::endl;
        std::cout << "Black level: " << raw.blackLevel << std::endl;
        std::cout << "White level: " << raw.whiteLevel << std::endl;
        std::cout << "As-shot WB (R/G/B): " << raw.wbRed << " / " << raw.wbGreen
                  << " / " << raw.wbBlue << std::endl;

        switch (raw.bayerPattern)
        {
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
            raw.bayer16, raw.rgbCam, raw.blackLevel, raw.whiteLevel, 2.2,
            raw.bayerPattern, raw.wbRed, raw.wbGreen, raw.wbBlue);

        cv::imshow("RAW Preview", preview);
        cv::waitKey(0);
    }
    catch (const std::exception &e)
    {
        std::cerr << "[ERROR] " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
