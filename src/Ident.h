#pragma once
#include "Camera.h"
#include "RaytraceRenderer.h"
#include "SceneTypes.h"

class Ident{
public:
    Ident(bool &redrawFlag) : redraw(redrawFlag) {}

    bool active = false;
    int frame = 0;
    int startFrame = 305;

    void initialise(Camera &camera, RaytraceRenderer &rt);
    void step(Camera &camera, RaytraceRenderer &rt);
private:
    bool &redraw;
    float orbitStep;
    float moveStep;
};
