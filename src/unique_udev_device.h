#pragma once

#include <libudev.h>

#include <memory>

namespace tt_helper
{

struct UdevDeviceFreer
{
    inline void operator()(udev_device* device) const noexcept
    {
        udev_device_unref(device);
    }
};

using UniqueUdevDevice = std::unique_ptr<udev_device, UdevDeviceFreer>;

} // namespace tt_helper
