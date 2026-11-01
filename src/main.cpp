#include "board.h"
#include "flash_writer.h"
#include "fpga.h"
#include "find_device.h"

#include <lyra/lyra.hpp>

#include <cstdio>
#include <iostream>
#include <format>
#include <print>
#include <stdexcept>
#include <string>

int main(int argc, char** argv)
{
    using namespace tt_helper;

    bool showHelp = false;
    std::string toolName;
    std::string boardName;
    std::string devicePath;
    std::string filePath;
    bool dryRun = false;
    bool verbose = false;

    auto cli = lyra::cli()
        | lyra::opt(toolName, "TOOL")["-t"]["--tool"]("Tool to run (write-flash, run-fpga, install-fpga, start-fpga, stop-fpga)").choices("write-flash", "run-fpga", "install-fpga", "start-fpga", "stop-fpga").required()
        | lyra::opt(boardName, "BOARD")["-b"]["--board"]("Board in use (pico-ice, tt-etr)").choices("pico-ice", "tt-06", "tt-etr").required()
        | lyra::opt(filePath, "FILE")["-f"]["--file"]("File to load")
        | lyra::opt(devicePath, "PATH")["-d"]["--device"]("Path to serial device.")
        | lyra::opt(dryRun)["--dry-run"]("Dry run")
        | lyra::opt(verbose)["-v"]["--verbose"]("Be verbose")
        | lyra::help(showHelp);

    auto result = cli.parse({argc, argv});
    if (!result)
    {
        std::println(stderr, "Error parsing command line arguments: {}", result.message());
        return EXIT_FAILURE;
    }
    if (showHelp)
    {
        std::cout << cli << std::endl;
        return EXIT_SUCCESS;
    }

    try
    {
        Board board = Board::Unknown;
        if (boardName == "pico-ice")
        {
            board = Board::PicoIce;
        }
        else if (boardName == "tt-06")
        {
            board = Board::TinyTapeout06;
        }
        else if (boardName == "tt-etr")
        {
            board = Board::TinyTapeoutEtr;
        }
        else
        {
            throw std::logic_error(std::format("Invalid board name: {}", boardName));
        }

        if (devicePath.empty())
        {
            devicePath = findDevice(board);
        }

        if (toolName == "write-flash")
        {
            if (filePath.empty())
            {
                throw std::invalid_argument("Missing filename");
            }
            FlashWriter::writeFlash(board, devicePath, filePath, dryRun, verbose);
        }
        else if (toolName == "run-fpga")
        {
            if (filePath.empty())
            {
                throw std::invalid_argument("Missing filename");
            }

            Fpga fpga(board, devicePath, dryRun, verbose);
            fpga.run(filePath);
        }

        else if (toolName == "install-fpga")
        {
            if (filePath.empty())
            {
                throw std::invalid_argument("Missing filename");
            }

            Fpga fpga(board, devicePath, dryRun, verbose);
            fpga.install(filePath);
        }
        else if (toolName == "start-fpga")
        {
            Fpga fpga(board, devicePath, dryRun, verbose);
            fpga.start();
        }
        else if (toolName == "stop-fpga")
        {
            Fpga fpga(board, devicePath, dryRun, verbose);
            fpga.stop();
        }
        else
        {
            throw std::logic_error(std::format("Invalid tool: {}", toolName));
        }

        return EXIT_SUCCESS;
    }
    catch (const std::exception& exception)
    {
        std::println(stderr, "Caught exception: {}", exception.what());
        return EXIT_FAILURE;
    }
}

