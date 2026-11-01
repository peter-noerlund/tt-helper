#include "fpga.h"

#include <stdexcept>

namespace tt_helper
{

Fpga::Fpga(Board board, const std::string& devicePath, bool dryRun, bool verbose)
    : m_device(devicePath, dryRun, verbose)
    , m_board(board)
    , m_dryRun(dryRun)
{
}

void Fpga::install(const std::filesystem::path&)
{
    throw std::runtime_error("Not implemented");
}

void Fpga::start()
{
    throw std::runtime_error("Not implemented");
}

void Fpga::stop()
{
    throw std::runtime_error("Not implemented");
}

} // namespace tt_helper
