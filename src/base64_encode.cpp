#include "base64_encode.h"

namespace tt_helper
{

std::string base64Encode(std::span<const std::uint8_t> data)
{
    static const std::array<char, 64> alphabet = {
        'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q',
        'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h',
        'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y',
        'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'
    };

    std::string out;
    out.reserve(((data.size() + 1) * 4) / 3);
 
    while (!data.empty())
    {
        auto b0 = data[0];
        out.push_back(alphabet[b0 >> 2]);                                       // xxxxxx-- -------- --------
        if (data.size() == 1)
        {
            out.push_back(alphabet[(b0 << 4) & 0x30]);
            break;
        }
 
        auto b1 = data[1];
        out.push_back(alphabet[((b0 << 4) & 0x30) | ((b1 >> 4) & 0x0F)]);       // ------xx xxxx---- --------
        if (data.size() == 2)
        {
            out.push_back(alphabet[(b1 << 2) & 0x3C]);
            break;
        }
 
        auto b2 = data[2];
        out.push_back(alphabet[((b1 << 2) & 0x3C) | ((b2 >> 6) & 0x03)]);       // -------- ----xxxx xx------
        out.push_back(alphabet[b2 & 0x3F]);                                     // -------- -------- --xxxxxx
 
        data = data.subspan(3);
    }
 
    while (out.size() % 4 != 0)
    {
        out.push_back('=');
    }
 
    return out;
}

} // namespace tt_helper
