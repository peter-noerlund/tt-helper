#include "device_finder.h"

#include "from_hex.h"

#ifdef HAVE_LIBUDEV

#ifdef HAVE_LIBMOUNT

#include "unique_libmnt_table.h"

#endif // HAVE_LIBMOUNT

#include "unique_udev.h"
#include "unique_udev_device.h"
#include "unique_udev_enumerate.h"

#include <libudev.h>

#endif // HAVE_LIBUDEV

#include <stdexcept>

namespace tt_helper
{

DeviceFinder::DevicePath DeviceFinder::findDevice(Board board)
{
    switch (board)
    {
        case Board::PicoIce:
            return findDevice(0x1209, 0xB1C0);

        case Board::TinyTapeoutEtr:
            return findDevice(0x2E8A, 0x0005);

        case Board::TinyTapeout06:
            break;

        case Board::Unknown:
            break;
    }
    
    return {};
}

DeviceFinder::DevicePath DeviceFinder::findDevice(std::uint16_t vendorId, std::uint16_t productId)
{
    DevicePath path;

#ifdef HAVE_LIBUDEV

    UniqueUdev udev(udev_new());
    auto usbDevice = findUsbDevice(udev.get(), vendorId, productId);
    if (!usbDevice)
    {
        return {};
    }

    path.ttyPath = findTtyPath(usbDevice.get());
    path.mountPath = findMountPath(usbDevice.get());

#endif // HAVE_LIBUDEV

    return path;
}

#ifdef HAVE_LIBUDEV

UniqueUdevDevice DeviceFinder::findUsbDevice(udev* udev, std::uint16_t vendorId, std::uint16_t productId)
{
    UniqueUdevEnumerate udevEnum(udev_enumerate_new(udev));
    udev_enumerate_add_match_subsystem(udevEnum.get(), "usb");
    udev_enumerate_scan_devices(udevEnum.get());

    auto ttyDevices = udev_enumerate_get_list_entry(udevEnum.get());
    decltype(ttyDevices) ttyDevice;
    udev_list_entry_foreach(ttyDevice, ttyDevices)
    {
        UniqueUdevDevice usbDevice(udev_device_new_from_syspath(udev, udev_list_entry_get_name(ttyDevice)));
        if (!usbDevice)
        {
            continue;
        }

        auto devType = udev_device_get_devtype(usbDevice.get());
        if (devType != std::string_view("usb_device"))
        {
            continue;
        }

        if (fromHex<unsigned int>(udev_device_get_sysattr_value(usbDevice.get(), "idVendor")) != vendorId)
        {
            continue;
        }

        if (fromHex<unsigned int>(udev_device_get_sysattr_value(usbDevice.get(), "idProduct")) != productId)
        {
            continue;
        }

        return usbDevice;
    }

    return nullptr;
}

UniqueUdevDevice DeviceFinder::findChildDevice(udev_device* parentDevice, const std::string& subsystem)
{
    auto udev = udev_device_get_udev(parentDevice);

    UniqueUdevEnumerate udevEnum(udev_enumerate_new(udev));
    udev_enumerate_add_match_parent(udevEnum.get(), parentDevice);
    udev_enumerate_add_match_subsystem(udevEnum.get(), subsystem.c_str());
    udev_enumerate_scan_devices(udevEnum.get());

    auto listEntries = udev_enumerate_get_list_entry(udevEnum.get());
    if (!listEntries)
    {
        return nullptr;
    }

    UniqueUdevDevice device(udev_device_new_from_syspath(udev, udev_list_entry_get_name(listEntries)));

    return device;
}

std::string DeviceFinder::findTtyPath(udev_device* parentDevice)
{
    auto ttyDevice = findChildDevice(parentDevice, "tty");
    if (!ttyDevice)
    {
        return {};
    }

    return udev_device_get_devnode(ttyDevice.get());
}

std::string DeviceFinder::findMountPath(udev_device* parentDevice)
{
    std::string mountPath;
#ifdef HAVE_LIBMOUNT
    auto storageDevice = findChildDevice(parentDevice, "block");
    auto devicePath = udev_device_get_devnode(storageDevice.get());

    UniqueLibmntTable table(mnt_new_table());
    if (mnt_table_parse_mtab(table.get(), nullptr) != 0)
    {
        return {};
    }

    auto fs = mnt_table_find_source(table.get(), devicePath, MNT_ITER_FORWARD);
    if (!fs)
    {
        return {};
    }

    return mnt_fs_get_target(fs);
#else // HAVE_LIBMOUNT
    return {};
#endif // HAVE_LIBMOUNT
}

#endif // HAVE_LIBUDEV

} // namespace tt_helper
