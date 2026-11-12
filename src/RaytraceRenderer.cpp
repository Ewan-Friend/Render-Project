#include "RaytraceRenderer.h"
#include "DrawingWindow.h"
#include "ModelTriangle.h"
#include "Renderer.h"
#include "Globals.h"
#include "ObjParser.h"
#include "RayTriangleIntersection.h"
#include <CanvasPoint.h>
#include <Colour.h>
#include <cstdint>
#include <unordered_map>
#include <vector>
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
bool RaytraceRenderer::inShadow(const glm::vec3 &point, const glm::vec3 &normal, int triIndex) {
    float offsetDistance = 0.001f;

    glm::vec3 toLight = lightPos - point; 
    float distanceFromLight = glm::length(toLight);
    glm::vec3 lightDirection = glm::normalize(toLight);

//    if (glm::dot(normal, lightDirection) <= 0.0f) return true;

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
    return NearestCover.distanceFromCamera < distanceFromLight - offsetDistance;
};

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
    return std::pow(specular, N);
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

/// Calculates the brightness of a single point
glm::vec3 RaytraceRenderer::calculatePointColour(
    const glm::vec3 &point, 
    const glm::vec3 &normal, 
    const ModelTriangle &tri, 
    int triIndex) 
{
    // Diffuse lighting (now uses the passed-in vertex normal)
    float incidence = angleOfIncidence(point, normal);
    float direct = inShadow(point, normal, triIndex) ? 0.0f : lightLevel(point);
//    float incidence = 1.0f;
    float brightness = std::max(minBrightnessThreshold, incidence * direct);

    // Specular lighting (fancy reflections)
    glm::vec3 view = glm::normalize(camera.position - point);
    float pureSpecular = specularLight(point, normal, specularExponent, view);
    float specular = direct * pureSpecular;

//    float specular = 0.0f;
    // Apply the lighting
    Colour c = tri.colour;
    c.red = c.red * brightness + 255.0f * specular;
    c.green = c.green * brightness + 255.0f * specular;
    c.blue = c.blue * brightness + 255.0f * specular;

    return glm::vec3(float(c.red), float(c.green), float(c.blue));
}

/// Applies Gouraud Shading to interpolate brighness from vertex normals
glm::vec3 RaytraceRenderer::gouraudShading(
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
    if (seenVersion != SCENE_VERSION) {
        triangles = SCENE_TRIANGLES;
        vertexNormals = findVertexNormals(triangles);
        seenVersion = SCENE_VERSION;
    }
    //if (!loaded){
//        std::unordered_map<std::string, Colour> pallete = parseMat(COR_MATERIAL);
//        triangles = parseObj(COR_OBJECT, pallete);
        //triangles = parseObj(SPHERE_OBJECT);
        //vertexNormals = findVertexNormals(triangles);
        //loaded = true;
    //}

    #pragma omp parallel for schedule(dynamic, 4)
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
