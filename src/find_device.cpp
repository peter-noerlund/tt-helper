#include "find_device.h"

#include "from_hex.h"

#ifdef HAVE_LIBUDEV

#include "unique_udev.h"
#include "unique_udev_device.h"
#include "unique_udev_enumerate.h"

#include <libudev.h>

#endif // HAVE_LIBUDEV

#include <stdexcept>

namespace tt_helper
{

std::string findDevice(Board board)
{
    switch (board)
    {
        case Board::PicoIce:
            return findDevice(0x1209, 0xB1C0);

        case Board::TinyTapeoutEtr:
            return findDevice(0x2E8A, 0x0005);

        case Board::TinyTapeout06:
            throw std::invalid_argument("Auto detection is not supported for Tiny Tapeout 06+");

        case Board::Unknown:
            break;
    }
    
    throw std::invalid_argument("Unknown board");
}

std::string findDevice(std::uint16_t vendorId, std::uint16_t productId)
{
#ifdef HAVE_LIBUDEV

    UniqueUdev udev(udev_new());
    UniqueUdevEnumerate udevEnum(udev_enumerate_new(udev.get()));
    udev_enumerate_add_match_subsystem(udevEnum.get(), "tty");
    udev_enumerate_scan_devices(udevEnum.get());

    auto ttyDevices = udev_enumerate_get_list_entry(udevEnum.get());
    decltype(ttyDevices) ttyDevice;
    udev_list_entry_foreach(ttyDevice, ttyDevices)
    {
        UniqueUdevDevice device(udev_device_new_from_syspath(udev.get(), udev_list_entry_get_name(ttyDevice)));
        if (!device)
        {
            continue;
        }

        auto usbDevice = udev_device_get_parent_with_subsystem_devtype(device.get(), "usb", "usb_device");
        if (!usbDevice)
        {
            continue;
        }

        if (fromHex<unsigned int>(udev_device_get_sysattr_value(usbDevice, "idVendor")) != vendorId)
        {
            continue;
        }

        if (fromHex<unsigned int>(udev_device_get_sysattr_value(usbDevice, "idProduct")) != productId)
        {
            continue;
        }

        auto devicePath = udev_device_get_devnode(device.get());
        if (devicePath)
        {
            return std::string(devicePath);
        }
    }

    throw std::runtime_error("Unable to find device");

#else // HAVE_LIBUDEV

    (void)vendorId;
    (void)productId;
    throw std::runtime_error("tt-helper must be installed with libudev support to autodetect device path");

#endif // HAVE_LIBUDEV
}

} // namespace tt_helper
