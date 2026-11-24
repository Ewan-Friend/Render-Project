#include "Ident.h"
#include "Globals.h"
#include "RaytraceRenderer.h"

static void resetRaytraceFeatures(RaytraceRenderer &rt) {
    rt.shading = RaytraceRenderer::ShadingMethod::Flat;
    rt.isSoftShadowOn = false;
    rt.isReflectionOn = false;
    rt.isMetalOn = false;
    rt.isGlassOn = false;
    rt.isEnvironmentOn = false;
    rt.isPhotonOn = false;
    rt.isDepthOn = false;
    rt.aperture = 0.0f;
}

void Ident::initialise(Camera &camera, RaytraceRenderer &rt) {
    frame = 0;
    active = true;
    redraw = true;

    loadScene(false);
    currentRenderer = &wireframeRenderer;
    resetRaytraceFeatures(rt);

    camera.reset();
    while (frame < startFrame) {
        step(camera, rt);
    }
}

constexpr float PI = 3.14159265f;
void Ident::step(Camera &camera, RaytraceRenderer &rt) {
    // Wireframe rotate

    if (frame < 30) {
        orbitStep = (2.0f * PI) / 60.0f;
        currentRenderer = &wireframeRenderer;
        camera.isLookingAt = true;
        camera.lookTarget = glm::vec3(0.0f, 0.0f, 0.0f);
        camera.orbitY(orbitStep);
        camera.snapLook();
    }
    // Rasterised rotate
    else if (frame < 60) {
        orbitStep = (2.0f * PI) / 60.0f;
        currentRenderer = &modelRenderer;
        camera.isLookingAt = true;
        camera.lookTarget = glm::vec3(0.0f, 0.0f, 0.0f);
        camera.orbitY(orbitStep);
        camera.snapLook();
    }
    // Raytraced ball bounce
    else if (frame < 190) {
        currentRenderer = &raytraceRenderer;
        animateScene(frame - 60);
    }
    // Rotate to back side
    else if (frame < 220) {
        orbitStep = PI / 60.0f;
        camera.orbitUp(orbitStep);
    }
    // Rotate back to complex model
    else if (frame < 250) {
        orbitStep = PI / 60.0f;
        loadScene(true);
        rt.shading = RaytraceRenderer::ShadingMethod::Phong;
        camera.orbitUp(-orbitStep);
    }
    // Look towards shadow dense area
    else if (frame < 265) {
        static glm::vec3 targetStart;
        const glm::vec3 targetEnd(0.2f, -0.5f, 0.2f);

        if (frame == 250) {
            targetStart = camera.lookTarget;
        }

        float t = float(frame - 250) / 14.0f;
        float ease = 0.5f * (1.0f - std::cos(PI * t));

        camera.isLookingAt = true;
        camera.lookTarget = glm::mix(targetStart, targetEnd, ease);
        camera.snapLook();
    }
    // Easing zoom into shadow dense area
    else if (frame < 310) {
        static glm::vec3 zoomStartPos;
        static glm::vec3 zoomEndPos;
        float moveStep = 0.1f;

        if (frame == 265) {
            zoomStartPos = camera.position;
            // Move 60% of the way from camera.position toward lookTarget
            zoomEndPos = glm::mix(zoomStartPos, camera.lookTarget, 0.6f);
        }

        float t = float(frame - 255) / 44.0f; // Normalized 0.0 -> 1.0
        float ease = 0.5f * (1.0f - std::cos(PI * t)); // Smooth sine ease-in-out

        camera.position = glm::mix(zoomStartPos, zoomEndPos, ease);
        camera.snapLook();
        rt.isSoftShadowOn = true;
    }
    // Turn on soft shadows and look around
    else if (frame < 340) {
        static float prevAngle = 0.0f;
        if (frame == 310) {
            rt.isSoftShadowOn = true;
            prevAngle = 0.0f;
        }

        const float maxAngle = glm::radians(15.0f); // 15° each way = 30° total cone
        float t = float(frame - 310 + 1) / 30.0f;   // 0 -> 1 over 30 frames
        float angle = maxAngle * std::sin(2.0f * PI * t);

        camera.pan(angle - prevAngle);
        prevAngle = angle;
    }


    frame++;
    redraw = true;
}
