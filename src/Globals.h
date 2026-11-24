#pragma  once 
#include <string>
#include <vector>
#include "ModelTriangle.h"
#include "SceneTypes.h"
#include "WireframeRenderer.h"
#include "ModelRenderer.h"
#include "RaytraceRenderer.h"

extern const std::string COR_OBJECT;
extern const std::string COR_MATERIAL;
extern const std::string SPHERE_OBJECT;
extern const std::string BUNNY_OBJECT;
extern const std::string BUNNY_MATERIAL;
extern const std::string TEX_OBJECT;
extern const std::string TEX_MATERIAL;
extern const std::string HACK_OBJECT;
extern const std::string HACK_MATERIAL;

extern const std::string COBBLES;

extern std::vector<ModelTriangle> SCENE_TRIANGLES;
extern std::vector<ModelTriangle> SCENE_BASE_TRIANGLES;
extern std::vector<std::array<glm::vec3, 3>> SCENE_VERTEX_NORMALS;
extern std::vector<std::string> SCENE_TEXTURE_TRIANGLES;
extern std::vector<Surface> SCENE_SURFACES;
extern std::vector<bool> SCENE_SHADINGS;
extern int SCENE_VERSION;

extern int ANIM_START;
extern int ANIM_COUNT;
void animateScene(int frame);

extern Renderer* currentRenderer;
extern ModelRenderer modelRenderer;
extern RaytraceRenderer raytraceRenderer;
extern WireframeRenderer wireframeRenderer;

void loadScene(bool complex=true);

