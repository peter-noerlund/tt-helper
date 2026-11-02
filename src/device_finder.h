#pragma once

#include "board.h"

#ifdef HAVE_LIBUDEV

#include "unique_udev_device.h"

#include <libudev.h>

#endif // HAVE_LIBUDEV

#include <cstdint>
#include <string>

namespace tt_helper
{

class DeviceFinder
{
public:
    struct DevicePath
    {
        std::string ttyPath;
        std::string mountPath;
    };

    static DevicePath findDevice(Board board);
    static DevicePath findDevice(std::uint16_t vendorId, std::uint16_t productId);

private:
#ifdef HAVE_LIBUDEV
    static UniqueUdevDevice findUsbDevice(udev* udev, std::uint16_t vendorId, std::uint16_t productId);
    static UniqueUdevDevice findChildDevice(udev_device* parentDevice, const std::string& subsystem);
    static std::string findTtyPath(udev_device* parentDevice);
    static std::string findMountPath(udev_device* parentDevice);
#endif // HAVE_LIBUDEV
};

} // namespace tt_helper
