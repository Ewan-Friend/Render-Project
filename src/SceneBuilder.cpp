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
    Surface surface,
    bool shading
)
{
    ModelData data = mtlFile.empty() ? parseObj(objFile) : parseObj(objFile, parseMat(mtlFile));

    for (ModelTriangle &tri: data.triangles){
        for (int i = 0; i < 3; i++){
            tri.vertices[i] = tri.vertices[i] * scale + offset;
        }
        triangles.push_back(tri);
        surfaces.push_back(surface);
        shadings.push_back(shading);
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
        surfaces.at(i) = surface;
    }
    return *this;
}

/// Return model data from the scene
ModelData SceneBuilder::build() {
    ModelData result = {triangles, vertexNormals, textureTriangles, surfaces, shadings};
    triangles.clear();
    vertexNormals.clear();
    textureTriangles.clear();
    surfaces.clear();
    shadings.clear();
    return result;
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
        .createObject(SPHERE_OBJECT, material, offset, scale, surface, true)
        .build();
    return scene;
} 

/// SCENE DESCRIPTION
/// Same as the cornell box with the sphere, as well as a little rabbit familiar 
ModelData SceneBuilder::cornellBunnyCage() {
    ModelData scene = createObject(COR_OBJECT, COR_MATERIAL)
        .createObject(SPHERE_OBJECT, material, offset, scale, surface, true)
        .createObject(BUNNY_OBJECT, BUNNY_MATERIAL, offset, scale, surface, true)
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
ModelData SceneBuilder::cornellMirror() {
    ModelData scene = createObject(COR_OBJECT, COR_MATERIAL)
        .surfaceFace(Surface::Mirror, tallBoxFrontFace)
        .build();
    return scene;
}

/// SCENE DESCRIPTION
/// Ball wit the mirror on it
ModelData SceneBuilder::mirrorBall() {
    ModelData scene = createObject(SPHERE_OBJECT, "", glm::vec3(0), 1, Surface::Mirror)
        .createObject(SPHERE_OBJECT, "", glm::vec3(-0.65, 0.04, -0.35), 0.8, Surface::Metal, true)
        .build();
    return scene;
}

/// SCENE DESCRIPTION
/// Cornell Box wit the ball wit the mirror on it
ModelData SceneBuilder::cornellMirrorBall() {
    ModelData scene = createObject(COR_OBJECT, COR_MATERIAL)
        .createObject(SPHERE_OBJECT, "", glm::vec3(-0.35, 0.04, -0.35), 0.8, Surface::Mirror, true)
        .build();
    return scene;
} 

/// SCENE DESCRIPTION
/// Cornell box with everything needed to showcase 
ModelData SceneBuilder::kitchenSink() {
    ModelData scene = createObject(TEX_OBJECT, TEX_MATERIAL)
        .surfaceFace(Surface::Mirror, tallBoxFrontFace)
        .createObject(SPHERE_OBJECT, material, glm::vec3(-0.5, -1.1, 0.5), 0.8, Surface::Glass, true)
        .createObject(SPHERE_OBJECT, material, glm::vec3(-0.3, 0.05, -0.3), 0.8, Surface::Metal, true)
        .createObject(BUNNY_OBJECT, BUNNY_MATERIAL, offset, scale, Surface::Glass, true)
        .createObject(HACK_OBJECT, HACK_MATERIAL, glm::vec3(0, 0, -0.5), 0.005)
        .build();
    return scene;
} 

/// SCENE DESCRIPTION 
/// Cornell box with a bouncing ball and back mirror
ModelData SceneBuilder::animatedSphere(){
    createObject(COR_OBJECT, COR_MATERIAL);
    surfaceFace(Surface::Mirror, backWall);
    ANIM_START = triangles.size();
    createObject(SPHERE_OBJECT, material, glm::vec3(-0.5, -1.0, 0.5), 0.4, Surface::Glass, true);
    ANIM_COUNT = triangles.size() - ANIM_START;
    return build();
}

/// SCENE DESCRIPTION
/// Hackspace logo with animation attatched
ModelData SceneBuilder::hackspace(){
    createObject(HACK_OBJECT, HACK_MATERIAL, glm::vec3(-3, 0, 0), 0.002);
    ANIM_START = 0;
    ANIM_COUNT = triangles.size();
    return build();
}
