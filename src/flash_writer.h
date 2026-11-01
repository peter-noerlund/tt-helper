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

class FlashWriter
{
public:
    static void writeFlash(Board board, const std::string& devicePath, const std::filesystem::path& filePath, bool dryRun, bool verbose);

private:
    static void eraseFlash(MicroPythonDevice& device, std::size_t size);
    static void programFlash(MicroPythonDevice& device, const std::filesystem::path& filePath, std::size_t size);

    static std::string initCommand(std::span<const int, 8> pins);
    static std::string_view cleanupCommand();
    static std::string verifyCommand();
    static std::string spiCommand(std::span<const std::uint8_t> data, std::size_t recvBytes = 0, std::string_view indent = {});
    static std::string writeEnableCommand(std::string_view indent = {});
    static std::string sleepCommand(std::chrono::nanoseconds delay, std::string_view indent = {});
    static std::string waitForWriteCommand(std::chrono::nanoseconds delay, std::string_view indent = {});
};

} // namespace tt_helper
