#include "chunk.h"
#include "utils.h"

#include <iostream>
#include <filesystem>

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
    std::cout << *root << std::endl;
    return 0;
}