#include "noise_removal/bm3d.h"

#include <stdexcept>

namespace NoiseRemoval
{
cv::Mat BM3D(cv::Mat &image)
{
    (void)image;
    throw std::runtime_error("BM3D is not implemented yet.");
}
} // namespace NoiseRemoval

