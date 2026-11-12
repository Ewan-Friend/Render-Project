#pragma once
#include <vector>
#include <string>
#include "ModelTriangle.h"
#include "ObjParser.h"
#include "glm/detail/type_vec.hpp"

class SceneBuilder {
public:
    // TODO: CREATE AN ENUM TO CHOOSE MATERIAL
    std::vector<ModelTriangle> triangles;
    std::vector<std::array<glm::vec3, 3>> vertexNormals;

    SceneBuilder& createObject(const std::string &objFile,
                               const std::string &mtlFile = "",
                               glm::vec3 offset = glm::vec3(0),
                               float scale = 1.0f);

    ModelData build() const; 

    // ---------- Scenes ---------
    ModelData cornellScene();
    ModelData cornellAndSphereScene();
    ModelData cornellBunnyCage();

    // ModelData kitchenSink();
};
