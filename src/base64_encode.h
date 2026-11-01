#pragma once

#include <cstdint>
#include <span>
#include <string>

namespace tt_helper
{

std::string base64Encode(std::span<const std::uint8_t> data);

} // namespace tt_helper
