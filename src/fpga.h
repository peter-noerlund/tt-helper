#pragma once

#include "board.h"
#include "micro_python_device.h"

#include <filesystem>
#include <string>

namespace tt_helper
{

class Fpga
{
public:
    explicit Fpga(Board board, const std::string& devicePath, bool dryRun, bool verbose);

    void install(const std::filesystem::path& filePath);
    void start();
    void stop();

private:
    MicroPythonDevice m_device;
    Board m_board;
    bool m_dryRun;
};

} // namespace tt_helper
