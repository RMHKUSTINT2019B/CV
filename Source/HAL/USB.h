#pragma once

#include "Base.h"
#include <libusb.h>
#include <memory>

struct _USBDvInsRec {};

_USBDvInsRec Install(int vendor, int product, IDriver* (*loader)(libusb_device*));

std::shared_ptr<IDriver> Open(struct libusb_device* device);

template<class T>
std::shared_ptr<T> OpenAs(libusb_device* device) { return std::dynamic_pointer_cast<T>(Open(device)); }