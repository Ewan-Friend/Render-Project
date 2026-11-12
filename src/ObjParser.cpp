// ObjParser.cpp
#include "ObjParser.h"
#include "Colour.h"
#include "ModelTriangle.h"
#include "glm/detail/func_geometric.hpp"
#include "glm/detail/type_vec.hpp"
#include <array>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <cstdlib>

/// Parse object files into a set of trian
ModelData parseObj(const std::string &filename, const std::unordered_map<std::string, Colour> &pallete){

    std::vector<ModelTriangle> triangles;
    std::vector<glm::vec3> definedNormals;
    std::vector<std::array<glm::vec3, 3>> vertexNormals;

    std::string line;
    std::vector<glm::vec3> vertices;
    // Fun random colour for no material!!!    
    Colour colour(255, 0, 0);
    Colour fallbacks[] = {
        Colour(255, 220, 40), Colour(255, 60, 60), Colour(60, 120, 255),
        Colour(60, 255, 60), Colour(255, 60, 255), Colour(40, 255, 255)
    };
    if (pallete.empty()){
        colour = fallbacks[rand() % 6];
    }

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
        if (token == "vn") {
            float nx, ny, nz;
            ss >> nx >> ny >> nz;
            definedNormals.push_back(glm::normalize(glm::vec3(nx, ny, nz)));
        }
        if (token == "f") {
            std::string a, b, c; 
            ss >> a >> b >> c;
            // Takes the indexes as the values before the first slash 
            int ai = std::stoi(a.substr(0, a.find("/"))) - 1;
            int bi = std::stoi(b.substr(0, b.find("/"))) - 1;
            int ci = std::stoi(c.substr(0, c.find("/"))) - 1;

            if (!definedNormals.empty()){
                int nai = std::stoi(a.substr(a.rfind("/") + 1)) - 1;Colour colour(255, 0, 0);
if (pallete.empty()) {
    Colour bright[] = {
        Colour(255, 60, 60), Colour(60, 255, 60), Colour(60, 120, 255),
        Colour(255, 220, 40), Colour(255, 60, 255), Colour(40, 255, 255)
    };
    colour = bright[rand() % 6];
}
                int nbi = std::stoi(b.substr(b.rfind("/") + 1)) - 1;
                int nci = std::stoi(c.substr(c.rfind("/") + 1)) - 1;
                vertexNormals.push_back({definedNormals[nai], definedNormals[nbi], definedNormals[nci]});
            }

            glm::vec3 va = vertices[ai];
            glm::vec3 vb = vertices[bi];
            glm::vec3 vc = vertices[ci];

            glm::vec3 normal = glm::normalize(glm::cross((vb - va), (vc - va)));

            ModelTriangle tri(va, vb, vc, colour);
            tri.normal = normal;

            triangles.push_back(tri);
        }
    }

    if (vertexNormals.empty()) {
        vertexNormals = findVertexNormals(triangles);
    }

    return ModelData{triangles, vertexNormals};
}

//TODO:: CHANGE BACK TO DEFAULT ARGUMENT
ModelData parseObj(const std::string &filename) {
    return parseObj(filename, {});
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
