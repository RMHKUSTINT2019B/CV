//
// Created by 刘予顺 on 2018/10/15.
//

#pragma once

#include <opencv2/core/mat.hpp>
#include <opencv2/videoio.hpp>
#include <vector>
#include <string>

struct ICamera {
    virtual ~ICamera() = default;

    virtual bool good() const noexcept = 0;

    virtual long frameId() const noexcept = 0;

    virtual double frameRate() const noexcept = 0;

    virtual bool getNextFrame(cv::Mat& frame) = 0;
};

class LogicalCamera : public ICamera {
public:
    explicit LogicalCamera(int id);

    explicit LogicalCamera(const std::string& filename);

    bool good() const noexcept override;

    long frameId() const noexcept override;

    double frameRate() const noexcept override;

    bool getNextFrame(cv::Mat& frame) override;

private:
    cv::VideoCapture mVideoCapture;
};

class MockCamera : public ICamera {
public:
    explicit MockCamera(const std::vector<std::string>& imgs);

    bool good() const noexcept override;

    long frameId() const noexcept override;

    double frameRate() const noexcept override;

    bool getNextFrame(cv::Mat& frame) override;

private:
    std::vector<std::string> images;
    std::vector<std::string>::const_iterator itImg;
};
