#include "spi.h"

#include "base64_encode.h"
#include "micro_python_device.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdio>
#include <fstream>
#include <format>
#include <iterator>
#include <print>
#include <span>
#include <stdexcept>
#include <thread>

namespace tt_helper
{

std::string SPI::spiCommand(std::span<const std::uint8_t> data, std::size_t recvBytes, std::string_view indent)
{
    std::string command = std::format(
            "{0}spi_cs(0)\r\n"
            "{0}spi.write(binascii.a2b_base64(\"{1}\"))\r\n",
            indent, base64Encode(data));
    if (recvBytes > 0)
    {
        std::format_to(std::back_inserter(command), "{}res = spi.read({})\r\n", indent, recvBytes);
    }

    std::format_to(std::back_inserter(command), "{}spi_cs(1)\r\n", indent);

    return command;
}

std::string SPI::writeEnableCommand(std::string_view indent)
{
    return spiCommand(std::to_array<std::uint8_t>({0x06}), 0, indent);
}

std::string SPI::sleepCommand(std::chrono::nanoseconds delay, std::string_view indent)
{
    using namespace std::chrono_literals;

    std::string sleepCommand;
    if (delay.count() % 1000000 == 0)
    {
        return std::format("{}time.sleep_ms({})\r\n", indent, std::chrono::duration_cast<std::chrono::milliseconds>(delay).count());
    }
    else if (delay < 1us)
    {
        return std::format("{}time.sleep_us(1)\r\n", indent);
    }
    else
    {
        return std::format("{}time.sleep_us({})\r\n", indent, std::chrono::duration_cast<std::chrono::microseconds>(delay).count());
    }
}

std::string SPI::waitForWriteCommand(std::chrono::nanoseconds delay, std::string_view indent)
{
    return std::format(
            "{0}while True:\r\n"
            "{1}\r\n"
            "{2}\r\n"
            "{0}    if res[0] & 1 == 0:\r\n"
            "{0}        break\r\n",
            indent,
            sleepCommand(delay, std::format("{}    ", indent)),
            spiCommand(std::to_array<std::uint8_t>({0x05}), 1, std::format("{}    ", indent))
    );
}

std::string SPI::initCommand(std::span<const int, 8> pins)
{
    return std::format(
        "from machine import Pin, SoftSPI\r\n"
        "import binascii\r\n"
        "import time\r\n"
        "spi_cs = Pin({})\r\n"
        "spi_mosi = Pin({})\r\n"
        "spi_miso = Pin({})\r\n"
        "spi_sck = Pin({})\r\n"
        "spi_io2 = Pin({})\r\n"
        "spi_io3 = Pin({})\r\n"
        "spi_cs1 = Pin({})\r\n"
        "spi_cs2 = Pin({})\r\n"
        "spi_cs.init(spi_cs.OUT)\r\n"
        "spi_cs(1)\r\n"
        "spi_cs1.init(spi_cs1.OUT)\r\n"
        "spi_cs1(1)\r\n"
        "spi_cs2.init(spi_cs2.OUT)\r\n"
        "spi_cs2(1)\r\n"
        "spi_io2.init(spi_io2.IN)\r\n"
        "spi_io2.init(spi_io3.IN)\r\n"
        "spi = SoftSPI(sck=spi_sck, mosi=spi_mosi, miso=spi_miso)\r\n"
        "{}"
        "if res[0] != 0xEF or res[1] != 0x70 or res[2] != 0x18:\r\n"
        "    raise Exception(\"Unsupported device\")\r\n",
        pins[0], pins[1], pins[2], pins[3], pins[4], pins[5], pins[6], pins[7],
        spiCommand(std::to_array<std::uint8_t>({0x9F}), 3)
    );
}

std::string_view SPI::cleanupCommand()
{
    return
        "del spi\r\n"
        "spi_cs.init(spi_cs.IN)\r\n"
        "spi_mosi.init(spi_mosi.IN)\r\n"
        "spi_miso.init(spi_miso.IN)\r\n"
        "spi_sck.init(spi_sck.IN)\r\n"
        "spi_cs1.init(spi_cs1.IN)\r\n"
        "spi_cs2.init(spi_cs2.IN)\r\n"
        "del spi_cs\r\n"
        "del spi_mosi\r\n"
        "del spi_miso\r\n"
        "del spi_sck\r\n"
        "del spi_io2\r\n"
        "del spi_io3\r\n"
        "del spi_cs1\r\n"
        "del spi_cs2\r\n";
}

void SPI::eraseFlash(MicroPythonDevice& device, std::size_t size)
{
    using namespace std::chrono_literals;

    auto blocks64 = size / 65536;
    auto blocks32 = (size - blocks64 * 65536) / 32768;
    auto sectors = (size - blocks64 * 65536 - blocks32 * 32768) / 4096;
    if (size > blocks64 * 65536 + blocks32 * 32768 + sectors * 4096)
    {
        ++sectors;
    }

    auto eraseSize = blocks64 * 65536 + blocks32 * 32768 + sectors * 4096;

    std::print("Erasing: 000000/{:06x} (0%)", eraseSize);
    std::fflush(stdout);

    std::uint32_t address = 0;
    for (auto [opcode, count, blockSize, delay] : std::to_array<std::tuple<std::uint8_t, unsigned int, unsigned int, std::chrono::nanoseconds>>({
                {0xD8, blocks64, 64, 150ms},
                {0x52, blocks32, 32, 120ms},
                {0x20, sectors,   4,  45ms}
                }))
    {
        for (unsigned int i = 0; i != count; ++i)
        {
            auto replCommand =
                writeEnableCommand() +
                spiCommand(std::to_array<std::uint8_t>({
                            opcode,
                            static_cast<std::uint8_t>(address >> 16),
                            static_cast<std::uint8_t>(address >> 8),
                            static_cast<std::uint8_t>(address)})) +
                waitForWriteCommand(delay);

            device.execute(replCommand);

            address += blockSize * 1024;

            std::print("\rErasing: {:06x}/{:06x} ({}%)", address, eraseSize, address * 100 / eraseSize);
            std::fflush(stdout);
        }
    }

    std::println("");
}

void SPI::programFlash(MicroPythonDevice& device, const std::filesystem::path& filePath, std::size_t size)
{
    using namespace std::chrono_literals;

    std::ifstream file;
    file.exceptions(std::ifstream::badbit | std::ifstream::failbit);
    file.open(filePath, std::ifstream::in | std::ifstream::binary);

    std::vector<std::uint8_t> buffer;
    buffer.reserve(4 * 256);

    std::print("Programming: 000000/{:06x} (0%)", size);
    std::fflush(stdout);

    std::uint32_t address = 0;
    auto remainingSize = size;
    while (remainingSize != 0)
    {
        auto chunkSize = std::min<std::size_t>(remainingSize, 256);

        buffer.resize(4 + chunkSize);
        buffer.at(0) = 0x02;
        buffer.at(1) = static_cast<std::uint8_t>(address >> 16);
        buffer.at(2) = static_cast<std::uint8_t>(address >> 8);
        buffer.at(3) = static_cast<std::uint8_t>(address);

        file.read(static_cast<char*>(static_cast<void*>(buffer.data() + 4)), chunkSize);

        auto replCommand =
            writeEnableCommand() +
            spiCommand(buffer) +
            waitForWriteCommand(440ns);

        device.execute(replCommand);

        address += chunkSize;
        remainingSize -= chunkSize;

        std::print("\rProgramming: {:06x}/{:06x} ({}%)", address, size, address * 100 / size);
        std::fflush(stdout);
    }

    std::println("");

}

void SPI::writeFlash(Board board, const std::string& ttyPath, const std::filesystem::path& filePath, bool dryRun, bool verbose)
{
    static const std::array<int, 8> picoIcePins = {1, 3, 0, 2, 5, 7, 4, 6};
    static const std::array<int, 8> tt06Pins = {21, 22, 23, 24, 25, 26, 27, 28};
    static const std::array<int, 8> etrPins = {25, 26, 27, 28, 29, 30, 31, 32};
    std::span<const int> pins;

    switch (board)
    {
        case Board::PicoIce:
            pins = picoIcePins;
            break;

        case Board::TinyTapeoutEtr:
            pins = etrPins;
            break;

        case Board::TinyTapeout06:
            pins = tt06Pins;
            break;

        case Board::Unknown:
            break;
    }

    if (pins.empty())
    {
        throw std::invalid_argument("Unsupported board");
    }

    if (!std::filesystem::exists(filePath))
    {
        throw std::invalid_argument(std::format("File {} does not exist", filePath.string()));
    }

    MicroPythonDevice device(ttyPath, dryRun, verbose);

    device.execute(initCommand(pins.subspan<0, 8>()));

    auto size = std::filesystem::file_size(filePath);
    if (size > 16 * 1024 * 1024)
    {
        throw std::invalid_argument("File is too large");
    }

    eraseFlash(device, size);
    programFlash(device, filePath, size);

    device.execute(cleanupCommand());
}

} // namespace tt_helper
