#include "fpga.h"

#include <stdexcept>

namespace tt_helper
{

Fpga::Fpga(Board board, const std::string& devicePath, bool dryRun, bool verbose)
    : m_device(devicePath, dryRun, verbose)
    , m_board(board)
    , m_dryRun(dryRun)
{
    if (board != Board::PicoIce)
    {
        throw std::invalid_argument("Unsupported board");
    }
}

void Fpga::run(const std::filesystem::path&)
{
    throw std::runtime_error("Not implemented");
}

void Fpga::install(const std::filesystem::path&)
{
    throw std::runtime_error("Not implemented");
}

void Fpga::start()
{
    m_device.execute(
        "from machine import Pin\r\n"
        "import ice\r\n"
        "fpga = ice.fpga(cdone=Pin(26), clock=Pin(24), creset=Pin(27), cram_cs=Pin(9), cram_mosi=Pin(8), cram_sck=Pin(10), frequency=48)\r\n"
        "fpga.start()\r\n"
    );
}

void Fpga::stop()
{
    m_device.execute("fpga.stop()");
}

} // namespace tt_helper
