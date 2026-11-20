#include "RaytraceRenderer.h"
#include "DrawingWindow.h"
#include "ModelTriangle.h"
#include "Renderer.h"
#include "Globals.h"
#include "RayTriangleIntersection.h"
#include <CanvasPoint.h>
#include <Colour.h>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <string>
#include <strings.h>
#include <sys/types.h>
#include <vector>
#include "SceneTypes.h"
#include "TextureMap.h"
#include "glm/detail/func_geometric.hpp"
#include "glm/detail/type_vec.hpp"

constexpr double PI = 3.14159265;


/// Gets information about the closest intersection when firing a ray
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
    if (!isSoftShadowOn) return inShadow(point, normal, triIndex, lightPos);
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

//TODO: ADD GLOBAL FLAG TO TOGGLE ENVIRONMENT MAPS ON / OFF (return glm::vec3(0))
/// Converts a direction vector to cubemap pixel 
/// Concepts from https://en.wikipedia.org/wiki/Cube_mapping#Memory_addressing
glm::vec3 RaytraceRenderer::getCubemapColour(glm::vec3 dir){
    if (!isEnvironmentOn) return glm::vec3(20.0f);
    float X = std::fabs(dir.x);
    float Y = std::fabs(dir.y);
    float Z = std::fabs(dir.z);

    float bigAxis, u, v; 
    int face;

    // Check which face a specific vector is facing, as well as local u,v coordinates on said face
    if (X >= Y && X >= Z){
        bigAxis = X;
        bool positive = dir.x > 0;
        u = positive ? -dir.z : dir.z;
        v = dir.y;
        face = positive ? 0 : 1;
    }
    else if (Y >= X && Y >= Z){
        bigAxis = Y;
        bool positive = dir.y > 0;
        u = dir.x;
        v = positive ? -dir.z : dir.z;
        face = positive ? 2 : 3;
    }
    else if (Z >= X && Z >= Y){
        bigAxis = Z;
        bool positive = dir.z > 0;
        u = positive ? dir.x : -dir.x;
        v = dir.y;
        face = positive ? 4 : 5;
    }
    // Apply face and face coordinates and gets a colour from it
    const TextureMap &faceTexture = cubemapFaces.at(face);
    float fx = (u / bigAxis + 1) / 2  * faceTexture.width;
    float fy = (1 - (v / bigAxis + 1) / 2) * faceTexture.height;
    int x = std::min(std::max((int)fx, 0), (int)faceTexture.width  - 1);
    int y = std::min(std::max((int)fy, 0), (int)faceTexture.height - 1);

    uint32_t p = faceTexture.pixels[y * faceTexture.width + x];
    return glm::vec3((p >> 16) & 0xff, (p >> 8) & 0xff, p & 0xff);
}

/// Finds the texture position to convert to a colour
Colour RaytraceRenderer::getTextureColour(
    const ModelTriangle &tri,
    const glm::vec3 &point,
    int index
){
    std::string name = textureTriangles[index];
    TextureMap &tex = textures.at(name);
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

    uint32_t p = tex.pixels[y * tex.width + x];
    // Must unpack colour
    return Colour((p >> 16) & 0xff, (p >> 8) & 0xff, p & 0xff);
}

/// Generate random unit vector
glm::vec3 randomUnitVector(uint32_t &seed){
    int x, y, z;
    while (true) {
        glm::vec3 p(
            2.0f * rand_r(&seed) / RAND_MAX - 1.0f, 
            2.0f * rand_r(&seed) / RAND_MAX - 1.0f, 
            2.0f * rand_r(&seed) / RAND_MAX - 1.0f
        );
        if (glm::dot(p, p) > 0 && glm::dot(p, p) < 1){
            return glm::normalize(p);
        }
    }
}

//TODO: PHONG SHADING ENABLER
//TODO: ADD TOGGLE FOR MIRRORS
/// colour is a reflection from the objects faces, can bounce multiple times
glm::vec3 RaytraceRenderer::getReflectColour(
    const glm::vec3 &point, 
    glm::vec3 normal, 
    int index, 
    glm::vec3 view, 
    int bouncesLeft, 
    float fuzz, 
    glm::vec3 albedo, 
    uint32_t seed
) 
{
    if (bouncesLeft <= 0) return getCubemapColour(view);
    if (glm::dot(normal, view) > 0) normal = -normal;

    glm::vec3 mirrorReflection = glm::normalize(view - 2.0f * normal * (glm::dot(view, normal)));
    int samples = (fuzz > 0.0f) ? fuzzSamples : 1;
    glm::vec3 total(0.0f);

    for (int i = 0; i < samples; i++){
        glm::vec3 currentReflection = mirrorReflection;
        if (fuzz > 0){
//            float freq = 45.0f;
//            glm::vec3 perturb(
//                std::sin(point.y * freq + point.z * freq),
//                std::sin(point.z * freq + point.x * freq),
//                std::sin(point.x * freq + point.y * freq)
//            );
            // TODO: BETTER SAMPLING METHOD
            glm::vec3 perturb = randomUnitVector(seed);
            currentReflection = mirrorReflection + fuzz * perturb;
            if (dot(currentReflection, normal) <= 0){
                continue;
            }
        }
        glm::vec3 origin = point + normal * 0.001f;
        total += rayTrace(origin, currentReflection, seed, bouncesLeft - 1, index);
    }
    return total / (float)samples;
}

glm::vec3 RaytraceRenderer::getGlassColour(
    const glm::vec3 &point, 
    glm::vec3 normal, 
    int index, 
    glm::vec3 view, 
    int bouncesLeft,
    uint32_t seed
) {
    if (bouncesLeft <= 0) return getCubemapColour(view);

    bool enter = glm::dot(view, normal) < 0;

    if (!enter) normal = -normal;
    float eta1 = enter ? 1 : indexOfRefraction;
    float eta2 = enter ? indexOfRefraction : 1;

    float cosTheta = std::fmin(glm::dot(-view, normal), 1);
    float sinTheta = std::sqrt(1 - cosTheta * cosTheta);
    
    glm::vec3 directedRay;
    glm::vec3 origin;
    float refractionIndex = (enter) ? 1/indexOfRefraction : indexOfRefraction;

    bool refract = refractionIndex * sinTheta <= 1;
    if (!refract){
        directedRay = glm::normalize(view - 2.0f * normal * (glm::dot(view, normal)));
        origin = point + normal * 0.001f;
    } else {
        glm::vec3 perpendicular = (eta1 / eta2) * (view + cosTheta * normal);
        glm::vec3 parallel = -std::sqrt(std::fabs(1 - glm::dot(perpendicular, perpendicular))) * normal;
        directedRay = perpendicular + parallel;
        origin = point - normal * 0.001f;
    }
    return rayTrace(point, directedRay, seed, bouncesLeft - 1);
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

glm::vec3 RaytraceRenderer::rayTrace(glm::vec3 origin, glm::vec3 direction, uint32_t seed, int bouncesLeft, int ignoreIndex){
    glm::vec3 c;
    RayTriangleIntersection intersection = getClosestIntersection( 
        origin, direction, triangles, ignoreIndex
    ); 
    if (intersection.distanceFromCamera < renderDistance){ 

        glm::vec3 point = intersection.intersectionPoint;
        ModelTriangle tri = intersection.intersectedTriangle;
        int triIndex = intersection.triangleIndex;
        glm::vec3 tint = glm::vec3(tri.colour.red, tri.colour.green, tri.colour.blue) / 255.0f;
        ShadingMethod method = (!shadings[triIndex]) ? ShadingMethod::Flat : shading;

        glm::vec3 normal;
        std::array<glm::vec3, 3> vNormals = vertexNormals[triIndex];
        switch(method) {
            case ShadingMethod::Flat:
                normal = tri.normal;
                if (surfaces[triIndex] == Surface::Mirror && isReflectionOn) {
                    c = getReflectColour(point, normal, triIndex, direction, bouncesLeft);
//                            * reflectiveness
//                            + calculatePointColour(point, normal, tri, triIndex) * (1 - reflectiveness);
                } else if (surfaces[triIndex] == Surface::Metal && isMetalOn) {
                    c = getReflectColour(
                        point, normal, triIndex, direction, bouncesLeft,
                        metalFuzz, tint, seed);
                } else if (surfaces[triIndex] == Surface::Glass && isGlassOn){
                    c = getGlassColour(point, normal, triIndex, direction, bouncesLeft, seed);
                } else {
                    c = calculatePointColour(point, normal, tri, triIndex);
                }
                break;

            case ShadingMethod::Gouraud:
                normal = tri.normal;
                if (surfaces[triIndex] == Surface::Mirror && isReflectionOn) {
                    c = getReflectColour(point, normal, triIndex, direction, bouncesLeft);
//                            * reflectiveness
//                            + gouraudShading(intersection, point, vNormals) * (1 - reflectiveness);
                } else if (surfaces[triIndex] == Surface::Metal && isMetalOn) {
                    c = getReflectColour(
                        point, normal, triIndex, direction, bouncesLeft,
                        metalFuzz, tint, seed);
                } else if (surfaces[triIndex] == Surface::Glass && isGlassOn){

                } else {
                    c = gouraudShading(intersection, point, vNormals);
                }
                break;

            case ShadingMethod::Phong:
                normal = phongShading(intersection, point, vNormals);
                if (surfaces[triIndex] == Surface::Mirror && isReflectionOn) {
               glm::vec3 tint = glm::vec3(
                    tri.colour.red, tri.colour.green, tri.colour.blue) / 255.0f;     c = getReflectColour(point, normal, triIndex, direction, bouncesLeft);
//                        * reflectiveness
//                        + calculatePointColour(point, normal, tri, triIndex)
//                        * (1 - reflectiveness);
                } else if (surfaces[triIndex] == Surface::Metal && isMetalOn) {
                    c = getReflectColour(
                        point, normal, triIndex, direction, bouncesLeft,
                        metalFuzz, tint, seed);
                } else if (surfaces[triIndex] == Surface::Glass && isGlassOn){
                    c = getGlassColour(point, normal, triIndex, direction, bouncesLeft, seed);
                } else {
                    c = calculatePointColour(point, normal, tri, triIndex);
                }
                break;
        }
    }
    else {
        c = getCubemapColour(direction);
    }
    return c;
}

// TODO: GET SHADER ENABLER TO SEE IF AN OBJECT SHOULD BE SHADED
void RaytraceRenderer::draw(DrawingWindow &window) {
    // camera.update();
    window.clearPixels();
    if (!loaded) {
        triangles = SCENE_TRIANGLES;
        vertexNormals = SCENE_VERTEX_NORMALS;
        textureTriangles = SCENE_TEXTURE_TRIANGLES;
        surfaces = SCENE_SURFACES;
        shadings = SCENE_SHADINGS;
        for (const std::string &name : textureTriangles) {
            if (!name.empty() && !textures.count(name)) {
                textures[name] = TextureMap(name);
            }
        }
        std::string pathPrefix = "textures/skybox/";
        std::vector<std::string> facePaths = {"posx.ppm", "negx.ppm", "posy.ppm", "negy.ppm", "posz.ppm", "negz.ppm"};
        for (int i = 0; i < 6; i++){
            cubemapFaces.push_back(TextureMap(pathPrefix + facePaths.at(i)));
        }
        loaded = true;
    }

    #pragma omp parallel for schedule(dynamic, 16)
    for (int x = 0; x < WIDTH; x++){
        for (int y = 0; y < HEIGHT; y++){
            glm::vec3 direction = glm::normalize(camera.pixelToRayDir(x, y));
            // Creates a new seed based on the pixel location (consistent randomness)
            uint32_t seed = (y * 10101) ^ (x * 80085) ^ 1234u;
            glm::vec3 c;
            // TODO: PART OF CAMERA IF FEELIN IT
            // TODO: INVESTIGATE MULTIPLE BOKEH METHODS AND SWITCH BETWEEN
            if (aperture > 0 && isDepthOn){
                glm::vec3 focalPoint = camera.position + direction * camera.focalDistance;
                glm::vec3 right = camera.orientation[0];
                glm::vec3 up = camera.orientation[1];
                glm::vec3 forward = camera.orientation[2];
                glm::vec3 total(0);
                for (int s = 0; s < depthSamples; s++){
                    // Get random point in a disc
                    float u, v;
                    do {
                        u = 2 * (float)rand_r(&seed) / RAND_MAX - 1;
                        v = 2 * (float)rand_r(&seed) / RAND_MAX - 1;
                    } while (u * u + v * v >= 1);
                    glm::vec3 lensOffset = right * (u * aperture) + up * (v * aperture); 
                    glm::vec3 origin = camera.position + lensOffset;
                    glm::vec3 sampleDirection = glm::normalize(focalPoint - origin);
                    total += rayTrace(origin, sampleDirection, seed, maxBounces);
                }
                c = total / (float)depthSamples;
            }
            else{
                c = rayTrace(camera.position, direction, seed, maxBounces);
            }
            uint32_t r = (uint32_t)std::min(255.0f, std::max(0.0f, c.r));
            uint32_t g = (uint32_t)std::min(255.0f, std::max(0.0f, c.g));
            uint32_t b = (uint32_t)std::min(255.0f, std::max(0.0f, c.b));

            uint32_t colour = (255 << 24) + (r << 16) + (g << 8) + b;
            window.setPixelColour(x, y, colour);
        }
    }
}
