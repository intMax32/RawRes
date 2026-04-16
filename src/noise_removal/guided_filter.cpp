#include "noise_removal/guided_filter.h"

namespace NoiseRemoval
{
cv::Mat GuidedFilter(int radius, cv::Mat &p, cv::Mat &I, float eps)
{
    int ICh = I.channels();
    cv::Mat output;

    if (ICh == 1) // Grayscale guide image
    {
        output = GrayGuidedFilter(radius, p, I, eps);
    }
    else if (ICh == 3) // Color guide image
    {
        output = ColorGuidedFilter(radius, p, I, eps);
    }

    return output;
}

cv::Mat GrayGuidedFilter(int radius, cv::Mat &p, cv::Mat &I, float eps)
{
    int pCh = p.channels();
    cv::Mat q;

    if (pCh == 1) // Grayscale input image
    {
        cv::Mat meanOfP;
        cv::Mat meanOfI;

        cv::Mat a;
        cv::Mat meanOfA;
        cv::Mat b;
        cv::Mat meanOfB;

        cv::boxFilter(I, meanOfI, CV_32F,
                      cv::Size(2 * radius + 1, 2 * radius + 1));
        cv::boxFilter(p, meanOfP, CV_32F,
                      cv::Size(2 * radius + 1, 2 * radius + 1));

        cv::Mat IP = I.mul(p);
        cv::Mat II = I.mul(I);

        cv::Mat corrOfIP;
        cv::Mat corrOfI;
        cv::boxFilter(IP, corrOfIP, CV_32F,
                      cv::Size(2 * radius + 1, 2 * radius + 1));

        cv::boxFilter(II, corrOfI, CV_32F,
                      cv::Size(2 * radius + 1, 2 * radius + 1));

        cv::Mat varOfI;
        cv::subtract(corrOfI, meanOfI.mul(meanOfI), varOfI);

        cv::Mat covOfIP;
        cv::subtract(corrOfIP, meanOfI.mul(meanOfP), covOfIP);

        cv::divide(covOfIP, (varOfI + eps), a);
        cv::subtract(meanOfP, a.mul(meanOfI), b);

        cv::boxFilter(a, meanOfA, CV_32F,
                      cv::Size(2 * radius + 1, 2 * radius + 1));
        cv::boxFilter(b, meanOfB, CV_32F,
                      cv::Size(2 * radius + 1, 2 * radius + 1));

        cv::multiply(meanOfA, I, q);
        q += b;
    }

    else if (pCh == 3) // Color input image
    {
        std::vector<cv::Mat> pChannels;
        cv::split(p, pChannels);

        cv::Mat pBlue = pChannels[0];
        cv::Mat pGreen = pChannels[1];
        cv::Mat pRed = pChannels[2];

        cv::Mat meanOfPBlue;
        cv::Mat meanOfPGreen;
        cv::Mat meanOfPRed;
        cv::Mat meanOfI;

        cv::Mat aBlue;
        cv::Mat aGreen;
        cv::Mat aRed;
        cv::Mat meanOfABlue;
        cv::Mat meanOfAGreen;
        cv::Mat meanOfARed;
        cv::Mat bBlue;
        cv::Mat bGreen;
        cv::Mat bRed;
        cv::Mat meanOfBBlue;
        cv::Mat meanOfBGreen;
        cv::Mat meanOfBRed;

        cv::boxFilter(I, meanOfI, CV_32F,
                      cv::Size(2 * radius + 1, 2 * radius + 1));
        cv::boxFilter(pBlue, meanOfPBlue, CV_32F,
                      cv::Size(2 * radius + 1, 2 * radius + 1));
        cv::boxFilter(pGreen, meanOfPGreen, CV_32F,
                      cv::Size(2 * radius + 1, 2 * radius + 1));
        cv::boxFilter(pRed, meanOfPRed, CV_32F,
                      cv::Size(2 * radius + 1, 2 * radius + 1));

        cv::Mat IPBlue = I.mul(pBlue);
        cv::Mat IPGreen = I.mul(pGreen);
        cv::Mat IPRed = I.mul(pRed);

        cv::Mat II = I.mul(I);

        cv::Mat corrOfIPBlue;
        cv::Mat corrOfIPGreen;
        cv::Mat corrOfIPRed;
        cv::Mat corrOfI;

        cv::boxFilter(IPBlue, corrOfIPBlue, CV_32F,
                      cv::Size(2 * radius + 1, 2 * radius + 1));

        cv::boxFilter(IPGreen, corrOfIPGreen, CV_32F,
                      cv::Size(2 * radius + 1, 2 * radius + 1));

        cv::boxFilter(IPRed, corrOfIPRed, CV_32F,
                      cv::Size(2 * radius + 1, 2 * radius + 1));

        cv::boxFilter(II, corrOfI, CV_32F,
                      cv::Size(2 * radius + 1, 2 * radius + 1));

        cv::Mat varOfI;
        cv::subtract(corrOfI, meanOfI.mul(meanOfI), varOfI);

        cv::Mat covOfIPBlue;
        cv::Mat covOfIPGreen;
        cv::Mat covOfIPRed;
        cv::subtract(corrOfIPBlue, meanOfI.mul(meanOfPBlue), covOfIPBlue);
        cv::subtract(corrOfIPGreen, meanOfI.mul(meanOfPGreen), covOfIPGreen);
        cv::subtract(corrOfIPRed, meanOfI.mul(meanOfPRed), covOfIPRed);

        cv::divide(covOfIPBlue, (varOfI + eps), aBlue);
        cv::divide(covOfIPGreen, (varOfI + eps), aGreen);
        cv::divide(covOfIPRed, (varOfI + eps), aRed);
        cv::subtract(meanOfPBlue, aBlue.mul(meanOfI), bBlue);
        cv::subtract(meanOfPGreen, aGreen.mul(meanOfI), bGreen);
        cv::subtract(meanOfPRed, aRed.mul(meanOfI), bRed);

        cv::boxFilter(aBlue, meanOfABlue, CV_32F,
                      cv::Size(2 * radius + 1, 2 * radius + 1));
        cv::boxFilter(bBlue, meanOfBBlue, CV_32F,
                      cv::Size(2 * radius + 1, 2 * radius + 1));

        cv::boxFilter(aGreen, meanOfAGreen, CV_32F,
                      cv::Size(2 * radius + 1, 2 * radius + 1));
        cv::boxFilter(bGreen, meanOfBGreen, CV_32F,
                      cv::Size(2 * radius + 1, 2 * radius + 1));

        cv::boxFilter(aRed, meanOfARed, CV_32F,
                      cv::Size(2 * radius + 1, 2 * radius + 1));
        cv::boxFilter(bRed, meanOfBRed, CV_32F,
                      cv::Size(2 * radius + 1, 2 * radius + 1));

        cv::Mat qBlue;
        cv::Mat qGreen;
        cv::Mat qRed;
        cv::multiply(meanOfABlue, I, qBlue);
        cv::multiply(meanOfAGreen, I, qGreen);
        cv::multiply(meanOfARed, I, qRed);
        qBlue += meanOfBBlue;
        qGreen += meanOfBGreen;
        qRed += meanOfBRed;
        std::vector<cv::Mat> qChannels = {qBlue, qGreen, qRed};
        cv::merge(qChannels, q);
    }

    return q;
}

cv::Mat ColorGuidedFilter(int radius, cv::Mat &p, cv::Mat &I, float eps)
{

    cv::Mat meanOfP;
    cv::Mat meanOfI;

    cv::Mat a;
    cv::Mat meanOfA;
    cv::Mat b;
    cv::Mat meanOfB;

    cv::boxFilter(I, meanOfI, CV_32F, cv::Size(2 * radius + 1, 2 * radius + 1));
    cv::boxFilter(p, meanOfP, CV_32F, cv::Size(2 * radius + 1, 2 * radius + 1));

    cv::Mat IP = I.mul(p);
    cv::Mat II = I.mul(I);

    cv::Mat corrOfIP;
    cv::Mat corrOfI;
    cv::boxFilter(IP, corrOfIP, CV_32F,
                  cv::Size(2 * radius + 1, 2 * radius + 1));

    cv::boxFilter(II, corrOfI, CV_32F,
                  cv::Size(2 * radius + 1, 2 * radius + 1));

    cv::Mat covMatOfI;
    cv::subtract(corrOfI, meanOfI.mul(meanOfI), covMatOfI);

    cv::Mat covOfIP;
    cv::subtract(corrOfIP, meanOfI.mul(meanOfP), covOfIP);

    cv::divide(covOfIP, (covMatOfI + eps), a);
    cv::subtract(meanOfP, a.mul(meanOfI), b);

    cv::boxFilter(a, meanOfA, CV_32F, cv::Size(2 * radius + 1, 2 * radius + 1));
    cv::boxFilter(b, meanOfB, CV_32F, cv::Size(2 * radius + 1, 2 * radius + 1));

    cv::Mat q;

    cv::multiply(meanOfA, I, q);
    q += b;

    return q;
}
} // namespace NoiseRemoval
