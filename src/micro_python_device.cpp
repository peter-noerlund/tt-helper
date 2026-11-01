#include "micro_python_device.h"

#include <asio/buffer.hpp>
#include <asio/read_until.hpp>
#include <asio/write.hpp>

#include <array>
#include <cstdint>

namespace imp_tool
{

MicroPythonDevice::MicroPythonDevice(const std::string& devicePath)
    : m_serialPort(m_context, devicePath)
{
    init();
}

MicroPythonDevice::~MicroPythonDevice()
{
    fini();
}

void MicroPythonDevice::init()
{
    // Ctrl+C, Ctrl+C, Ctrl+B, Ctrl+D
    const auto resetSequence = std::to_array<const std::uint8_t>({0x03, 0x03, 0x02, 0x04});
    asio::write(m_serialPort, asio::buffer(resetSequence));

    // Wait for prompt
    std::string lineBuffer;
    asio::read_until(m_serialPort, asio::dynamic_buffer(lineBuffer), ">>> ");

    // Enter RAW REPL by sending Ctrl+A
    const auto rawReplCommand = std::to_array<const std::uint8_t>({0x01});
    asio::write(m_serialPort, asio::buffer(rawReplCommand));

    lineBuffer.clear();
    asio::read_until(m_serialPort, asio::dynamic_buffer(lineBuffer), "raw REPL; Ctrl-B to exit\r\n>");
}

void MicroPythonDevice::fini()
{
    // Ctrl+C, Ctrl+C, Ctrl+B, Ctrl+D
    const auto resetSequence = std::to_array<const std::uint8_t>({0x03, 0x03, 0x02, 0x04});
    asio::write(m_serialPort, asio::buffer(resetSequence));
}

} // namespace imp_tool
