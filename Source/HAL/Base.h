#pragma once

struct IDriver {
    virtual ~IDriver() = default;
    virtual void Close() = 0;
};

constexpr int Any = -1;
