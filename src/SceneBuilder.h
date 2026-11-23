#pragma once
#include <vector>
#include <string>
#include "ModelTriangle.h"
#include "SceneTypes.h"
#include "glm/detail/type_vec.hpp"

class SceneBuilder {
public:
    // DEFAULTS
    std::string material = "";
    glm::vec3 offset = glm::vec3(0);
    int scale = 1;
    Surface surface = Surface::Base;


    std::vector<int> tallBoxFrontFace = {26, 31};
    std::vector<int> backWall = {2, 3};
    
    std::vector<ModelTriangle> triangles;
    std::vector<std::array<glm::vec3, 3>> vertexNormals;
    std::vector<std::string> textureTriangles;
    std::vector<Surface> surfaces;
    std::vector<bool> shadings;

    SceneBuilder& createObject(const std::string &objFile,
                               const std::string &mtlFile = "",
                               glm::vec3 offset = glm::vec3(0),
                               float scale = 1.0f,
                               Surface surface = Surface::Base,
                               bool shading = false
    );
    SceneBuilder& surfaceFace(Surface surface, std::vector<int> triIndexes);
    ModelData build() const; 

    // ---------- Scenes ---------
    ModelData cornellScene();
    ModelData cornellAndSphereScene();
    ModelData cornellBunnyCage();
    ModelData texturedFloor();
    ModelData cornellMirror();
    ModelData mirrorBall();
    ModelData cornellMirrorBall();
    ModelData kitchenSink();
    ModelData animatedSphere();
    ModelData hackspace();
};
