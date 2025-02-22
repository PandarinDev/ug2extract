#pragma once

#include "chunk.h"

#include <vector>
#include <string>
#include <cstdint>
#include <filesystem>

namespace ug2e {

    struct FileUtils {

        FileUtils() = delete;

        static std::vector<unsigned char> read_file_bytes(const std::filesystem::path& path);

    };

    struct ByteUtils {

        ByteUtils() = delete;

        static std::uint32_t parse_u32(const std::vector<unsigned char>& data, std::size_t offset);

    };

    struct VectorUtils {

        VectorUtils() = delete;

        template<typename T>
        static std::string vec_to_str(const std::vector<T>& vec) {
            std::string result;
            for (const auto& entry : vec) {
                result += std::to_string(entry) + ", ";
            }
            return result;
        }

        template<>
        static std::string vec_to_str<unsigned char>(const std::vector<unsigned char>& vec) {
            static char constexpr hex_chars[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
            std::string result;
            for (const auto& entry : vec) {
                result += "0x";
                result += hex_chars[(entry & 0xF0) >> 4];
                result += hex_chars[entry & 0xF];
                result += ", ";
            }
            return result;
        }

    };

    struct ChunkUtils {

        ChunkUtils() = delete;

        static std::vector<const Chunk*> get_all_with_type(const Chunk& root, std::uint32_t type);

    };

}