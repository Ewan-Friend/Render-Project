#pragma once
#include "Camera.h"

class Ident{
public:
    Ident(bool &redrawFlag) : redraw(redrawFlag) {}

    bool active = false;
    int frame = 0;

    void initialise(Camera &camera);
    void step(Camera &camera);
private:
    float theta = glm::radians(2.0f);
    bool &redraw;
};
