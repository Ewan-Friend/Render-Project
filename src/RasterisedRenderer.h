#pragma once

#include "DrawingWindow.h"
#include "Renderer.h"
#include <CanvasPoint.h>
#include <CanvasTriangle.h>
#include <Colour.h>

class RasterisedRenderer: public Renderer {
    public:
        void draw(DrawingWindow &window) override;
        static void createLine(CanvasPoint from, CanvasPoint to, Colour colour, DrawingWindow &window);
        static void createTriangle(CanvasTriangle tri, Colour color, DrawingWindow &window);
        static void createFilled(CanvasTriangle tri, Colour color, DrawingWindow &window);
        static void createTexture(CanvasTriangle tri, CanvasTriangle tex, DrawingWindow &window);
        static void clearDepth();
};
