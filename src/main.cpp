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
    static constexpr std::uint32_t mesh_chunk_type = 0x134B01;
    const auto mesh_chunks = ChunkUtils::get_all_with_type(*root, mesh_chunk_type);
    std::cout << "Found " << mesh_chunks.size() << " mesh chunks." << std::endl;

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

        // Vertex data in different chunks seem to start at different places so we
        // try to seek to the first normal data in every chunk to find the starting
        // position. TODO: I'm sure there is a better way to figure this out.
        const auto first_normal_offset = mesh_chunk->find_first_normal_offset();
        if (!first_normal_offset) {
            std::cout << "Mesh chunk at " << mesh_chunk->offset << " does not seem to contain any normals, skipping it." << std::endl;
            continue;
        }
        // The vertex layout starts with position data that is 12 bytes before the first normals
        const auto starting_idx = first_normal_offset.value() - 12;

        // std::size_t vertex_idx = 0;
        for (std::size_t i = starting_idx; i <= data.size() - stride; i += stride) {
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

            // For every 3rd vertex create a face
            /* TODO: Skip faces for now as we do not know yet how the geometry data is indexed
            if (++vertex_idx % 3 == 0) {
                faces.emplace_back(
                    std::array<std::size_t, 3>{ vertex_idx - 2, vertex_idx - 1, vertex_idx },
                    std::array<std::size_t, 3>{ vertex_idx - 2, vertex_idx - 1, vertex_idx },
                    std::array<std::size_t, 3>{ vertex_idx - 2, vertex_idx - 1, vertex_idx }
                );
            }
            */
        }
        std::filesystem::path file_path("out/" + std::to_string(mesh_chunk_idx++) + ".obj");
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
    }
    return 0;
}