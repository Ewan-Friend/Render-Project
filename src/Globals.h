#pragma  once 
#include <string>
#include <vector>
#include "ModelTriangle.h"

extern const std::string COR_OBJECT;
extern const std::string COR_MATERIAL;
extern const std::string SPHERE_OBJECT;
extern const std::string BUNNY_OBJECT;
extern const std::string BUNNY_MATERIAL;

extern std::vector<ModelTriangle> SCENE_TRIANGLES;
extern std::vector<std::array<glm::vec3, 3>> SCENE_VERTEX_NORMALS;
extern int SCENE_VERSION;
