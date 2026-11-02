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
    explicit Fpga(Board board, const std::string& ttyPath, const std::filesystem::path& mountPath, bool dryRun, bool verbose);

    void run(const std::filesystem::path& filename);
    void install(const std::filesystem::path& filename);
    void start();
    void stop();

private:
    void installFile(const std::filesystem::path& filename);
    static std::string_view fpgaCommand() noexcept;

    MicroPythonDevice m_device;
    std::filesystem::path m_mountPath;
    Board m_board;
    bool m_dryRun;
    bool m_verbose;
};

} // namespace tt_helper
