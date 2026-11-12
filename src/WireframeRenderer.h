#pragma once

#include "Camera.h"
#include "DrawingWindow.h"
#include "Renderer.h"

class WireframeRenderer: public Renderer {
    public:
        WireframeRenderer(Camera &c) : camera(c) {}
        void draw(DrawingWindow &window) override;

        std::vector<ModelTriangle> triangles;
        bool loaded = false;
    private:
        Camera &camera;
};
