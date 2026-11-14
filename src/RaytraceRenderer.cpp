#include "RaytraceRenderer.h"
#include "DrawingWindow.h"
#include "ModelTriangle.h"
#include "Renderer.h"
#include "Globals.h"
#include "RayTriangleIntersection.h"
#include <CanvasPoint.h>
#include <Colour.h>
#include <cstdint>
#include <strings.h>
#include <vector>
#include "TextureMap.h"
#include "glm/detail/func_geometric.hpp"
#include "glm/detail/type_vec.hpp"

constexpr double PI = 3.14159265;

// Gets information about the closest intersection when firing a ray
RayTriangleIntersection getClosestIntersection(glm::vec3 cameraPosition, glm::vec3 rayDirection,
                                            const std::vector<ModelTriangle> &tris,
                                            int ignoreIndex){
    RayTriangleIntersection closestIntersection;
    closestIntersection.distanceFromCamera = FLT_MAX;

    for (size_t i = 0; i < tris.size(); i++) {
        if ((int)i == ignoreIndex) continue;
        const ModelTriangle &tri = tris[i];
        glm::vec3 e0 = tri.vertices[1] - tri.vertices[0];
        glm::vec3 e1 = tri.vertices[2] - tri.vertices[0];
        glm::vec3 SPVector = cameraPosition - tri.vertices[0];
        glm::mat3 DEMatrix(-rayDirection, e0, e1);
        glm::vec3 possibleSolution = glm::inverse(DEMatrix) * SPVector;

        float t = possibleSolution.x, u = possibleSolution.y, v = possibleSolution.z;

        if (t > 0.0001f && u >= 0 && v >= 0 && u + v <= 1 && t < closestIntersection.distanceFromCamera) {
            glm::vec3 point = cameraPosition + t * rayDirection;
            closestIntersection = RayTriangleIntersection(point, t, tri, i);
        }
    }

    return closestIntersection;
}

/// Check if the point is in shadow 
bool RaytraceRenderer::inShadow(const glm::vec3 &point, const glm::vec3 &normal, int triIndex, const glm::vec3 &light) {
    float offsetDistance = 0.001f;

    // Find vector from point to light
    glm::vec3 toLight = light - point; 
    float distanceFromLight = glm::length(toLight);
    glm::vec3 lightDirection = glm::normalize(toLight);

    // See if triangle face is "facing" the light
    bool lightSide  = glm::dot(normal, lightPos - point) > 0;
    bool viewerSide = glm::dot(normal, camera.position - point) > 0;
    if (lightSide != viewerSide) return true;

    // See if there is a triangle in the way of the light
    RayTriangleIntersection NearestCover = getClosestIntersection(
                                                point + normal * offsetDistance, 
                                                lightDirection, 
                                                triangles,
                                                triIndex
                                                );
    // "In shadow" if the object is in the way of the light
    return NearestCover.distanceFromCamera < distanceFromLight - offsetDistance;
};

/// Averages the how many "shadow rays"" hit the lights 
float RaytraceRenderer::ShadowStrength(const glm::vec3 point, const glm::vec3 &normal, int triIndex){
    int totalChecked = 0;
    int totalBlocked = 0;
    // Light are spread along the horizontal plane here (like the ceiling light)
    for (int x = -1; x <= 1; x++){
        for (int z = -1; z <= 1; z++){
            glm::vec3 smallLight = lightPos + glm::vec3(x * lightSpread, 0.0f, z * lightSpread);
            if (inShadow(point, normal, triIndex, smallLight)) totalBlocked++;
            totalChecked++;
        }
    }
    return ((float)totalBlocked / totalChecked);
}

/// Finds the angle of incidence between the normal of a face and the light
float RaytraceRenderer::angleOfIncidence(const glm::vec3 &point, glm::vec3 normal) {
    glm::vec3 toLight = lightPos - point; 
    glm::vec3 lightDirection = glm::normalize(toLight);

    return glm::dot(normal, lightDirection);
}

/// Returns the light level my measuring distance to from the light
float RaytraceRenderer::lightLevel(const glm::vec3 &point) {
    glm::vec3 toLight = lightPos - point; 
    float radius = glm::length(toLight);

    return (lightStrength * 2 / (7 * PI * (radius * radius)));
}

/// Calculate the reflection of the light source onto a surface
float RaytraceRenderer::specularLight(const glm::vec3 &point, glm::vec3 normal, int N, glm::vec3 view) {
    glm::vec3 toLight = lightPos - point; 
    glm::vec3 lightDirection = glm::normalize(toLight);

    glm::vec3 reflection = lightDirection - 2.0f * normal * (glm::dot(lightDirection, normal));

    float specular = glm::dot(reflection, view);
    return (shading != ShadingMethod::Gouraud) ? std::pow(specular, N) : 0.0f;
}

// !!! taken from utils.h but changed to vec3 !!!
/// Converts a point to corresponding barycentric coordinates from three vertices
glm::vec3 convertToBarycentricCoordinates(glm::vec3 v0, glm::vec3 v1, glm::vec3 v2, glm::vec3 r)
{
    glm::vec3 e0 = v1 - v0;
    glm::vec3 e1 = v2 - v0;
    glm::vec3 e2 = r - v0;
    float d00 = glm::dot(e0, e0);
    float d01 = glm::dot(e0, e1);
    float d11 = glm::dot(e1, e1);
    float d20 = glm::dot(e2, e0);
    float d21 = glm::dot(e2, e1);
    float denominator = d00 * d11 - d01 * d01;
    float u = (d11 * d20 - d01 * d21) / denominator;
    float v = (d00 * d21 - d01 * d20) / denominator;
    float w = 1.0f - u - v;
    return glm::vec3(u,v,w);
}

Colour RaytraceRenderer::getTextureColour(
    const ModelTriangle &tri,
    const glm::vec3 &point,
    int index
){
    std::string name = textureTriangles[index];
    TextureMap &tex = textures[name];
    glm::vec3 barycentricWeights = convertToBarycentricCoordinates(
        tri.vertices[0], tri.vertices[1], tri.vertices[2], point 
    );
    float u = barycentricWeights.x;
    float v = barycentricWeights.y;
    float w = barycentricWeights.z;

    float tx = w * tri.texturePoints[0].x + u * tri.texturePoints[1].x + v * tri.texturePoints[2].x;
    float ty = w * tri.texturePoints[0].y + u * tri.texturePoints[1].y + v * tri.texturePoints[2].y;
    int x = (int)(tx * tex.width) % tex.width;
    int y = (int)(ty * tex.height) % tex.height;

    uint32_t pixel = tex.pixels[y * tex.width + x];
    return Colour((pixel >> 16) & 0xff, (pixel >> 8) & 0xff, pixel & 0xff);
}

/// Calculates the brightness of a single point
glm::vec3 RaytraceRenderer::calculatePointColour(
    const glm::vec3 &point, 
    const glm::vec3 &normal, 
    const ModelTriangle &tri, 
    int index) 
{
    // Diffuse lighting (now uses the passed-in vertex normal)
    float incidence = angleOfIncidence(point, normal);
    float direct = (1 - ShadowStrength(point, normal, index)) * lightLevel(point);
    float brightness = std::max(minBrightnessThreshold, incidence * direct);

    // Specular lighting (fancy reflections)
    glm::vec3 view = glm::normalize(camera.position - point);
    float pureSpecular = specularLight(point, normal, specularExponent, view);
    float specular = direct * pureSpecular;

    Colour c = (textureTriangles[index].empty()) 
        ? tri.colour 
        : getTextureColour(tri, point, index);

    c.red = c.red * brightness + 255.0f * specular;
    c.green = c.green * brightness + 255.0f * specular;
    c.blue = c.blue * brightness + 255.0f * specular;

    return glm::vec3(float(c.red), float(c.green), float(c.blue));
}

/// Applies Gouraud Shading to interpolate brighness from vertex normals
glm::vec3 RaytraceRenderer::gouraudShading(
    RayTriangleIntersection intersection, 
    glm::vec3 point,
    std::array<glm::vec3, 3> vNormals)
{
    ModelTriangle tri = intersection.intersectedTriangle;
    glm::vec3 barycentricWeights = convertToBarycentricCoordinates(
        tri.vertices[0], tri.vertices[1], tri.vertices[2], point
    );
    float u = barycentricWeights.x;
    float v = barycentricWeights.y;
    float w = barycentricWeights.z;

    glm::vec3 c0 = calculatePointColour(
        tri.vertices[0], vNormals[0], tri, intersection.triangleIndex
    );
    glm::vec3 c1 = calculatePointColour(
        tri.vertices[1], vNormals[1], tri, intersection.triangleIndex
    );
    glm::vec3 c2 = calculatePointColour(
        tri.vertices[2], vNormals[2], tri, intersection.triangleIndex
    );

    return w * c0 + u * c1 + v * c2;
}

glm::vec3 RaytraceRenderer::phongShading(
    RayTriangleIntersection intersection, 
    glm::vec3 hitPoint,
    std::array<glm::vec3, 3> vNormals)
{   
    ModelTriangle tri = intersection.intersectedTriangle;
    glm::vec3 barycentricWeights = convertToBarycentricCoordinates(
        tri.vertices[0], tri.vertices[1], tri.vertices[2], hitPoint
    );
    float u = barycentricWeights.x;
    float v = barycentricWeights.y;
    float w = barycentricWeights.z;

    return glm::normalize(
        w * vNormals[0] + u * vNormals[1] + v * vNormals[2]
    );
}


void RaytraceRenderer::draw(DrawingWindow &window) {
    // camera.update();
    window.clearPixels();
    if (!loaded) {
        triangles = SCENE_TRIANGLES;
        vertexNormals = SCENE_VERTEX_NORMALS;
        textureTriangles = SCENE_TEXTURE_TRIANGLES;
        surfaces = SCENE_SURFACES;
        for (const std::string &name : textureTriangles) {
            if (!name.empty() && !textures.count(name)) {
                textures[name] = TextureMap(name);
            }
        }
        loaded = true;
    }

    #pragma omp parallel for schedule(dynamic, 16)
    for (int x = 0; x < WIDTH; x++){
        for (int y = 0; y < HEIGHT; y++){
            glm::vec3 direction = camera.pixelToRayDir(x, y); 
            RayTriangleIntersection intersection = getClosestIntersection( 
                camera.position, direction, SCENE_TRIANGLES, -1
            ); 
            if (intersection.distanceFromCamera < renderDistance){ 

                glm::vec3 point = intersection.intersectionPoint;
                ModelTriangle tri = intersection.intersectedTriangle;
                int triIndex = intersection.triangleIndex;

                glm::vec3 normal, c;
                std::array<glm::vec3, 3> vNormals = vertexNormals[intersection.triangleIndex];
                switch(shading) {
                    case ShadingMethod::Flat:
                        normal = tri.normal;
                        c = calculatePointColour(point, normal, tri, triIndex);
                        break;
                    case ShadingMethod::Gouraud:
                        normal = tri.normal;
                        c = gouraudShading(intersection, point, vNormals);
                        break;
                    case ShadingMethod::Phong:
                        normal = phongShading(intersection, point, vNormals);
                        c = calculatePointColour(point, normal, tri, triIndex);
                        break;
                }
                uint32_t r = (uint32_t)std::min(255.0f, std::max(0.0f, c.r));
                uint32_t g = (uint32_t)std::min(255.0f, std::max(0.0f, c.g));
                uint32_t b = (uint32_t)std::min(255.0f, std::max(0.0f, c.b));

                uint32_t colour = (255 << 24) + (r << 16) + (g << 8) + b;

                window.setPixelColour(x, y, colour);
            }
        }
    }
}
