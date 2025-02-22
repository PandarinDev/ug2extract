#pragma once

#include <array>
#include <cstddef>

namespace ug2e {

    struct Vec2 {

        float x;
        float y;

        Vec2(float x, float y) : x(x), y(y) {}

    };

    struct Vec3 {

        float x;
        float y;
        float z;

        Vec3(float x, float y, float z) : x(x), y(y), z(z) {}

    };

    struct Face {

        // These are all indexed from 1 (due to OBJ conventions)
        std::array<std::size_t, 3> vertex_indinces;
        std::array<std::size_t, 3> texture_indices;
        std::array<size_t, 3> normal_indices;

        Face(std::array<std::size_t, 3> vertex_indinces,
            std::array<std::size_t, 3> texture_indices,
            std::array<size_t, 3> normal_indices) :
            vertex_indinces(vertex_indinces),
            texture_indices(texture_indices),
            normal_indices(normal_indices) {}

    };

}