#include "Colour.h"
#include "Globals.h"
#include "RasterisedRenderer.h"
#include "Renderer.h"
#include "RedNoiseRenderer.h"
#include "BlueNoiseRenderer.h"
#include "ColourSpectrumRenderer.h"
#include "RasterisedRenderer.h"
#include "WireframeRenderer.h"
#include "ModelRenderer.h"
#include "RaytraceRenderer.h"
#include "Camera.h"
#include "ObjParser.h"
#include "glm/detail/func_trigonometric.hpp"
#include "glm/detail/type_vec.hpp"
#include <SDL_keycode.h>
#include <string>
#include <vector>

// Define globals for WIDTH and HEIGHT of the window (can be accessed from any renderer)
// Stable: WIDTH = 320, HEIGHT = 240
extern const int WIDTH = 320;
extern const int HEIGHT = 240;

// Define global object and materials files 
extern const std::string COR_OBJECT = "./cornell-box.obj";
extern const std::string COR_MATERIAL = "./cornell-box.mtl";
extern const std::string SPHERE_OBJECT = "./sphere.obj";

DrawingWindow window = DrawingWindow(WIDTH, HEIGHT);
Camera camera;

RedNoiseRenderer redNoise = RedNoiseRenderer();
BlueNoiseRenderer blueNoise = BlueNoiseRenderer();
ColourSpectrumRenderer colourSpectrum = ColourSpectrumRenderer();
RasterisedRenderer rasterisedRenderer = RasterisedRenderer();
ModelRenderer modelRenderer = ModelRenderer(camera);
RaytraceRenderer raytraceRenderer = RaytraceRenderer(camera);
WireframeRenderer wireframeRenderer = WireframeRenderer(camera);

Renderer* currentRenderer = &raytraceRenderer;
bool savingFrames = false;
int frameCounter = 0;

// Control variables
float cameraStepSize = 0.25;
float lightStepSize = 0.1;
float theta_step = glm::radians(5.0f);

// TODO: DELETE MODEL CHANGER BEFORE SUBMISSION
std::vector<ModelTriangle> SCENE_TRIANGLES;
int SCENE_VERSION = 0;
struct ModelEntry { std::string obj; std::string mtl; };
std::vector<ModelEntry> MODELS = {
    { COR_OBJECT,    COR_MATERIAL },
    { SPHERE_OBJECT, "" },
};
int currentModel = 0;
void loadScene(int index) {
    const ModelEntry &m = MODELS[index];
    if (m.mtl.empty()) SCENE_TRIANGLES = parseObj(m.obj);
    else               SCENE_TRIANGLES = parseObj(m.obj, parseMat(m.mtl));
    SCENE_VERSION++;
}

void handleEvent(SDL_Event event, DrawingWindow &window) {
	if (event.type == SDL_KEYDOWN) {
        // TODO: MAKE A HELPER FUNCTION FOR THIS
        if (event.key.keysym.sym == SDLK_l) {
            camera.isLookingAt = !camera.isLookingAt; 
            camera.lookAt(camera.position);
        }
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
        else if (event.key.keysym.sym == SDLK_j) raytraceRenderer.lightPos.x -= lightStepSize;
        else if (event.key.keysym.sym == SDLK_l) raytraceRenderer.lightPos.x += lightStepSize;
        else if (event.key.keysym.sym == SDLK_k) raytraceRenderer.lightPos.y -= lightStepSize;
        else if (event.key.keysym.sym == SDLK_i) raytraceRenderer.lightPos.y += lightStepSize;
        else if (event.key.keysym.sym == SDLK_p) raytraceRenderer.lightPos.z -= lightStepSize;
        else if (event.key.keysym.sym == SDLK_o) raytraceRenderer.lightPos.z += lightStepSize;
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
        //TODO: DELETE MODEL CHANGER BEFORE SUBMISSION
        else if (event.key.keysym.sym == SDLK_m) {
            currentModel = (currentModel + 1) % MODELS.size();
            loadScene(currentModel);
        }
	}
}


int main(int argc, char *argv[]) {
	SDL_Event event;
	while (true) {
		// We MUST poll for events - otherwise the window will freeze !
		if (window.pollForInputEvents(event)) handleEvent(event, window);
		// In C++ we can't use "." notation for dynamic dispatch, so we have to use -> instead :o(
		currentRenderer->draw(window);
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
