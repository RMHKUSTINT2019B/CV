//
// Created by KASHUN SHUM on 2018/10/14.
//

#include "VideoProcessor.h"
#include <iostream>

using namespace cv;

void VideoProcessor::displayInput(const std::string &wt) {
    windowNameInput = wt;
    namedWindow(windowNameInput);
}

void VideoProcessor::displayOutput(const std::string &wn) {
    windowNameOutput = wn;
    namedWindow(windowNameOutput);
}

void VideoProcessor::dontDisplay() {
    destroyWindow(windowNameInput);
    destroyWindow(windowNameOutput);
    windowNameInput.clear();
    windowNameOutput.clear();
}

bool VideoProcessor::setInput(const std::string &filename) {
    fnumber = 0;

    capture.release();
    images.clear();

    return capture.open(filename);
}

bool VideoProcessor::setInput(int id) {
    fnumber = 0;

    capture.release();
    images.clear();

    return capture.open(id);
}


bool VideoProcessor::setInput(const std::vector<std::string> &imgs) {
    fnumber = 0;

    capture.release();

    images = imgs;
    itImg = images.begin();

    return true;
}

void VideoProcessor::setDelay(int d) {
    delay = d;
}


double VideoProcessor::getFrameRate() {
    if (!images.empty()) return 0;
    double r = capture.get(CV_CAP_PROP_FPS);
    return r;
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

void VideoProcessor::stopIt() {
    stop = true;
}

bool VideoProcessor::isStopped() {
    return stop;
}

bool VideoProcessor::isOpened() {
    return capture.isOpened() || !images.empty();
}

long VideoProcessor::getFrameNumber() {
    return images.empty() ? static_cast<long>(capture.get(CV_CAP_PROP_POS_FRAMES)) : static_cast<long>(itImg -
                                                                                                       images.begin());
}


void VideoProcessor::run() {

    Mat frame;

    Mat output;


    if (!isOpened()) {
        std::cout << "fail" << std::endl;
        return;
    }
    stop = false;
    while (!isStopped()) {

        if (!readNextFrame(frame))
            break;

        if (windowNameInput.length() != 0)
            imshow(windowNameInput, frame);

        if (callIt) {
            if (frameProcessor)
                frameProcessor(frame, output);
            fnumber++;
        } else {
            output = frame;
        }

        if (windowNameOutput.length() != 0)
            cv::imshow(windowNameOutput, output);

        if (delay >= 0 && cv::waitKey(delay) >= 0)
            stopIt();

        if (frameToStop >= 0 && getFrameNumber() == frameToStop)
            stopIt();
    }
}
