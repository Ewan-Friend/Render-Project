#pragma once
#include <unordered_map>
#include <vector>
#include <string>
#include "Colour.h"
#include "ModelTriangle.h"

std::unordered_map<std::string, Colour> parseMat(const std::string &filename);
std::vector<ModelTriangle> parseObj(const std::string &filename,
                                    const std::unordered_map<std::string, Colour> &pallete = {}
                                    );
std::vector<std::array<glm::vec3, 3>> findVertexNormals(const std::vector<ModelTriangle> &triangles);
