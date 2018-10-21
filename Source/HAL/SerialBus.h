#pragma once

#include "Base.h"
#include <future>

struct ISerialBus : IDriver {
    virtual void Read(void* buffer, int size) const = 0;
    virtual void Write(const void* buffer, int size) = 0;
    virtual std::future<void> ReadAsync(void* buffer, int size) const = 0;
    virtual std::future<void> WriteAsync(const void* buffer, int size) = 0;
};

struct UARTConfig {
    enum class Parity : int {
        None = 0, Add = 1, Even = 2, Mark = 3, Space = 4
    };
    int BaudRate = 9600;
    int DataBits = 8;
    int StopBits = 1;
    Parity ParityBit = Parity::None;
    bool FlowControlEnable = false;
};

struct IUARTBus : ISerialBus {
    virtual void Configure(const UARTConfig& cfg) = 0;
};
