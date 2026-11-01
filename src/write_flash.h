#pragma once

#include "board.h"

#include <filesystem>
#include <string>

namespace tt_helper
{

void writeFlash(Board board, const std::string& devicePath, const std::filesystem::path& filePath);

} // namespace tt_helper
