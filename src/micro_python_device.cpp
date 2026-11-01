#include "micro_python_device.h"

#include <asio/buffer.hpp>
#include <asio/read.hpp>
#include <asio/read_until.hpp>
#include <asio/write.hpp>

#include <array>
#include <cstdint>
#include <stdexcept>

namespace tt_helper
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

std::string MicroPythonDevice::execute(std::string_view command)
{
    if (command.empty())
    {
        return {};
    }

    asio::write(m_serialPort, asio::buffer(command));

    const auto commitCommand = std::to_array<const std::uint8_t>({0x04});
    asio::write(m_serialPort, asio::buffer(commitCommand));

    std::array<char, 2> okBuffer;
    asio::read(m_serialPort, asio::buffer(okBuffer));

    if (okBuffer[0] != 'O' || okBuffer[1] != 'K')
    {
        throw std::runtime_error("device did not return expected response");
    }

    std::string response;
    auto cnt = asio::read_until(m_serialPort, asio::dynamic_buffer(response), "\x04>");
    response.resize(cnt);

    std::string_view responseView(response);

    auto exceptionStart = responseView.find('\x04');
    if (exceptionStart == std::string_view::npos)
    {
        throw std::runtime_error("Device returned invalid data");
    }

    auto exceptionEnd = responseView.find('\x04', exceptionStart + 1);
    if (exceptionEnd == std::string_view::npos)
    {
        throw std::runtime_error("Device returned invalid data");
    }

    auto exception = responseView.substr(exceptionStart + 1, exceptionEnd - exceptionStart - 1);
    if (!exception.empty())
    {
        throw std::runtime_error(std::format("Device threw exception: {}", exception));
    }

    return std::string(responseView.substr(exceptionEnd));
}

} // namespace tt_helper
