#ifndef TARGET_H
#define TARGET_H

#include <opencv2/opencv.hpp>

double median(cv::InputArray);
void detect(cv::Mat&, bool = true, bool = false);

#endif