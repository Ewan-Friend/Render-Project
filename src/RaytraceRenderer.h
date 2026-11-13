#pragma once

#include <array>
#include <cfloat>
#include <vector>
#include <glm/glm.hpp>
#include "Camera.h"
#include "DrawingWindow.h"
#include "Renderer.h"
#include "ModelTriangle.h"
#include "TextureMap.h"
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

        /// Base position of the light
        glm::vec3 lightPos = glm::vec3(0, 0.8, 1);
        /// Types of shading that the renderer can take
        enum class ShadingMethod { Flat = 0, Gouraud = 1, Phong = 2 };
        /// What the current shading method is
        ShadingMethod shading = ShadingMethod::Flat;
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

        //TODO: use these for optimisation
        glm::vec3 lastPos;
        glm::mat3 lastOri;
        bool hasRendered = false;

        float lightLevel(const glm::vec3 &point);
        bool inShadow(const glm::vec3 &point, const glm::vec3 &normal, int triIndex, const glm::vec3 &light);
        float ShadowStrength(const glm::vec3 point, const glm::vec3 &normal, int triIndex);
        float angleOfIncidence(const glm::vec3 &point, glm::vec3 normal);
        float specularLight(const glm::vec3 &point, glm::vec3 normal, int N, glm::vec3 view);
        Colour getTextureColour(
            const ModelTriangle &tri,
            const glm::vec3 &point,
            int triIndex
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

        bool loaded = false;
};
