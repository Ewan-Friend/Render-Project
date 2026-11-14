#pragma  once 
#include <string>
#include <vector>
#include "ModelTriangle.h"
#include "SceneTypes.h"

extern const std::string COR_OBJECT;
extern const std::string COR_MATERIAL;
extern const std::string SPHERE_OBJECT;
extern const std::string BUNNY_OBJECT;
extern const std::string BUNNY_MATERIAL;
extern const std::string TEX_OBJECT;
extern const std::string TEX_MATERIAL;

extern const std::string COBBLES;

extern std::vector<ModelTriangle> SCENE_TRIANGLES;
extern std::vector<std::array<glm::vec3, 3>> SCENE_VERTEX_NORMALS;
extern std::vector<std::string> SCENE_TEXTURE_TRIANGLES;
extern std::vector<Surface> SCENE_SURFACES;
extern int SCENE_VERSION;
