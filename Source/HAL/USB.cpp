//
// Created by 刘予顺 on 2018/10/21.
//

#include "USB.h"
#include <map>
#include <thread>

namespace {
    auto& Map() {
        static std::map<uint64_t, IDriver* (*)(libusb_device*)> _DevMap {};
        return _DevMap;
    }

    struct _A{
        _A() {
            libusb_init(nullptr);
            _e = std::thread([this](){
                while (!_exit)
                    libusb_handle_events_completed(nullptr, nullptr);
            });
        }
        ~_A() {
            _exit = true;
            libusb_interrupt_event_handler(nullptr);
            if (_e.joinable())
                _e.join();
            libusb_exit(nullptr);
        }
        std::thread _e;
        bool _exit = false;
    } _AA;
}

std::shared_ptr<IDriver> Open(struct libusb_device* device) {
    libusb_device_descriptor desc {};
    libusb_get_device_descriptor(device, &desc);
    return std::shared_ptr<IDriver>(Map()[static_cast<uint64_t>(desc.idVendor) << 32 | desc.idProduct](device));
}

_USBDvInsRec Install(int vendor, int product, IDriver* (* loader)(libusb_device*)) {
    Map()[static_cast<uint64_t>(vendor) << 32 | product] = loader;
    return _USBDvInsRec();
}

const char* USBError::what() const noexcept { return libusb_error_name(_ErrorNo); }
