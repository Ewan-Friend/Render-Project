#pragma once
#include <vector>
#include <string>
#include "ModelTriangle.h"
#include "SceneTypes.h"
#include "glm/detail/type_vec.hpp"

class SceneBuilder {
public:
    std::vector<ModelTriangle> triangles;
    std::vector<std::array<glm::vec3, 3>> vertexNormals;
    std::vector<std::string> textureTriangles;
    std::vector<Surface> surfaces; 

    SceneBuilder& createObject(const std::string &objFile,
                               const std::string &mtlFile = "",
                               glm::vec3 offset = glm::vec3(0),
                               float scale = 1.0f,
                               Surface surface = Surface::Base
    );
    SceneBuilder& surfaceFace(Surface surface, std::vector<int> triIndexes);
    ModelData build() const; 

    // ---------- Scenes ---------
    ModelData cornellScene();
    ModelData cornellAndSphereScene();
    ModelData cornellBunnyCage();
    ModelData texturedFloor();
    ModelData cornellMirror();

    // ModelData kitchenSink();
};
