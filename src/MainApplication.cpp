#include "Globals.h"
#include "Renderer.h"
#include "RasterisedRenderer.h"
#include "SceneBuilder.h"
#include "WireframeRenderer.h"
#include "ModelRenderer.h"
#include "RaytraceRenderer.h"
#include "Camera.h"
#include "SceneTypes.h"
#include "Ident.h"
#include "glm/detail/func_trigonometric.hpp"
#include "glm/detail/type_vec.hpp"
#include <SDL_keycode.h>
#include <string>
#include <vector>

// Define globals for WIDTH and HEIGHT of the window (can be accessed from any renderer)
// Stable: WIDTH = 320, HEIGHT = 240
extern const int WIDTH = 640;
extern const int HEIGHT = 480;

// Define global object and materials files 
extern const std::string COR_OBJECT = "./cornell-box.obj";
extern const std::string COR_MATERIAL = "./cornell-box.mtl";
extern const std::string SPHERE_OBJECT = "./sphere.obj";
extern const std::string BUNNY_OBJECT = "./models/bunny/bunny.obj";
extern const std::string BUNNY_MATERIAL = "./models/bunny/bunny.mtl";
extern const std::string TEX_OBJECT = "./models/t_cornell/textured-cornell-box.obj";
extern const std::string TEX_MATERIAL = "./models/t_cornell/textured-cornell-box.mtl";

extern const std::string COBBLES = "./models/t_cornell/texture.ppm";

DrawingWindow window = DrawingWindow(WIDTH, HEIGHT);
Camera camera;
SceneBuilder sceneBuilder = SceneBuilder();

RasterisedRenderer rasterisedRenderer = RasterisedRenderer();
ModelRenderer modelRenderer = ModelRenderer(camera);
RaytraceRenderer raytraceRenderer = RaytraceRenderer(camera);
WireframeRenderer wireframeRenderer = WireframeRenderer(camera);

Renderer* currentRenderer = &raytraceRenderer;
bool savingFrames = false;
int frameCounter = 0;
bool redraw = true;
Ident ident = Ident(redraw);

// Control variables
float cameraStepSize = 0.25;
float lightStepSize = 0.1;
float theta_step = glm::radians(5.0f);

std::vector<ModelTriangle> SCENE_TRIANGLES;
std::vector<std::array<glm::vec3, 3>> SCENE_VERTEX_NORMALS;
std::vector<std::string> SCENE_TEXTURE_TRIANGLES;
std::vector<Surface> SCENE_SURFACES;
std::vector<bool> SCENE_SHADINGS;
// PICK SCENE HERE
// TODO: ADD ARGUMENT TO PICK SCENE (MAYBE OVERKILL NO?)
void loadScene() {
    const ModelData scene = sceneBuilder.kitchenSink(); 
    SCENE_TRIANGLES = scene.triangles;
    SCENE_VERTEX_NORMALS = scene.vertexNormals;
    SCENE_TEXTURE_TRIANGLES = scene.textureTriangles;
    SCENE_SURFACES = scene.surfaces;
    SCENE_SHADINGS = scene.shadings;
}

void handleEvent(SDL_Event event, DrawingWindow &window) {
	if (event.type == SDL_KEYDOWN) {
        redraw = true;
        if (event.key.keysym.sym == SDLK_l) camera.toggleLookAt();
        else if (event.key.keysym.sym == SDLK_LEFT) camera.translate(glm::vec3(-cameraStepSize, 0, 0));
        else if (event.key.keysym.sym == SDLK_RIGHT) camera.translate(glm::vec3(cameraStepSize, 0, 0));
        else if (event.key.keysym.sym == SDLK_UP) camera.translate(glm::vec3(0, cameraStepSize, 0));
        else if (event.key.keysym.sym == SDLK_DOWN) camera.translate(glm::vec3(0, -cameraStepSize, 0));
        else if (event.key.keysym.sym == SDLK_MINUS) camera.translate(glm::vec3(0, 0, cameraStepSize));
        else if (event.key.keysym.sym == SDLK_EQUALS) camera.translate(glm::vec3(0, 0, -cameraStepSize));
        // Pan
        else if (event.key.keysym.sym == SDLK_d) camera.pan(theta_step);
        else if (event.key.keysym.sym == SDLK_a) camera.pan(-theta_step);
        // Tilt
        else if (event.key.keysym.sym == SDLK_w) camera.tilt(-theta_step);
        else if (event.key.keysym.sym == SDLK_s) camera.tilt(theta_step);
        // Orbit
        else if (event.key.keysym.sym == SDLK_PERIOD) camera.orbitY(-theta_step);
        else if (event.key.keysym.sym == SDLK_COMMA) camera.orbitY(theta_step);
        else if (event.key.keysym.sym == SDLK_SLASH) camera.orbitUp(theta_step);
        else if (event.key.keysym.sym == SDLK_QUOTE) camera.orbitUp(-theta_step);
        else if (event.key.keysym.sym == SDLK_q) camera.reset();
        // Light Positioning
        else if (event.key.keysym.sym == SDLK_z) raytraceRenderer.lightPos.x -= lightStepSize;
        else if (event.key.keysym.sym == SDLK_x) raytraceRenderer.lightPos.x += lightStepSize;
        else if (event.key.keysym.sym == SDLK_c) raytraceRenderer.lightPos.y -= lightStepSize;
        else if (event.key.keysym.sym == SDLK_v) raytraceRenderer.lightPos.y += lightStepSize;
        else if (event.key.keysym.sym == SDLK_b) raytraceRenderer.lightPos.z -= lightStepSize;
        else if (event.key.keysym.sym == SDLK_n) raytraceRenderer.lightPos.z += lightStepSize;
        else if (event.key.keysym.sym == SDLK_TAB) savingFrames = !savingFrames;
        else if (event.key.keysym.sym == SDLK_RETURN) window.toggleFullscreen();
        // Change renderers
        else if (event.key.keysym.sym == SDLK_1) currentRenderer = &raytraceRenderer;
        else if (event.key.keysym.sym == SDLK_2) currentRenderer = &modelRenderer;
        else if (event.key.keysym.sym == SDLK_3) currentRenderer = &wireframeRenderer;
        // Change shaders
        else if (event.key.keysym.sym == SDLK_4) raytraceRenderer.shading = 
            (raytraceRenderer.shading != RaytraceRenderer::ShadingMethod::Gouraud) ? 
                raytraceRenderer.shading = RaytraceRenderer::ShadingMethod::Gouraud : 
                raytraceRenderer.shading = RaytraceRenderer::ShadingMethod::Flat; 
        else if (event.key.keysym.sym == SDLK_5) raytraceRenderer.shading = 
            (raytraceRenderer.shading != RaytraceRenderer::ShadingMethod::Phong) ? 
                raytraceRenderer.shading = RaytraceRenderer::ShadingMethod::Phong : 
                raytraceRenderer.shading = RaytraceRenderer::ShadingMethod::Flat; 
        // toggle soft shadows 
        else if (event.key.keysym.sym == SDLK_h) raytraceRenderer.isSoftShadowOn = !raytraceRenderer.isSoftShadowOn; 
        // toggle reflections (mirrors) 
        else if (event.key.keysym.sym == SDLK_r) raytraceRenderer.isReflectionOn = !raytraceRenderer.isReflectionOn; 
        // toggle glass materials
        else if (event.key.keysym.sym == SDLK_g) raytraceRenderer.isGlassOn = !raytraceRenderer.isGlassOn; 
        // toggle metal surfaces
        else if (event.key.keysym.sym == SDLK_m) raytraceRenderer.isMetalOn = !raytraceRenderer.isMetalOn;
        // toggle environment maps 
        else if (event.key.keysym.sym == SDLK_e) raytraceRenderer.isEnvironmentOn = !raytraceRenderer.isEnvironmentOn;
        // ADDITIONAL
        // toggle depth of field
        else if (event.key.keysym.sym == SDLK_f) raytraceRenderer.isDepthOn = !raytraceRenderer.isDepthOn;
        else if (event.key.keysym.sym == SDLK_SPACE){ 
            ident.initialise(camera);
            savingFrames = true;
            frameCounter = 0;
        }
	}
}

int main(int argc, char *argv[]) {
	SDL_Event event;
    loadScene();
	while (true) {
        if (redraw){
            currentRenderer->draw(window);
            redraw = false;
        }
		// We MUST poll for events - otherwise the window will freeze !
		if (window.pollForInputEvents(event)) handleEvent(event, window);
		// In C++ we can't use "." notation for dynamic dispatch, so we have to use -> instead :o(
//      currentRenderer->draw(window);

        if (ident.active) ident.step(camera);
        if (!ident.active) savingFrames = false;

		// Need to ask the SDL window to render the frame, otherwise nothing actually gets shown on the screen !
		window.renderFrame();

		if(savingFrames) {
			std::string paddedCounter = std::to_string(frameCounter);
			while(paddedCounter.length() < 6) paddedCounter = "0" + paddedCounter;
			// Use whichever image file format works best on your platform (comment out the one you don't need)
			window.savePPM("saved-frames", paddedCounter + ".ppm");
			window.saveBMP("saved-frames", paddedCounter + ".bmp");
			frameCounter++;
		}
	}
}
