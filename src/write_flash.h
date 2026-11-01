#pragma once

#include "board.h"

#include <string>

namespace tt_helper
{

void writeFlash(Board board, const std::string& devicePath, const std::string& filePath);

} // namespace tt_helper
