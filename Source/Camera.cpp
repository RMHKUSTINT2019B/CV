//
// Created by 刘予顺 on 2018/10/15.
//

#include "Camera.h"
#include <opencv2/imgcodecs.hpp>

namespace {
    [[noreturn]] void throwCameraOpenFailure() { throw std::runtime_error("Cannot Open Specified Camera"); }
}

LogicalCamera::LogicalCamera(int id) : mVideoCapture(id) {
    if (!mVideoCapture.isOpened())
        throwCameraOpenFailure();
}

LogicalCamera::LogicalCamera(const std::string& filename) : mVideoCapture(filename) {
    if (!mVideoCapture.isOpened())
        throwCameraOpenFailure();
}

bool LogicalCamera::getNextFrame(cv::Mat& frame) { return mVideoCapture.read(frame); }

bool LogicalCamera::good() const noexcept { return mVideoCapture.isOpened(); }

long LogicalCamera::frameId() const noexcept { return static_cast<long>(mVideoCapture.get(cv::CAP_PROP_POS_FRAMES)); }

double LogicalCamera::frameRate() const noexcept { return mVideoCapture.get(cv::CAP_PROP_FPS); }

MockCamera::MockCamera(const std::vector<std::string>& imgs) : images(imgs) { itImg = images.begin(); }

bool MockCamera::getNextFrame(cv::Mat& frame) {
    if (itImg != images.end()) {
        frame = cv::imread(*itImg);
        itImg++;
        return frame.data != nullptr;
    }
    return false;
}

bool MockCamera::good() const noexcept { return !images.empty(); }

long MockCamera::frameId() const noexcept { return static_cast<long>(itImg - images.begin()); }

double MockCamera::frameRate() const noexcept { return 0.0; }
