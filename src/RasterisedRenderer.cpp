#include "RasterisedRenderer.h"
#include "DrawingWindow.h"
#include "Renderer.h"
#include "TextureMap.h"
#include <CanvasPoint.h>
#include <Colour.h>
#include <cmath>
#include <cstdint>
#include <utility>
#include <vector>
#include <algorithm>

Colour WHITE("White", 255, 255, 255);
static std::vector<float> depthBuffer(WIDTH * HEIGHT, 0.0f);

void RasterisedRenderer::draw(DrawingWindow &window) {
    window.clearPixels();
    std::fill(depthBuffer.begin(), depthBuffer.end(), 0.0f);

    // CanvasTriangle tri(CanvasPoint(160, 10), CanvasPoint(300, 230), CanvasPoint(10, 150));
    // CanvasTriangle tex(CanvasPoint(195, 5),  CanvasPoint(395, 380), CanvasPoint(65, 330));
    // RasterisedRenderer::createTexture(tri, tex, window);
}


void RasterisedRenderer::createLine(CanvasPoint from, CanvasPoint to, Colour c, DrawingWindow &window) {
    uint32_t colour = (255 << 24) + (c.red << 16) + (c.green << 8) + c.blue;

    float xDiff = to.x - from.x;
    float yDiff = to.y - from.y;
    float zDiff = to.depth - from.depth;

    float numSteps = std::max(1.0f, std::ceil(std::max(std::abs(xDiff), std::abs(yDiff))));
    float xStep = xDiff/numSteps;
    float yStep = yDiff/numSteps;
    float zStep = zDiff/numSteps;

    for (float i = 0.0; i<=numSteps; i++){
        float x = std::round(from.x + (xStep*i));
        float y = std::round(from.y + (yStep*i));
        float z = from.depth + (zStep * i);

        if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT) continue;
        if (z > depthBuffer[y * WIDTH + x]){
            depthBuffer[y * WIDTH + x] = z;
            window.setPixelColour(x, y, colour);
        }
    }
}

void RasterisedRenderer::createTriangle(CanvasTriangle tri, Colour c, DrawingWindow &window) {
    RasterisedRenderer::createLine(tri.v0(), tri.v1(), c, window);
    RasterisedRenderer::createLine(tri.v0(), tri.v2(), c, window);
    RasterisedRenderer::createLine(tri.v1(), tri.v2(), c, window);
}

static void fillFlat(CanvasPoint tip, CanvasPoint a, CanvasPoint b, Colour c, DrawingWindow &window) {
    float height = a.y - tip.y;
    if (height == 0) { return; }

    int startY = int(std::round(a.y));
    int endY = int(std::round(tip.y));
    int dir = (endY - startY > 0) ? 1 : -1;

    for (int y = startY; ; y += dir) {
        float prog = std::max(0.0f, std::min(1.0f, (y - tip.y) / height));
        float a_ = tip.x + prog * (a.x - tip.x);
        float b_ = tip.x + prog * (b.x - tip.x);

        CanvasPoint Left(a_, y);
        CanvasPoint Right(b_, y);

        Left.depth = tip.depth + prog * (a.depth - tip.depth);
        Right.depth = tip.depth + prog * (b.depth - tip.depth);

        RasterisedRenderer::createLine(Left, Right, c, window);
        if (y == endY) break;
    }
}

static void texLine(CanvasPoint from,    CanvasPoint to,
                    CanvasPoint texFrom, CanvasPoint texTo,
                    TextureMap &Image, DrawingWindow &window) {

    float xDiff = to.x - from.x;
    float yDiff = to.y - from.y;
    float texxDiff= texTo.x - texFrom.x;
    float texyDiff = texTo.y - texFrom.y;

    float numSteps = std::max(abs(xDiff), abs(yDiff));
    if (numSteps == 0) {
        int tx = (int)std::round(texFrom.x), ty = (int)std::round(texFrom.y);
        int px = (int)std::round(from.x),    py = (int)std::round(from.y);
        if (tx >= 0 && tx < (int)Image.width && ty >= 0 && ty < (int)Image.height &&
            px >= 0 && px < WIDTH && py >= 0 && py < HEIGHT)
            window.setPixelColour(px, py, Image.pixels[ty * Image.width + tx]);
        return;
    }


    float xStep = xDiff/numSteps;
    float yStep = yDiff/numSteps;
    float texxStep = texxDiff/numSteps;
    float texyStep = texyDiff/numSteps;
    for (float i = 0.0; i<=numSteps; i++){
        float x = std::round(from.x + (xStep*i));
        float y = std::round(from.y + (yStep*i));
        float texx = std::round(texFrom.x + (texxStep*i));
        float texy = std::round(texFrom.y + (texyStep*i));

       if (texx < 0 || texx >= (int)Image.width  ||
            texy < 0 || texy >= (int)Image.height ||
            x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT) continue;
        uint32_t colour = Image.pixels[texy * Image.width + texx];
        uint32_t invert_colour = colour ^ 0x00FFFFFF;
        window.setPixelColour(x, y, colour);
    }
}

static void texFillFlat(CanvasPoint tip,    CanvasPoint a,    CanvasPoint b, 
                        CanvasPoint texTip, CanvasPoint texa, CanvasPoint texb, 
                        TextureMap &Image, DrawingWindow &window) {

    float height = a.y - tip.y;
    float tex_height = texa.y - texTip.y;
    if (height == 0 || tex_height == 0) { return; }

    int startY = int(std::round(a.y));
    int endY = int(std::round(tip.y));
    // int tex_startY = int(std::round(texa.y));
    // int tex_endY = int(std::round(texTip.y));

    int dir = (endY - startY > 0) ? 1 : -1;

    for (int y = startY; ; y += dir) {
        float prog = (y - tip.y) / height;
        float a_ = tip.x + prog * (a.x - tip.x);
        float b_ = tip.x + prog * (b.x - tip.x);
        float texa_x = texTip.x + prog * (texa.x - texTip.x);
        float texb_x = texTip.x + prog * (texb.x - texTip.x);
        float texa_y = texTip.y + prog * (texa.y - texTip.y);
        float texb_y = texTip.y + prog * (texb.y - texTip.y);

        CanvasPoint L(a_, y), R(b_, y);
        CanvasPoint texL(texa_x, texa_y), texR(texb_x, texb_y);
        if (L.x > R.x) { std::swap(L, R); std::swap(texL, texR); }

        texLine(CanvasPoint(a_, y), CanvasPoint(b_, y), 
                CanvasPoint(texa_x, texa_y), CanvasPoint(texb_x, texb_y),
                Image, window);
        if (y == endY) break;
    }
}

void RasterisedRenderer::createFilled(CanvasTriangle tri, Colour c, DrawingWindow &window) {

    if (tri.v0().y > tri.v1().y) { std::swap(tri.v0(), tri.v1()); }
    if (tri.v1().y > tri.v2().y) { std::swap(tri.v1(), tri.v2()); }
    if (tri.v0().y > tri.v1().y) { std::swap(tri.v0(), tri.v1()); }

    float tri_ratio = (tri.v1().y - tri.v0().y) / (tri.v2().y - tri.v0().y);

    if (tri.v2().y - tri.v0().y == 0) return;

    CanvasPoint mid( tri.v0().x + tri_ratio * (tri.v2().x - tri.v0().x), tri.v1().y ); 
    mid.depth = tri.v0().depth + tri_ratio * (tri.v2().depth - tri.v0().depth);

    fillFlat(tri.v0(), tri.v1(), mid, c, window);
    fillFlat(tri.v2(), tri.v1(), mid, c, window);

    // createTriangle(tri, WHITE, window);
}


void RasterisedRenderer::createTexture(CanvasTriangle tri, CanvasTriangle tex, DrawingWindow &window){
    TextureMap Image("./texture.ppm");

    if (tri.v0().y > tri.v1().y) {
        std::swap(tri.v0(), tri.v1()); 
        std::swap(tex.v0(), tex.v1()); 
    }
    if (tri.v1().y > tri.v2().y) {
        std::swap(tri.v1(), tri.v2()); 
        std::swap(tex.v1(), tex.v2()); 
    }
    if (tri.v0().y > tri.v1().y) {
        std::swap(tri.v0(), tri.v1()); 
        std::swap(tex.v0(), tex.v1()); 
    }

    float ratio = (tri.v1().y - tri.v0().y) / (tri.v2().y - tri.v0().y);

    CanvasPoint tri_mid( tri.v0().x + ratio * (tri.v2().x - tri.v0().x), tri.v1().y ); 

    CanvasPoint tex_mid( tex.v0().x + ratio * (tex.v2().x - tex.v0().x), 
                         tex.v0().y + ratio * (tex.v2().y - tex.v0().y)); 

    texFillFlat(tri.v0(), tri.v1(), tri_mid, tex.v0(), tex.v1(), tex_mid, Image, window);
    texFillFlat(tri.v2(), tri.v1(), tri_mid, tex.v2(), tex.v1(), tex_mid, Image, window);

    createTriangle(tri, WHITE, window);
}

void RasterisedRenderer::clearDepth() {
    std::fill(depthBuffer.begin(), depthBuffer.end(), 0.0f);
}
