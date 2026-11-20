#pragma once
#include <array>
#include <string>
#include <vector>
#include "Colour.h"
#include "ModelTriangle.h"

/// Holds data for whats reflected from a face
enum class Surface { Base = 0, Mirror = 1, Metal = 2, Glass = 3 };

/// Holds basic data about the model
struct ModelData {
    std::vector<ModelTriangle> triangles;
    std::vector<std::array<glm::vec3, 3>> vertexNormals;
    std::vector<std::string> textureTriangles;
    std::vector<Surface> surfaces;
    std::vector<bool> shadings;
};

/// Holds data for whats drawn onto a face
struct Material {
    Colour colour;
    std::string texture;
};
