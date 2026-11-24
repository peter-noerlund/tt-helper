#pragma once

#include <asio/io_context.hpp>
#include <asio/serial_port.hpp>

#include <memory>
#include <string>

namespace tt_helper
{

class MicroPythonDevice
{
public:
    explicit MicroPythonDevice(const std::string& ttyPath, bool dryRun, bool verbose);
    MicroPythonDevice(const MicroPythonDevice&) = delete;
    MicroPythonDevice(MicroPythonDevice&& other) noexcept = default;
    ~MicroPythonDevice();

    MicroPythonDevice& operator=(const MicroPythonDevice&) = delete;
    MicroPythonDevice& operator=(MicroPythonDevice&&) = delete;

    std::string execute(std::string_view command);

    void reset();

private:
    void init();
    void fini();

    std::unique_ptr<asio::io_context> m_context;
    std::optional<asio::serial_port> m_serialPort;
    bool m_dryRun;
    bool m_verbose;
};

} // namespace tt_helper
