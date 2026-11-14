#pragma once
#include <unordered_map>
#include <vector>
#include <string>
#include "SceneTypes.h"
#include "ModelTriangle.h"


std::unordered_map<std::string, Material> parseMat(const std::string &filename);
ModelData parseObj(const std::string &filename, const std::unordered_map<std::string, Material> &pallete);
ModelData parseObj(const std::string &filename);
std::vector<std::array<glm::vec3, 3>> findVertexNormals(const std::vector<ModelTriangle> &triangles);
