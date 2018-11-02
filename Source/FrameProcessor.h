//
// Created by KASHUN SHUM on 2018/10/14.
//

#pragma once

#include <opencv2/core/mat.hpp>
#include <functional>

using FrameProcessor = std::function<cv::Mat (const cv::Mat& input)>;
cv::Mat canny(const cv::Mat& img);
