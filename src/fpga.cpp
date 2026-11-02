#include "fpga.h"

#include <unistd.h>

#include <print>
#include <stdexcept>

namespace tt_helper
{

Fpga::Fpga(Board board, const std::string& ttyPath, const std::filesystem::path& mountPath, bool dryRun, bool verbose)
    : m_device(ttyPath, dryRun, verbose)
    , m_mountPath(mountPath)
    , m_board(board)
    , m_dryRun(dryRun)
    , m_verbose(verbose)
{
    if (board != Board::PicoIce)
    {
        throw std::invalid_argument("Unsupported board");
    }
}

void Fpga::run(const std::filesystem::path& filename)
{
    installFile(filename);


    m_device.execute(std::format(
                "{}"
                "file = open(\"{}\", \"rb\")\r\n"
                "fpga.start()\r\n"
                "fpga.cram(file)\r\n",
                fpgaCommand(),
                filename.filename().string()));
}

void Fpga::install(const std::filesystem::path& filename)
{
    installFile(filename);

    m_device.execute(std::format(
                "{}"
                "file = open(\"{}\", \"rb\")\r\n"
                "flash = ice.flash(miso=Pin(8), mosi=Pin(11), sck=Pin(10), cs=Pin(9))\r\n"
                "flash.write(file)\r\n"
                "fpga.start()\r\n",
                fpgaCommand(),
                filename.filename().string()));
}

std::string_view Fpga::fpgaCommand() noexcept
{
    return
        "from machine import Pin\r\n"
        "import ice\r\n"
        "fpga = ice.fpga(cdone=Pin(26), clock=Pin(24), creset=Pin(27), cram_cs=Pin(9), cram_mosi=Pin(8), cram_sck=Pin(10), frequency=48)\r\n";
}

void Fpga::installFile(const std::filesystem::path& filename)
{
    if (m_mountPath.empty())
    {
        throw std::runtime_error("Mount path required");
    }

    auto targetPath = m_mountPath / filename.filename();

    if (m_verbose)
    {
        std::println("Copying {} to {}", filename.string(), targetPath.string());
    }

    if (!m_dryRun)
    {
        std::filesystem::copy_file(filename, targetPath, std::filesystem::copy_options::overwrite_existing);

        ::sync();

        m_device.reset();
    }
}

void Fpga::start()
{
    m_device.execute(std::format("{}fpga.start()\r\n", fpgaCommand()));
}

void Fpga::stop()
{
    m_device.execute(std::format("{}fpga.stop()\r\n", fpgaCommand()));
}

} // namespace tt_helper
