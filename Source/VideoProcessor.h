//
// Created by KASHUN SHUM on 2018/10/14.
//

#pragma once

#include "FrameProcessor.h"
#include "Camera.h"
#include <memory>
#include <atomic>
#include <future>
#include <condition_variable>

using namespace cv;

class VideoProcessor {
public:
    // 默认设置 digits(0)
    VideoProcessor() : callIt(false), delay(-1), shouldStop(false), frameToStop(-1) {}

    void displayInput(const std::string& wt);

    void displayOutput(const std::string& wn);

    void dontDisplay();

    void setInput(std::shared_ptr<ICamera> newCamera) noexcept;

    void setFrameProcessor(FrameProcessor frameProcessor);

    void setDelay(int d);

    void callProcess();

    void dontCallProcess();

    void stop();

    bool isExecuting() const noexcept;

    bool isReady() const noexcept;

    double getFrameRate() const noexcept;

    long getFrameNumber() const noexcept;;

    std::future<void> run();

private:
    std::shared_ptr<ICamera> camera{nullptr};

    FrameProcessor frameProcessor{nullptr};

    std::atomic_bool callIt, shouldStop, executing{false};

    std::string windowNameInput, windowNameOutput;

    int delay;

    long frameToStop;

    std::condition_variable stopNotifier;
};
