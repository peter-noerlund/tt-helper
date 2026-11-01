#pragma once

#include <asio/io_context.hpp>
#include <asio/serial_port.hpp>

#include <string>

namespace imp_tool
{

class MicroPythonDevice
{
public:
    explicit MicroPythonDevice(const std::string& devicePath);
    ~MicroPythonDevice();

    std::string execute(std::string_view command);

private:
    void init();
    void fini();

    asio::io_context m_context;
    asio::serial_port m_serialPort;
};

} // namespace imp_tool
