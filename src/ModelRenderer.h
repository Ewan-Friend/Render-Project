#pragma once

#include "Camera.h"
#include "DrawingWindow.h"
#include "Renderer.h"

class ModelRenderer: public Renderer {
    public:
        ModelRenderer(Camera &c) : camera(c) {}
        void draw(DrawingWindow &window) override;

        std::vector<ModelTriangle> triangles;
        bool loaded = false;
    private:
        Camera &camera;
};
