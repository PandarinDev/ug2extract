#pragma once

#include <cstdint>
#include <vector>
#include <memory>
#include <ostream>

namespace ug2e {

    struct Chunk {
        
        std::uint32_t type;
        std::uint32_t size;
        std::vector<std::unique_ptr<Chunk>> children;
        std::vector<unsigned char> data;

        Chunk(
            std::uint32_t type,
            std::uint32_t size,
            std::vector<std::unique_ptr<Chunk>>&& children,
            std::vector<unsigned char>&& data);

        static std::unique_ptr<Chunk> parse_root(const std::vector<unsigned char>& data);

        friend std::ostream& operator<<(std::ostream& os, const Chunk& chunk);

    private:

        static std::unique_ptr<Chunk> parse(const std::vector<unsigned char>& data, std::size_t offset);
        static bool is_parent(std::uint32_t type);

    };

}