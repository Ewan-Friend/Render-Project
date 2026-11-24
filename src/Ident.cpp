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
}

constexpr float PI = 3.14159265f;
void Ident::step(Camera &camera, RaytraceRenderer &rt) {
    // =========================================================================
    // STAGE 1: SIMPLE SCENE (animatedSphere) — Wireframe -> Rasterised Fill
    // =========================================================================
    if (frame < 25) {
        currentRenderer = &wireframeRenderer;
        animateScene(frame);
        camera.isLookingAt = true;
        camera.lookTarget = glm::vec3(0.0f, 0.0f, 0.0f);
        camera.snapLook();
    }
    else if (frame < 40) {
        currentRenderer = &modelRenderer;
        animateScene(frame);
        camera.isLookingAt = true;
        camera.lookTarget = glm::vec3(0.0f, 0.0f, 0.0f);
        camera.snapLook();
    }

    // =========================================================================
    // STAGE 2: 360° ORBIT TRANSITION (Frames 40 - 99)
    // Rotates 180° behind the back wall (frames 40-69), swaps to kitchenSink()
    // at frame 70 while hidden behind the box, and orbits 180° back to reveal
    // =========================================================================
    else if (frame < 100) {
        float orbitStep = (2.0f * PI) / 60.0f; // 60 frames for a full 360° loop

        if (frame < 70) {
            // Still viewing the simple animated sphere as we orbit around the back
            animateScene(frame);
        }
        else if (frame == 70) {
            // Directly behind the back wall (pi radians): swap to complex scene
            loadScene(true);
            currentRenderer = &raytraceRenderer;
            resetRaytraceFeatures(rt);
        }

        camera.isLookingAt = true;
        camera.lookTarget = glm::vec3(0.0f, -0.1f, 0.0f);
        camera.orbitY(orbitStep);
        camera.snapLook();
    }

    // =========================================================================
    // STAGE 3: 3RD CLASS SHOWCASE (Frames 100 - 124)
    // Complex Multi-Model Scene + Ambient, Diffuse & Hard Shadows
    // =========================================================================
    else if (frame < 125) {
        camera.isLookingAt = true;
        camera.lookTarget = glm::vec3(0.0f, -0.1f, 0.0f);
        camera.orbitY(0.012f);
        camera.snapLook();
    }

    // =========================================================================
    // STAGE 4: 2.2 BAND (Frames 125 - 174)
    // Textures + Specular Spot + Gouraud -> Coarse Soft Shadows
    // =========================================================================
    else if (frame < 150) {
        if (frame == 125) rt.shading = RaytraceRenderer::ShadingMethod::Gouraud;
        rt.lightPos.x = std::sin((frame - 125) * 0.12f) * 0.4f;
        camera.isLookingAt = true;
        camera.lookTarget = glm::vec3(0.0f, -0.1f, 0.0f);
        camera.orbitY(-0.01f);
        camera.snapLook();
    }
    else if (frame < 175) {
        if (frame == 150) {
            rt.lightPos.x = 0.0f;
            rt.isSoftShadowOn = true;
            rt.numLights = 9;
        }
        camera.isLookingAt = true;
        camera.lookTarget = glm::vec3(0.0f, -0.1f, 0.0f);
        camera.orbitY(0.01f);
        camera.snapLook();
    }

    // =========================================================================
    // STAGE 5: 2.1 BAND (Frames 175 - 224)
    // Phong Shading + Smooth Soft Shadows + Mirrors
    // =========================================================================
    else if (frame < 200) {
        if (frame == 175) {
            rt.shading = RaytraceRenderer::ShadingMethod::Phong;
            rt.numLights = 36;
        }
        camera.isLookingAt = true;
        camera.lookTarget = glm::vec3(0.0f, -0.15f, 0.0f);
        camera.translate(glm::vec3(0.0f, 0.0f, -0.015f));
        camera.snapLook();
    }
    else if (frame < 225) {
        if (frame == 200) rt.isReflectionOn = true;
        camera.isLookingAt = true;
        camera.lookTarget = glm::vec3(0.0f, -0.15f, 0.0f);
        camera.orbitY(-0.015f);
        camera.snapLook();
    }

    // =========================================================================
    // STAGE 6: 1ST CLASS BAND (Frames 225 - 379)
    // Environment Map -> Metals -> Glass -> Photon Caustics -> Depth of Field
    // =========================================================================
    else if (frame < 250) {
        if (frame == 225) rt.isEnvironmentOn = true;
        camera.isLookingAt = true;
        camera.lookTarget = glm::vec3(0.0f, -0.1f, 0.0f);
        camera.orbitY(0.018f);
        camera.snapLook();
    }
    else if (frame < 280) {
        if (frame == 250) rt.isMetalOn = true;
        float t = (frame - 250) / 30.0f;
        rt.metalFuzz = 0.02f + t * 0.25f;
        camera.isLookingAt = true;
        camera.lookTarget = glm::vec3(0.0f, -0.1f, 0.0f);
        camera.orbitY(-0.012f);
        camera.snapLook();
    }
    else if (frame < 310) {
        if (frame == 280) {
            rt.metalFuzz = 0.08f;
            rt.isGlassOn = true;
        }
        camera.isLookingAt = true;
        camera.lookTarget = glm::vec3(0.0f, -0.2f, 0.0f);
        camera.orbitY(0.015f);
        camera.snapLook();
    }
    else if (frame < 345) {
        if (frame == 310) rt.isPhotonOn = true;
        camera.isLookingAt = true;
        camera.lookTarget = glm::vec3(0.0f, -0.35f, 0.0f);
        camera.orbitY(-0.012f);
        camera.snapLook();
    }
    else if (frame < 380) {
        if (frame == 345) {
            rt.isDepthOn = true;
            rt.aperture = 0.08f;
        }
        float t = (frame - 345) / 35.0f;
        camera.focalDistance = 2.6f + t * 1.4f;
        camera.isLookingAt = true;
        camera.lookTarget = glm::vec3(0.0f, -0.2f, 0.0f);
        camera.snapLook();
    }

    // =========================================================================
    // STAGE 7: FINAL COMPLEX ANIMATION (Frames 380+)
    // =========================================================================
    else {
        if (frame == 380) {
            rt.aperture = 0.04f;
            camera.focalDistance = 3.5f;
        }
        animateScene(frame - 380);
        camera.isLookingAt = true;
        camera.lookTarget = glm::vec3(0.0f, -0.15f, 0.0f);
        camera.orbitY(0.01f);
        camera.snapLook();
    }

    frame++;
    redraw = true;
}
