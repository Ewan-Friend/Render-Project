#pragma once

#include <array>
#include <cfloat>
#include <sys/types.h>
#include <vector>
#include <glm/glm.hpp>
#include "Camera.h"
#include "DrawingWindow.h"
#include "Renderer.h"
#include "ModelTriangle.h"
#include "TextureMap.h"
#include "SceneTypes.h"
#include <unordered_map>
#include <RayTriangleIntersection.h>
#include "glm/detail/type_vec.hpp"

class RaytraceRenderer: public Renderer {
    public:
        RaytraceRenderer(Camera &c) : camera(c) {}
        void draw(DrawingWindow &window) override;

        std::vector<ModelTriangle> triangles;
        std::vector<std::array<glm::vec3, 3>> vertexNormals;
        std::vector<std::string> textureTriangles;
        std::unordered_map<std::string, TextureMap> textures;
        std::vector<Surface> surfaces;
        std::vector<TextureMap> cubemapFaces;

        /// Base position of the light
        glm::vec3 lightPos = glm::vec3(0, 0.8, 1);
        /// Types of shading that the renderer can take
        enum class ShadingMethod { Flat = 0, Gouraud = 1, Phong = 2 };
        /// What the current shading method is
        ShadingMethod shading = ShadingMethod::Flat;
        /// Toggles
        bool isSoftShadowOn = true;
        bool isReflectionOn = true;
        bool isMetalOn = true;
        bool isEnvironmentOn = true;
        bool isGlassOn = true;
        bool isDepthOn = false;
    private:
        Camera &camera;
        /// The maximum distance the camera will render
        float renderDistance = FLT_MAX;
        /// How bright the light is
        float lightStrength = 32.0f;
        /// "Strength" of the specular lighting (shininess)
        int specularExponent = 256;
        /// How dark a pixel can be
        float minBrightnessThreshold = 0.20;
        /// How spread out the light source is (for soft shadows)
        float lightSpread = 0.15;
        /// How much light a mirror reflects
        float reflectiveness = 0.7f;
        /// How many times multiple mirrors can reflect light
        int maxBounces = 3;
        /// "Fuzziness" of metal
        float metalFuzz = 0.125;
        /// How many metal fuzz samples are taken to average over
        int fuzzSamples = 5;
        /// How big the lens radius is
        float aperture = 0.08;
        /// How many samples are taken for depth of field 
        int depthSamples = 16;

        //TODO: use these for optimisation
//        glm::vec3 lastPos;
//        glm::mat3 lastOri;
//        bool hasRendered = false;

        float lightLevel(const glm::vec3 &point);
        bool inShadow(const glm::vec3 &point, const glm::vec3 &normal, int triIndex, const glm::vec3 &light);
        float ShadowStrength(const glm::vec3 point, const glm::vec3 &normal, int triIndex);
        float angleOfIncidence(const glm::vec3 &point, glm::vec3 normal);
        float specularLight(const glm::vec3 &point, glm::vec3 normal, int N, glm::vec3 view);
        glm::vec3 getCubemapColour(glm::vec3 dir);
        Colour getTextureColour(
            const ModelTriangle &tri,
            const glm::vec3 &point,
            int triIndex
        );
        glm::vec3 getReflectColour(
            const glm::vec3 &point, 
            glm::vec3 normal, 
            int index, 
            glm::vec3 view,
            int bouncesLeft,
            float fuzz = 0,
            glm::vec3 albedo = glm::vec3(1.0f),
            uint32_t seed = 0
        );
        glm::vec3 calculatePointColour(
            const glm::vec3 &point, 
            const glm::vec3 &normal, 
            const ModelTriangle &tri, 
            int triIndex
        ); 
        glm::vec3 gouraudShading(
            RayTriangleIntersection intersection, 
            glm::vec3 hitPoint,
            std::array<glm::vec3, 3> vNormals
        );
        glm::vec3 phongShading(
            RayTriangleIntersection intersection, 
            glm::vec3 hitPoint,
            std::array<glm::vec3, 3> vNormals
        );
        glm::vec3 rayTrace(glm::vec3 origin, glm::vec3 direction, uint32_t seed);

        bool loaded = false;
};
