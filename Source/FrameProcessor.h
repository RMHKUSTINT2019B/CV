//
// Created by KASHUN SHUM on 2018/10/14.
//

#pragma once

#include <opencv2/core/mat.hpp>
#include <functional>

using FrameProcessor = std::function<void(cv::Mat &input, cv::Mat &output)>;
void canny(cv::Mat &img, cv::Mat &out);
