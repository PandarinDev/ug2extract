#pragma once

#include <cstdint>
#include <vector>
#include <memory>
#include <ostream>
#include <optional>

namespace ug2e {

    struct Chunk {

        std::uint32_t type;
        std::uint32_t size;
        std::size_t offset;
        std::vector<std::unique_ptr<Chunk>> children;
        std::vector<unsigned char> data;

        Chunk(
            std::uint32_t type,
            std::uint32_t size,
            std::size_t offset,
            std::vector<std::unique_ptr<Chunk>>&& children,
            std::vector<unsigned char>&& data);

        static std::unique_ptr<Chunk> parse_root(const std::vector<unsigned char>& data);

        std::optional<std::size_t> find_first_normal_offset() const;
        std::vector<std::size_t> find_normal_offsets() const;

        friend std::ostream& operator<<(std::ostream& os, const Chunk& chunk);

    private:

        static std::unique_ptr<Chunk> parse(const std::vector<unsigned char>& data, std::size_t offset);
        static bool is_parent(std::uint32_t type);
        
        bool is_likely_normal_vector(std::size_t from) const;

    };

}