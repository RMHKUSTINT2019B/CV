//
// Created by KASHUN SHUM on 2018/10/14.
//

#pragma once

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "FrameProcessor.h"
#include <string>

using namespace cv;

class VideoProcessor {
private:
    VideoCapture capture;

    FrameProcessor frameProcessor{nullptr};

    bool callIt;

    std::string windowNameInput;

    std::string windowNameOutput;

    int delay;

    long fnumber;

    long frameToStop{};

    bool stop;

    std::vector<std::string> images;
    std::vector<std::string>::const_iterator itImg;

    bool readNextFrame(Mat &frame) {
        if (images.empty())
            return capture.read(frame);
        else {
            if (itImg != images.end()) {
                frame = imread(*itImg);
                itImg++;
                return frame.data != nullptr;
            }
        }

    }

public:
    // 默认设置 digits(0), frameToStop(-1),
    VideoProcessor() : callIt(false), delay(-1), fnumber(0), stop(false) {}


    void displayInput(const std::string &wt);

    void displayOutput(const std::string &wn);

    void dontDisplay();


    bool setInput(const std::string &filename);

    bool setInput(int id);

    bool setInput(const std::vector<std::string> &imgs);


    void setDelay(int d);


    double getFrameRate();


    void callProcess();


    void dontCallProcess();


    void setFrameProcessor(FrameProcessor frameProcessor);


    void stopIt();


    bool isStopped();


    bool isOpened();


    long getFrameNumber();


    void run();

};
