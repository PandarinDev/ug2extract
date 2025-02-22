#include "chunk.h"
#include "utils.h"

namespace ug2e {

    Chunk::Chunk(
        std::uint32_t type,
        std::uint32_t size,
        std::size_t offset,
        std::vector<std::unique_ptr<Chunk>>&& children,
        std::vector<unsigned char>&& data) :
        type(type), size(size), offset(offset), children(std::move(children)), data(std::move(data)) {
        }

    std::ostream& operator<<(std::ostream& os, const Chunk& chunk) {
        os << "Chunk[type=" << chunk.type << ", size=" << chunk.size <<
            ", offset=" << chunk.offset << ", children=" <<
            chunk.children.size() << "]" << std::endl;
        for (const auto& child : chunk.children) {
            os << *child << std::endl;
        }
        return os;
    }

    std::unique_ptr<Chunk> Chunk::parse_root(const std::vector<unsigned char>& data) {
        return parse(data, 0);
    }

    std::optional<std::size_t> Chunk::find_first_normal_offset() const {
        for (std::size_t i = 0; i < data.size() - 12; i += 4) {
            if (is_likely_normal_vector(i)) {
                return i;
            }
        }
        return std::nullopt;
    }

    std::vector<std::size_t> Chunk::find_normal_offsets() const {
        std::vector<std::size_t> offsets;
        if (data.size() < 12) {
            return offsets;
        }
        for (std::size_t i = 0; i < data.size() - 12; i += 4) {
            if (is_likely_normal_vector(i)) {
                offsets.emplace_back(i);
            }
        }
        return offsets;
    }

    std::unique_ptr<Chunk> Chunk::parse(const std::vector<unsigned char>& data, std::size_t offset) {
        static constexpr std::size_t header_size = 8;
        const auto type = ByteUtils::parse_u32(data, offset);
        const auto size = ByteUtils::parse_u32(data, offset + 4);
        std::vector<std::unique_ptr<Chunk>> children;
        std::vector<unsigned char> chunk_data;
        auto cursor = offset + header_size;
        const auto chunk_start = cursor;
        const auto chunk_end = chunk_start + size;
        if (is_parent(type)) {
            while (cursor < chunk_end) {
                auto child = parse(data, cursor);
                cursor += child->size + header_size;
                // Ignore padding blocks
                if (child->size > 0) {
                    children.emplace_back(std::move(child));
                }
            }
        }
        else {
            chunk_data = std::vector<unsigned char>(data.cbegin() + chunk_start, data.cbegin() + chunk_end);
        }
        return std::make_unique<Chunk>(type, size, offset, std::move(children), std::move(chunk_data));
    }
    
    bool Chunk::is_parent(std::uint32_t type) {
        return (type & 0x80000000) == 0x80000000;
    }

    bool Chunk::is_likely_normal_vector(std::size_t from) const {
        const auto f1 = *reinterpret_cast<const float*>(&data.at(from));
        const auto f2 = *reinterpret_cast<const float*>(&data.at(from + 4));
        const auto f3 = *reinterpret_cast<const float*>(&data.at(from + 8));
        const auto length = std::sqrtf(f1*f1 + f2*f2 + f3*f3);
        return length >= 0.9999f && length <= 1.0001f;
    }

}