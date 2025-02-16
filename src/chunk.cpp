#include "chunk.h"
#include "utils.h"

namespace ug2e {

    Chunk::Chunk(
        std::uint32_t type,
        std::uint32_t size,
        std::vector<std::unique_ptr<Chunk>>&& children,
        std::vector<unsigned char>&& data) :
        type(type), size(size), children(std::move(children)), data(std::move(data)) {}

    std::ostream& operator<<(std::ostream& os, const Chunk& chunk) {
        os << "Chunk[type=" << chunk.type << ", size=" << chunk.size << ", children=" << chunk.children.size() << ", data=[" << VectorUtils::vec_to_str(chunk.data) << "]]" << std::endl;
        for (const auto& child : chunk.children) {
            os << *child << std::endl;
        }
        return os;
    }

    std::unique_ptr<Chunk> Chunk::parse_root(const std::vector<unsigned char>& data) {
        return parse(data, 0);
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
        return std::make_unique<Chunk>(type, size, std::move(children), std::move(chunk_data));
    }
    
    bool Chunk::is_parent(std::uint32_t type) {
        return (type & 0x80000000) == 0x80000000;
    }

}