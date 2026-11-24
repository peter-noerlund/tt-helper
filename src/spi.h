#pragma once

#include "board.h"

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <span>
#include <string.h>

namespace tt_helper
{

class MicroPythonDevice;

class SPI
{
public:
    static void writeFlash(Board board, const std::string& ttyPath, const std::filesystem::path& filePath, bool dryRun, bool verbose);
    static void enterQuad(Board board, const std::string& ttyPath, bool dryRun, bool verbose);
    static void pingFlash(Board board, const std::string& ttyPath, bool dryRun, bool verbose);
    static void pingRAM(Board board, const std::string& ttyPath, bool dryRun, bool verbose);

private:
    enum class ChipSelect
    {
        Flash,
        PSRAM1,
        PSRAM2
    };

    static MicroPythonDevice initDevice(Board board, const std::string& ttyPath, bool dryRun, bool verbose, ChipSelect chipSelect);

    static void eraseFlash(MicroPythonDevice& device, std::size_t size);
    static void programFlash(MicroPythonDevice& device, const std::filesystem::path& filePath, std::size_t size);

    static std::string initCommand(std::span<const int, 8> pins, ChipSelect chipSelect);
    static std::string_view cleanupCommand();
    static std::string verifyCommand();
    static std::string spiCommand(std::span<const std::uint8_t> data, std::size_t recvBytes = 0, std::string_view indent = {});
    static std::string writeEnableCommand(std::string_view indent = {});
    static std::string sleepCommand(std::chrono::nanoseconds delay, std::string_view indent = {});
    static std::string waitForWriteCommand(std::chrono::nanoseconds delay, std::string_view indent = {});
    static std::string enterQuadCommand();
};

} // namespace tt_helper
