#include "board.h"
#include "install_fpga.h"
#include "write_flash.h"

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

    auto cli = lyra::cli()
        | lyra::opt(toolName, "TOOL")["-t"]["--tool"]("Tool to run (write-flash, install-fpga)").choices("write-flash", "install-fpga").required()
        | lyra::opt(filePath, "FILE")["-f"]["--file"]("File to load").required()
        | lyra::opt(boardName, "BOARD")["-b"]["--board"]("Board in use (pico-ice, tt-etr)").choices("pico-ice", "tt-etr")
        | lyra::opt(devicePath, "PATH")["-d"]["--device"]("Path to serial device.")
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
        else if (boardName == "tt-etr")
        {
            board = Board::TinyTapeoutEtr;
        }
        else
        {
            throw std::logic_error(std::format("Invalid board name: {}", boardName));
        }

        if (toolName == "write-flash")
        {
            writeFlash(board, devicePath, filePath);
        }
        else if (toolName == "install-fpga")
        {
            installFpga(board, devicePath, filePath);
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

