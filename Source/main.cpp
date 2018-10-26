#include <string>
#include <iomanip>
#include <sstream>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include "VideoProcessor.h"
#include "FrameProcessor.h"
#include "HAL/USB.h"
#include "HAL/SerialBus.h"

bool is_interesting(libusb_device* pDevice);
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

    /*try {
        libusb_device** list;
        libusb_device* found = NULL;
        ssize_t cnt = libusb_get_device_list(NULL, &list);
        ssize_t i = 0;
        int err = 0;
        for (i = 0; i<cnt; i++) {
            libusb_device* device = list[i];
            if (is_interesting(device)) {
                found = device;
                break;
            }
        }
        if (found) {
            auto bus = OpenAs<IUARTBus>(found);
            UARTConfig cfg;
            cfg.ParityBit = UARTConfig::Parity::Even;
            bus->Configure(cfg);
            while(true) {
                char ch;
                bus->Read(&ch, 1);
                putchar(ch);
                fflush(stdout);
            }
            bus->Close();
        }
        libusb_free_device_list(list, 1);
    }
    catch (std::exception& exp) {
        std::cout << exp.what() << std::endl;
    }
    return 0;*/
}
bool is_interesting(libusb_device* pDevice) {
    libusb_device_descriptor desc {};
    libusb_get_device_descriptor(pDevice, &desc);
    return desc.idVendor == 0x1a86 && desc.idProduct == 0x7523;
}
