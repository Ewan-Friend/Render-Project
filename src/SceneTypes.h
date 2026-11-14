#pragma once
#include <array>
#include <string>
#include <vector>
#include "Colour.h"
#include "ModelTriangle.h"

enum class Surface { Base = 0, Mirror = 1 };

struct ModelData {
    std::vector<ModelTriangle> triangles;
    std::vector<std::array<glm::vec3, 3>> vertexNormals;
    std::vector<std::string> textureTriangles;
    std::vector<Surface> surfaces;
};

struct Material {
    Colour colour;
    std::string texture;
};
