//
// Created by 刘予顺 on 2018/10/20.
//

#include "../HAL/SerialBus.h"
#include "../HAL/USB.h"
#include <array>
#include <cstdint>
#include <utility>
#include <list>
#include <iostream>
#include <algorithm>

#define EP_DATA_IN        (0x2|LIBUSB_ENDPOINT_IN)
#define EP_DATA_OUT       (0x2|LIBUSB_ENDPOINT_OUT)
#define CTRL_IN           (LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_IN)
#define CTRL_OUT          (LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_OUT)

namespace {
    class CH341 : public IUARTBus {
    public:
        explicit CH341(libusb_device* dev);

        void Configure(const UARTConfig& cfg) override;

        void Read(void* buffer, int size) const override;

        void Write(const void* buffer, int size) override;

        std::future<void> ReadAsync(void* buffer, int size) const override;

        std::future<void> WriteAsync(const void* buffer, int size) override;

        void Close() override;
    private:
        void DevInit();

        void StartInputListener();

        int ControlTransfer(int requestType, int request, int value, int index, unsigned char* buffer, int length) {
            if (auto s = libusb_control_transfer(devh, requestType, request, value, index, buffer, length, 500); s<0)
                throw USBError(s);
            else
                return s;
        }

        void ControlTransferOut(int request, int value, int index) {
            ControlTransfer(CTRL_OUT, request, value, index, nullptr, 0);
        }

        void ControlTransferIn(int request, int value, int index, unsigned char* data, int length) {
            if (ControlTransfer(CTRL_IN, request, value, index, data, length)!=length)
                throw std::runtime_error("Got too few bytes for current connection");
        }

        struct libusb_device_handle* devh = nullptr;
        struct libusb_transfer* recv_bulk_transfer = nullptr;
        uint8_t recvbuf[4096];
        uint8_t _StoreBuffer[1 << 16];
        mutable int _BufferEnd = 0, _BufferBegin = 0, _BufferSize = 0;

        struct _RdT {
            unsigned char* buffer;
            int size;
            int current = 0;
            std::promise<void> promise{};
            bool Satisfy() { return size==current; }
        };

        mutable std::mutex _ReadTaskLock, _CircularBufferLock;
        mutable std::list<_RdT> _PendingReadTasks;

        void ClearReadTaskWithError(int error) {
            std::lock_guard<std::mutex> lk{_ReadTaskLock};
            for (auto& x : _PendingReadTasks)
                x.promise.set_exception(std::make_exception_ptr(USBError(error)));
            _PendingReadTasks.clear();
        }

        static void LIBUSB_CALL cb_img(struct libusb_transfer* transfer);

        void ProcessRecvBuffer(int received) {
            auto processed = 0;
            {
                std::lock_guard<std::mutex> lk{_ReadTaskLock};
                if (!_PendingReadTasks.empty()) {
                    for (auto& x : _PendingReadTasks) {
                        while (processed<received && !x.Satisfy())
                            x.buffer[x.current++] = recvbuf[processed++];
                        if (processed==received) break;
                    }
                    while (_PendingReadTasks.front().Satisfy()) {
                        _PendingReadTasks.front().promise.set_value();
                        _PendingReadTasks.pop_front();
                        if (_PendingReadTasks.empty()) break;
                    }
                }
            }
            {
                std::lock_guard<std::mutex> lk{_CircularBufferLock};
                while (processed<received) {
                    _StoreBuffer[(_BufferEnd++) & ((1 << 16)-1)] = recvbuf[processed++];
                    ++(_BufferSize);
                }
            }
        }

    };

    void LIBUSB_CALL CH341::cb_img(struct libusb_transfer* transfer) {
        auto _this = reinterpret_cast<CH341*>(transfer->user_data);
        if (transfer->status==LIBUSB_TRANSFER_COMPLETED) {
            _this->ProcessRecvBuffer(transfer->actual_length);
        }
        else {
            _this->ClearReadTaskWithError(transfer->status);
            libusb_free_transfer(transfer);
        }
        libusb_submit_transfer(_this->recv_bulk_transfer);
    }

    void CH341::Read(void* buffer, int size) const { ReadAsync(buffer, size).get(); }

    void CH341::Write(const void* buffer, int size) { WriteAsync(buffer, size).get(); }

    std::future<void> CH341::ReadAsync(void* buffer, int size) const {
        _RdT record{};
        record.size = size;
        record.buffer = reinterpret_cast<uint8_t*>(buffer);
        auto fut = record.promise.get_future();
        {
            std::lock_guard<std::mutex> lk{_CircularBufferLock};
            while (_BufferSize>0 && !record.Satisfy()) {
                record.buffer[record.current++] = _StoreBuffer[_BufferBegin++];
                --_BufferSize;
            }
        }
        if (!record.Satisfy()) {
            std::lock_guard<std::mutex> lk{_ReadTaskLock};
            _PendingReadTasks.push_back(std::move(record));
        }
        else {
            record.promise.set_value();
        }
        return fut;
    }

    std::future<void> CH341::WriteAsync(const void* buffer, int size) {
        auto data = reinterpret_cast<uint8_t*>(const_cast<void*>(buffer));
        libusb_transfer* send_bulk_transfer = libusb_alloc_transfer(0);
        if (!send_bulk_transfer) throw std::bad_alloc();
        auto promise = new std::promise<void>();
        try {
            auto fut = promise->get_future();
            libusb_fill_bulk_transfer(send_bulk_transfer, devh, EP_DATA_OUT, data, size,
                    [](libusb_transfer* transfer) {
                        auto promise = reinterpret_cast<std::promise<void>*>(transfer->user_data);
                        promise->set_value();
                        delete promise;
                        libusb_free_transfer(transfer);
                    }, promise, 0);
            if (auto s = libusb_submit_transfer(send_bulk_transfer); s<0) {
                libusb_free_transfer(send_bulk_transfer);
                throw USBError(s);
            }
            return fut;
        }
        catch (...) {
            delete promise;
            throw;
        }
    }

    void CH341::Close() {
        if (recv_bulk_transfer) libusb_cancel_transfer(recv_bulk_transfer);
        libusb_free_transfer(recv_bulk_transfer);
        libusb_release_interface(devh, 0);
        libusb_close(devh);
    }

    constexpr int _ComposeBaudRateIndex(int low, int high) noexcept { return (0x88 | low) | high << 8; }

    constexpr std::array<std::pair<int, int>, 23> Rates{{
            {50, _ComposeBaudRateIndex(0, 22)}, {75, _ComposeBaudRateIndex(0, 100)},
            {110, _ComposeBaudRateIndex(0, 150)}, {135, _ComposeBaudRateIndex(0, 169)},
            {150, _ComposeBaudRateIndex(0, 178)}, {300, _ComposeBaudRateIndex(0, 217)},
            {600, _ComposeBaudRateIndex(1, 100)}, {1200, _ComposeBaudRateIndex(1, 178)},
            {1800, _ComposeBaudRateIndex(1, 204)}, {2400, _ComposeBaudRateIndex(1, 217)},
            {4800, _ComposeBaudRateIndex(2, 100)}, {9600, _ComposeBaudRateIndex(2, 178)},
            {19200, _ComposeBaudRateIndex(2, 217)}, {38400, _ComposeBaudRateIndex(3, 100)},
            {57600, _ComposeBaudRateIndex(3, 152)}, {115200, _ComposeBaudRateIndex(3, 204)},
            {230400, _ComposeBaudRateIndex(3, 230)}, {460800, _ComposeBaudRateIndex(3, 243)},
            {500000, _ComposeBaudRateIndex(3, 244)}, {921600, _ComposeBaudRateIndex(7, 243)},
            {1000000, _ComposeBaudRateIndex(3, 250)}, {2000000, _ComposeBaudRateIndex(3, 253)},
            {3000000, _ComposeBaudRateIndex(3, 254)}
    }};

    constexpr int ParityBits[] = {0, 8, 24, 40, 56};
    template <class T = void>
    [[noreturn]] T RangeCheckError() { throw std::runtime_error("Range Check Error"); }

    void RangeCheck(int target, int first, int last) { if (target<first || target>last) RangeCheckError(); }

    void CH341::Configure(const UARTConfig& cfg) {
        // Set Baud Rate
        auto iter = std::lower_bound(Rates.begin(), Rates.end(), cfg.BaudRate,
                [](const auto& l, const auto& r) noexcept { return l.first<r; });
        auto _BaudRateBits = iter!=Rates.end() ? iter->second : RangeCheckError<int>();

        // Set Data Bits
        RangeCheck(cfg.DataBits, 5, 8);
        auto _DataBits = cfg.DataBits-5;

        // Set Stop Bits
        RangeCheck(cfg.StopBits, 1, 2);
        auto _StopBits = cfg.StopBits==2 ? 4 : 0;

        // Set Parity Bits
        auto idx = static_cast<int>(cfg.ParityBit);
        RangeCheck(idx, 0, 4);
        auto _ParityBits = ParityBits[idx];

        // Upload
        ControlTransferOut(161, 0x9C | (_ParityBits | _StopBits | _DataBits | 0xC0) << 8, _BaudRateBits);
        if (cfg.FlowControlEnable) ControlTransferOut(164, -97, 0);
    }

    CH341::CH341(libusb_device* dev) {
        try {
            if (auto s = libusb_open(dev, &devh); s<0) throw USBError(s);
            if (auto s = libusb_claim_interface(devh, 0); s<0) throw USBError(s);
            DevInit();
            StartInputListener();
        }
        catch (...) {
            libusb_release_interface(devh, 0);
            libusb_close(devh);
        }
    }

    void CH341::DevInit() {
        unsigned char arrayOfByte[8];
        ControlTransferOut(161, 0, 0);
        ControlTransferIn(95, 0, 0, arrayOfByte, 2);
        ControlTransferOut(154, 4882, 55682);
        ControlTransferOut(154, 3884, 4);
        ControlTransferIn(149, 9496, 0, arrayOfByte, 2);
        ControlTransferOut(154, 10023, 0);
        ControlTransferOut(164, 255, 0);
    }

    void CH341::StartInputListener() {
        recv_bulk_transfer = libusb_alloc_transfer(0);
        if (!recv_bulk_transfer) throw std::bad_alloc();
        libusb_fill_bulk_transfer(recv_bulk_transfer, devh, EP_DATA_IN, recvbuf, sizeof(recvbuf), cb_img, this, 0);
        if (auto s = libusb_submit_transfer(recv_bulk_transfer); s<0) {
            libusb_free_transfer(recv_bulk_transfer);
            recv_bulk_transfer = nullptr;
            throw USBError(s);
        }
    }

    IDriver* Create(libusb_device* dev) { return new CH341(dev); }

    auto R0 = Install(0x1a86, 0x7523, Create);
    auto R1 = Install(0x1a86, 0x5523, Create);
    auto R3 = Install(0x1a86, 0x5512, Create);
}