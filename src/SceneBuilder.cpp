#include "SceneBuilder.h"
#include "ObjParser.h"
#include "Globals.h"
#include "glm/detail/type_vec.hpp"
#include <array>

/// Create a model from obj and material files
SceneBuilder& SceneBuilder::createObject(
    const std::string &objFile, 
    const std::string &mtlFile, 
    glm::vec3 offset, 
    float scale)
{
    ModelData data = mtlFile.empty() ? parseObj(objFile) : parseObj(objFile, parseMat(mtlFile));

    for (ModelTriangle &tri: data.triangles){
        for (int i = 0; i < 3; i++){
            tri.vertices[i] = tri.vertices[i] * scale + offset;
        }
        triangles.push_back(tri);
    }

    for (std::array<glm::vec3, 3> &vn: data.vertexNormals){
        vertexNormals.push_back(vn);
    }

    return *this;
}

/// Return model data from the scene
ModelData SceneBuilder::build() const {
    return {triangles, vertexNormals};
}

/// SCENE DESCRIPTION:
/// Just a cornell box...
ModelData SceneBuilder::cornellScene() {
    ModelData scene = createObject(COR_OBJECT, COR_MATERIAL)
        .build();
    return scene;
} 

/// SCENE DESCRIPTION
/// who could have guessed... its a cornell box... with a sphere...
ModelData SceneBuilder::cornellAndSphereScene() {
    ModelData scene = createObject(COR_OBJECT, COR_MATERIAL)
        .createObject(SPHERE_OBJECT, "", glm::vec3(-0.5, -1.11, 0.5), 0.8)
        .build();
    return scene;
} 

/// SCENE DESCRIPTION
/// Same as the cornell box with the sphere, as well as a little rabbit familiar 
ModelData SceneBuilder::cornellBunnyCage() {
    ModelData scene = createObject(COR_OBJECT, COR_MATERIAL)
        .createObject(SPHERE_OBJECT, "", glm::vec3(-0.5, -1.1, 0.5), 0.8)
        .createObject(BUNNY_OBJECT, BUNNY_MATERIAL)
        .build();
    return scene;
} 
