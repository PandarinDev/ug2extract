#pragma once

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
                result += std::to_string(entry);
            }
            return result;
        }

    };

}