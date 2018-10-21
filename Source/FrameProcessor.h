//
// Created by KASHUN SHUM on 2018/10/14.
//

#pragma once

#include <opencv2/core/mat.hpp>
#include <functional>

using FrameProcessor = std::function<void(const cv::Mat& input, cv::Mat& output)>;

void canny(const cv::Mat& img, cv::Mat& out);
