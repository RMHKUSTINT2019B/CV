//
// Created by KASHUN SHUM on 2018/10/14.
//

#include <utility>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "VideoProcessor.h"

using namespace cv;

void VideoProcessor::displayInput(const std::string& wt) {
    windowNameInput = wt;
    namedWindow(windowNameInput);
}

void VideoProcessor::displayOutput(const std::string& wn) {
    windowNameOutput = wn;
    namedWindow(windowNameOutput);
}

void VideoProcessor::dontDisplay() {
    destroyWindow(windowNameInput);
    destroyWindow(windowNameOutput);
    windowNameInput.clear();
    windowNameOutput.clear();
}

void VideoProcessor::setDelay(int d) {
    delay = d;
}

void VideoProcessor::callProcess() {
    callIt = true;
}

void VideoProcessor::dontCallProcess() {
    callIt = false;
}

void VideoProcessor::setFrameProcessor(FrameProcessor frameProcessorPtr) {
    frameProcessor = std::move(frameProcessorPtr);
    callProcess();
}

void VideoProcessor::stop() {
    shouldStop = true;
    stopNotifier.notify_all();
}

bool VideoProcessor::isReady() const noexcept { return camera ? camera->good() : false; }

long VideoProcessor::getFrameNumber() const noexcept { return camera ? camera->frameId() : 0l; }

double VideoProcessor::getFrameRate() const noexcept { return camera ? camera->frameRate() : 0.0; }

std::future<void> VideoProcessor::run() {
#ifndef NDEBUG
    if (!isReady())
        throw std::runtime_error("VideoProcessor State is Incomplete!");
    if (executing.exchange(true))
        throw std::runtime_error("A processor is already running");
#else
    executing = true;
#endif
    shouldStop = false;
    // Fork-off a thread to do actual work
    return std::async(std::launch::async, [this]() {
        Mat frame, output;
        std::mutex mt; // idle mutex
        while (!shouldStop) {
            if (!camera->getNextFrame(frame) || (frameToStop >= 0 && getFrameNumber() == frameToStop))
                break;

            if (windowNameInput.length() != 0)
                imshow(windowNameInput, frame);

            if (callIt) {
                if (frameProcessor)
                    output = frameProcessor(frame);
            } else {
                output = frame;
            }

            if (windowNameOutput.length() != 0)
                cv::imshow(windowNameOutput, output);

            // IDLE
            if (delay > 0) {
                std::unique_lock<std::mutex> lk(mt);
                stopNotifier.wait_for(lk, std::chrono::milliseconds(delay), [this]() { return shouldStop.load(); });
            }
        }
        executing = false;
    });
}

void VideoProcessor::setInput(std::shared_ptr<ICamera> newCamera) noexcept { camera = std::move(newCamera); }

bool VideoProcessor::isExecuting() const noexcept { return executing; }
