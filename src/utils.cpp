#include "utils.h"

#include <fstream>
#include <stdexcept>

namespace ug2e {

    std::vector<unsigned char> FileUtils::read_file_bytes(const std::filesystem::path& path) {
        std::ifstream file_handle(path);
        if (!file_handle) {
            throw std::runtime_error("Failed to open file at '" + path.string() + "'.");
        }
        file_handle.seekg(0, std::ios_base::end);
        const auto file_size = file_handle.tellg();
        file_handle.seekg(0, std::ios_base::beg);
        std::vector<unsigned char> data(file_size);
        file_handle.read(reinterpret_cast<char*>(data.data()), file_size);
        return data;
    }

    std::uint32_t ByteUtils::parse_u32(const std::vector<unsigned char>& data, std::size_t offset) {
        return
            data.at(offset) |
            data.at(offset + 1) << 8 |
            data.at(offset + 2) << 16 |
            data.at(offset + 3) << 24;
    }

}