#include "WireframeRenderer.h"
#include "DrawingWindow.h"
#include "RasterisedRenderer.h"
#include "ObjParser.h"
#include "Globals.h"
#include <CanvasPoint.h>
#include <Colour.h>

void WireframeRenderer::draw(DrawingWindow &window) {
    window.clearPixels();
    RasterisedRenderer::clearDepth();
    camera.update();

    //if (!loaded){
        ////auto pallete = parseMat(COR_MATERIAL);
        ////triangles = parseObj(COR_OBJECT, pallete);
        //triangles = parseObj(SPHERE_OBJECT);
        //loaded = true;
    //}

    for (ModelTriangle &tri : SCENE_TRIANGLES){
        CanvasPoint v0, v1, v2;
        v0 = camera.projectVertexOntoCanvasPoint(tri.vertices[0]);
        v1 = camera.projectVertexOntoCanvasPoint(tri.vertices[1]);
        v2 = camera.projectVertexOntoCanvasPoint(tri.vertices[2]);
        RasterisedRenderer::createTriangle(CanvasTriangle(v0, v1, v2), tri.colour, window);
    }
}
