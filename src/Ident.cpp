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

//    rt.isReflectionOn = true;
//    rt.isMetalOn = true;
//    rt.isGlassOn = true;
//    rt.isPhotonOn = false;
//    rt.isDepthOn = false;
//    rt.aperture = 0.0f;
//    rt.numLights = 36;

    camera.reset();
    while (frame < startFrame) {
        step(camera, rt);
    }
}

constexpr float PI = 3.14159265f;

const int WIRE = 0;
const int RASTER = 90;
const int BALL = 180;
const int FLIP = 310;
const int BACK = 370;
const int LOOK = 430;
const int ZOOM = 475;
const int SWEEP = 565;
const int SHADE = 655;
const int MIRROR = 775;
const int METAL = 855;
const int GLASS = 945;
const int ENV = 1065;
const int DOF = 1145;
const int PHOTON = 1265;
const int END = 1385;

static float ease(float t) {
    return 0.5f * (1.0f - std::cos(PI * t));
}

// 0 -> 1 across a segment of len frames
static float progress(int frame, int start, int len) {
    return float(frame - start) / float(len - 1);
}

void Ident::step(Camera &camera, RaytraceRenderer &rt) {
    if (frame == 0) {
        rt.shading = RaytraceRenderer::ShadingMethod::Phong;
        rt.isEnvironmentOn = true;
    }

    // Wireframe half orbit
    if (frame < RASTER) {
        currentRenderer = &wireframeRenderer;
        camera.isLookingAt = true;
        camera.lookTarget = glm::vec3(0.0f);
        camera.orbitY((2.0f * PI) / 180.0f);
        camera.snapLook();
    }
    // Rasterised half orbit
    else if (frame < BALL) {
        currentRenderer = &modelRenderer;
        camera.orbitY((2.0f * PI) / 180.0f);
        camera.snapLook();
    }
    // Raytraced ball bounce, with a slow orbit
    else if (frame < FLIP) {
        if (frame == BALL) {
//             currentRenderer = &raytraceRenderer;
            rt.isReflectionOn = true;
            rt.isMetalOn = true;
            rt.isGlassOn = true;
        }
        animateScene(frame - BALL);
        static float prevBallSway = 0.0f;
        if (frame == BALL) prevBallSway = 0.0f;
        float t = progress(frame, BALL, FLIP - BALL);
        float sway = glm::radians(20.0f) * std::sin(2.0f * PI * t);
        camera.orbitY(sway - prevBallSway);
        camera.snapLook();
        prevBallSway = sway;
    }
    // Rise over to the back side
    else if (frame < BACK) {
        camera.orbitUp(PI / 120.0f);
    }
    // Come back down to the complex model
    else if (frame < LOOK) {
        if (frame == BACK) {
            loadScene(true);
            rt.shading = RaytraceRenderer::ShadingMethod::Phong;
        }
        camera.orbitUp(-PI / 120.0f);
    }
    // Ease look target to the shadow dense area
    else if (frame < ZOOM) {
        static glm::vec3 targetStart;
        const glm::vec3 targetEnd(0.2f, -0.5f, 0.2f);
        if (frame == LOOK) targetStart = camera.lookTarget;
        float t = ease(progress(frame, LOOK, 45));
        camera.isLookingAt = true;
        camera.lookTarget = glm::mix(targetStart, targetEnd, t);
        camera.snapLook();
    }
    // Ease zoom in, soft shadow light count ramps up
    else if (frame < SWEEP) {
        static glm::vec3 zoomStart;
        static glm::vec3 zoomEnd;
        if (frame == ZOOM) {
            zoomStart = camera.position;
            zoomEnd = glm::mix(zoomStart, camera.lookTarget, 0.6f);
            rt.isSoftShadowOn = true;
        }
        float t = progress(frame, ZOOM, 90);
        camera.position = glm::mix(zoomStart, zoomEnd, ease(t));
        camera.snapLook();
        rt.numLights = 4 + (int)(32.0f * t);
    }
    // Look around the soft shadows
    else if (frame < SHADE) {
        static float prevAngle = 0.0f;
        if (frame == SWEEP) prevAngle = 0.0f;
        float t = progress(frame, SWEEP, 90);
        float angle = glm::radians(15.0f) * std::sin(2.0f * PI * t);
        camera.pan(angle - prevAngle);
        prevAngle = angle;
    }
    // Flat, Gouraud, Phong
    else if (frame < MIRROR) {
        if (frame == SHADE) {
            camera.reset();
            camera.isLookingAt = true;
            camera.lookTarget = glm::vec3(0.0f);
            camera.snapLook();
            rt.numLights = 16;
            rt.shading = RaytraceRenderer::ShadingMethod::Flat;
        }
        if (frame == SHADE + 40)
            rt.shading = RaytraceRenderer::ShadingMethod::Gouraud;
        if (frame == SHADE + 80)
            rt.shading = RaytraceRenderer::ShadingMethod::Phong;
    }
    // Mirrors off, then on
    else if (frame < METAL) {
        if (frame == MIRROR) rt.isReflectionOn = false;
        if (frame == MIRROR + 40) rt.isReflectionOn = true;
    }
    // Metal fuzz ramp
    else if (frame < GLASS) {
        if (frame == METAL) baseFuzz = rt.metalFuzz;
        rt.metalFuzz = 0.3f * progress(frame, METAL, 90);
    }
    // Glass off, on, then IOR ramp
    else if (frame < ENV) {
        if (frame == GLASS) {
            rt.metalFuzz = baseFuzz;
            baseIOR = rt.indexOfRefraction;
            rt.isGlassOn = false;
        }
        if (frame == GLASS + 40) rt.isGlassOn = true;
        if (frame >= GLASS + 40) {
            float t = progress(frame, GLASS + 40, 80);
            rt.indexOfRefraction = 1.1f + 0.4f * t;
        }
    }
    // Environment map off, then on
    else if (frame < DOF) {
        if (frame == ENV) {
            rt.indexOfRefraction = baseIOR;
            rt.isEnvironmentOn = false;
        }
        if (frame == ENV + 40) rt.isEnvironmentOn = true;
    }
    // Depth of field with a focus pull
    else if (frame < PHOTON) {
        if (frame == DOF) {
            rt.isDepthOn = true;
            rt.aperture = 0.08f;
        }
        float t = ease(progress(frame, DOF, 120));
        camera.focalDistance = glm::mix(2.5f, 5.5f, t);
    }
    // Photon caustics diff
    else if (frame < END) {
        if (frame == PHOTON) {
            rt.isDepthOn = false;
            rt.aperture = 0.0f;
            camera.focalDistance = 3.5f;
            rt.isPhotonOn = false;
        }
        if (frame == PHOTON + 40) {
            rt.isPhotonOn = true;
            rt.photonsBuilt = false;
        }
    }
    else {
        active = false;
        return;
    }

    // Gentle sway across the comparison section
    if (frame >= SHADE && frame < END) {
        static float prevSway = 0.0f;
        if (frame == SHADE) prevSway = 0.0f;
        float t = float(frame - SHADE) / float(END - SHADE);
        float sway = glm::radians(12.0f) * std::sin(2.0f * PI * t);
        camera.orbitY(sway - prevSway);
        camera.snapLook();
        prevSway = sway;
    }

    frame++;
    redraw = true;
}
