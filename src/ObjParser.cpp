// ObjParser.cpp
#include "ObjParser.h"
#include "Colour.h"
#include "ModelTriangle.h"
#include "glm/detail/func_geometric.hpp"
#include "glm/detail/type_vec.hpp"
#include <cstddef>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

/// Parse object files into a set of trian
std::vector<ModelTriangle> parseObj(const std::string &filename, const std::unordered_map<std::string, Colour> &pallete){

    std::vector<ModelTriangle> triangles;
    std::string line;
    std::vector<glm::vec3> vertices;
    Colour colour(255, 0, 0);

    std::ifstream ObjFile(filename);

    const float scale_factor = 0.35;
    
    while (std::getline(ObjFile, line)){
        std::istringstream ss(line);
        std::string token;
        ss >> token;
        if (token.empty()) continue;

        if (token == "usemtl") {
            std::string material;
            ss >> material;
            colour = pallete.at(material);
        }
        if (token == "v") {
            float x, y, z;
            ss >> x >> y >> z;
            vertices.push_back(glm::vec3(x * scale_factor, y * scale_factor, z * scale_factor));
        }
        if (token == "f") {
            std::string a, b, c; 
            ss >> a >> b >> c;
            int ai = std::stoi(a.substr(0, a.find("/"))) - 1;
            int bi = std::stoi(b.substr(0, b.find("/"))) - 1;
            int ci = std::stoi(c.substr(0, c.find("/"))) - 1;

            glm::vec3 va = vertices[ai];
            glm::vec3 vb = vertices[bi];
            glm::vec3 vc = vertices[ci];

            glm::vec3 normal = glm::normalize(glm::cross((vb - va), (vc - va)));

            ModelTriangle tri(va, vb, vc, colour);
            tri.normal = normal;

            triangles.push_back(tri);
        }
    }

    return triangles;
}

/// Parse material files into a pallete
std::unordered_map<std::string, Colour> parseMat(const std::string &filename){
    std::unordered_map<std::string, Colour> pallete;
    std::string line, name;
    std::ifstream ObjFile(filename);

    while (std::getline(ObjFile, line)){
        std::istringstream ss(line);
        std::string token;
        ss >> token;
        if (token.empty()) continue;

        if (token == "newmtl") {
            ss >> name;
        }
        if (token == "Kd") {
            float r, g, b; 
            ss >> r >> g >> b;
            pallete[name] = Colour(
                              (int)std::round(r * 255),
                              (int)std::round(g * 255),
                              (int)std::round(b * 255));
        }
    }

    return pallete;
}

/// Return vertex normals for each triangle
std::vector<std::array<glm::vec3, 3>> findVertexNormals(const std::vector<ModelTriangle> &triangles) {
    std::vector<std::array<glm::vec3, 3>> result(triangles.size());

    for (size_t i = 0; i < triangles.size(); i++){
        for (size_t j = 0; j < 3; j++){
            glm::vec3 sum(0.0f);
            for (size_t i_ = 0; i_ < triangles.size(); i_++){
                for (size_t j_ = 0; j_ < 3; j_++){
                    float errorDistance = glm::distance(triangles[i_].vertices[j_], triangles[i].vertices[j]);
                    if (errorDistance < 0.001f) sum += triangles[i_].normal;
                }
            }
            result[i][j] = glm::normalize(sum);
        }
    }
    return result;
}
