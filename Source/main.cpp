#include <string>
#include <iomanip>
#include <sstream>
#include <thread>

#include "VideoProcessor.h"
#include "FrameProcessor.h"

int main() {
    VideoProcessor processor;
    // 打开摄像头
    processor.setInput(std::make_shared<LogicalCamera>(0));

    // 分别为输入和输出
    processor.displayInput("Input ");
    processor.displayOutput("Output ");

    processor.setDelay(1000. / processor.getFrameRate());

    processor.setFrameProcessor(canny);
    // 开始帧处理过程
    auto task = processor.run();
    waitKey();
    processor.stop();
    task.wait();
    //TODO 加入识别function
    return 0;
}
