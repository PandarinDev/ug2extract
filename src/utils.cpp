#include "utils.h"

#include <fstream>
#include <stdexcept>
#include <functional>

namespace ug2e {

    std::vector<unsigned char> FileUtils::read_file_bytes(const std::filesystem::path& path) {
        std::ifstream file_handle(path, std::ios::binary | std::ios::ate);
        if (!file_handle) {
            throw std::runtime_error("Failed to open file at '" + path.string() + "'.");
        }
        const auto file_size = file_handle.tellg();
        file_handle.seekg(0, std::ios_base::beg);
        std::vector<unsigned char> data(file_size);
        if (!file_handle.read(reinterpret_cast<char*>(data.data()), file_size)) {
            throw std::runtime_error("Failed to read file '" + path.string() + "' to completion.");
        }
        return data;
    }

    std::uint32_t ByteUtils::parse_u32(const std::vector<unsigned char>& data, std::size_t offset) {
        return
            data.at(offset) |
            data.at(offset + 1) << 8 |
            data.at(offset + 2) << 16 |
            data.at(offset + 3) << 24;
    }

    std::vector<const Chunk*> ChunkUtils::get_all_with_type(const Chunk& root, std::uint32_t type) {
        std::vector<const Chunk*> result;
        std::function<void(const Chunk&)> add_if_matching = [&add_if_matching, &result, &type](const Chunk& chunk) {
            if (chunk.type == type) {
                result.emplace_back(&chunk);
            }
            for (const auto& child : chunk.children) {
                add_if_matching(*child);
            }
        };
        add_if_matching(root);
        return result;
    }

}