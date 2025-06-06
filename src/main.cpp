#include "chunk.h"
#include "geometry.h"
#include "utils.h"

#include <fstream>
#include <iostream>
#include <filesystem>
#include <functional>
#include <algorithm>

using namespace ug2e;

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: ug2e <bin_file>" << std::endl;
        return 1;
    }
    std::filesystem::path bin_path(argv[1]);
    if (!std::filesystem::is_regular_file(bin_path)) {
        std::cerr << "Binary file at '" + bin_path.string() << "' not found." << std::endl;
        return 1;
    }
    std::vector<unsigned char> bytes;
    try {
        bytes = FileUtils::read_file_bytes(bin_path);
    } catch (const std::exception& e) {
        std::cerr << "Failed to read file: " << e.what() << std::endl;
        return 1;
    }

    const auto root = Chunk::parse_root(bytes);
    static constexpr std::uint32_t name_chunk_type = 0x134011;
    static constexpr std::uint32_t mesh_chunk_type = 0x134B01;
    static constexpr std::uint32_t index_chunk_type = 0x134B03;
    const auto name_chunks = ChunkUtils::get_all_with_type(*root, name_chunk_type);
    const auto mesh_chunks = ChunkUtils::get_all_with_type(*root, mesh_chunk_type);
    const auto index_chunks = ChunkUtils::get_all_with_type(*root, index_chunk_type);
    std::cout << "Found " << name_chunks.size() << " name chunks." << std::endl;
    std::cout << "Found " << mesh_chunks.size() << " mesh chunks." << std::endl;
    std::cout << "Found " << index_chunks.size() << " index chunks." << std::endl;

    // TODO: Not 100% convinced yet that the name parsing is correct. It seems to be, but double check.

    // Create the out folder if it does not exist yet
    try {
        std::filesystem::path out_folder("./out");
        std::filesystem::create_directory(out_folder);
    } catch (const std::exception& e) {
        std::cerr << "Failed to create output directory 'out' due to: " << e.what() << std::endl;
        return 1;
    }

    std::size_t mesh_chunk_idx = 0;
    for (const auto& mesh_chunk : mesh_chunks) {
        static constexpr auto stride = 9 * sizeof(float); // 3 vertices, 3 normals, 1 empty, 2 texcoords
        const auto& data = mesh_chunk->data;
        std::vector<Vec3> normals;
        std::vector<Vec2> texture_coordinates;
        std::vector<Vec3> vertices;
        std::vector<Face> faces;

        // Mesh chunks start with padding bytes so we need to seek to the first vertex
        const auto first_vertex_offset = mesh_chunk->find_first_non_padding_byte_offset();
        if (!first_vertex_offset) {
            std::cout << "Mesh chunk at " << mesh_chunk->offset << " does not seem to contain vertex data, skipping it." << std::endl;
            continue;
        }
        for (std::size_t i = *first_vertex_offset; i < data.size(); i += stride) {
            float vertex_x = *reinterpret_cast<const float*>(&data.at(i));
            float vertex_y = *reinterpret_cast<const float*>(&data.at(i + 4));
            float vertex_z = *reinterpret_cast<const float*>(&data.at(i + 8));
            vertices.emplace_back(vertex_x, vertex_y, vertex_z);

            float normal_x = *reinterpret_cast<const float*>(&data.at(i + 12));
            float normal_y = *reinterpret_cast<const float*>(&data.at(i + 16));
            float normal_z = *reinterpret_cast<const float*>(&data.at(i + 20));
            normals.emplace_back(normal_x, normal_y, normal_z);
            // There is a fixed NaN value of 4 bytes here, probably padding, skip it
            float u = *reinterpret_cast<const float*>(&data.at(i + 28));
            float v = *reinterpret_cast<const float*>(&data.at(i + 32));
            texture_coordinates.emplace_back(u, v);
        }

        // Face data is stored in the index chunks in 16 bit integers, we read out 3 at a time to construct a face
        const auto& index_chunk = *index_chunks[mesh_chunk_idx];
        const auto first_index_offset = index_chunk.find_first_non_padding_byte_offset();
        if (!first_index_offset) {
            std::cout << "Index chunk at " << index_chunk.offset << " does not seem to contain index data, skipping it." << std::endl;
            continue;
        }
        // TODO: That -6 should not be necessary but for some index chunks (data_size - padding_size) is not divisible by 3
        // Figure out why that is. They do not seem to contain quads instead of triangles, it seems to be 0 bytes at the end.
        for (std::size_t i = *first_index_offset; i <= index_chunk.data.size() - 6; i += 6) {
            // Adding 1 to all of these due to OBJ indexing from 1
            std::uint16_t first_index = *reinterpret_cast<const std::uint16_t*>(&index_chunk.data.at(i)) + 1;
            std::uint16_t second_index = *reinterpret_cast<const std::uint16_t*>(&index_chunk.data.at(i + 2)) + 1;
            std::uint16_t third_index = *reinterpret_cast<const std::uint16_t*>(&index_chunk.data.at(i + 4)) + 1;
            faces.emplace_back(
                std::array<std::size_t, 3>{ first_index, second_index, third_index },
                std::array<std::size_t, 3>{ first_index, second_index, third_index },
                std::array<std::size_t, 3>{ first_index, second_index, third_index }
            );
        }

        const auto& name_chunk = *name_chunks[mesh_chunk_idx];
        const auto name = name_chunk.get_name_chunk_name_value();
        std::filesystem::path file_path("out/" + name + ".obj");
        std::cout << "Writing '" << file_path.string() << "'..." << std::endl;
        std::ofstream file_handle(file_path);
        if (!file_handle) {
            throw std::runtime_error("Failed to open file at '" + file_path.string() + "' for writing.");
        }
        for (const auto& vertex : vertices) {
            file_handle << "v " << vertex.x << " " << vertex.y << " " << vertex.z << std::endl;
        }
        for (const auto& normal : normals) {
            file_handle << "vn " << normal.x << " " << normal.y << " " << normal.z << std::endl;
        }
        for (const auto& coord : texture_coordinates) {
            file_handle << "vt " << coord.x << " " << coord.y << std::endl;
        }
        for (const auto& face : faces) {
            file_handle << "f " <<
                face.vertex_indinces[0] << "/" << face.texture_indices[0] << "/" << face.normal_indices[0] << " " <<
                face.vertex_indinces[1] << "/" << face.texture_indices[1] << "/" << face.normal_indices[1] << " " <<
                face.vertex_indinces[2] << "/" << face.texture_indices[2] << "/" << face.normal_indices[2] << std::endl;
        }
        ++mesh_chunk_idx;
    }
    return 0;
}