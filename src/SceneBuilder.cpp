#include "SceneBuilder.h"
#include "ObjParser.h"
#include "Globals.h"
#include "SceneTypes.h"
#include "glm/detail/type_vec.hpp"
#include <array>
#include <string>
#include <vector>

/// Create a model from obj and material files
SceneBuilder& SceneBuilder::createObject(
    const std::string &objFile, 
    const std::string &mtlFile, 
    glm::vec3 offset, 
    float scale,
    Surface surface
)
{
    ModelData data = mtlFile.empty() ? parseObj(objFile) : parseObj(objFile, parseMat(mtlFile));

    for (ModelTriangle &tri: data.triangles){
        for (int i = 0; i < 3; i++){
            tri.vertices[i] = tri.vertices[i] * scale + offset;
        }
        triangles.push_back(tri);
        surfaces.push_back(surface);
    }

    for (std::array<glm::vec3, 3> &vn: data.vertexNormals){
        vertexNormals.push_back(vn);
    }
    for (const std::string &tx: data.textureTriangles){
        textureTriangles.push_back(tx);
    }

    return *this;
}

/// Apply a surface type to the face of a model by specifying triangle indexes 
SceneBuilder& SceneBuilder::surfaceFace(Surface surface, std::vector<int> triIndexes){
    for (int i: triIndexes){
        surfaces[i] = surface;
    }
    return *this;
}

/// Return model data from the scene
ModelData SceneBuilder::build() const {
    return {triangles, vertexNormals, textureTriangles};
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

/// SCENE DESCRIPTION
/// A cornell box, where the floor is a cobble texture
ModelData SceneBuilder::texturedFloor() {
    ModelData scene = createObject(TEX_OBJECT, TEX_MATERIAL)
        .build();
    return scene;
}

/// SCENE DESCRIPTION
/// A cornell box where one of the faces is a mirror
std::vector<int> faces(2, 3);
ModelData SceneBuilder::cornellMirror() {
    ModelData scene = createObject(COR_OBJECT, COR_MATERIAL)
        .surfaceFace(Surface::Mirror, faces)
        .build();
    return scene;
}
