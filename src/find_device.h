#pragma once

#include "board.h"

#include <cstdint>
#include <string>

namespace tt_helper
{

std::string findDevice(Board board);
std::string findDevice(std::uint16_t vendorId, std::uint16_t productId);

} // namespace tt_helper
