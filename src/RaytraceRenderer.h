#pragma once

#include <array>
#include <cfloat>
#include <vector>
#include <glm/glm.hpp>
#include "Camera.h"
#include "DrawingWindow.h"
#include "Renderer.h"
#include "ModelTriangle.h"
#include <RayTriangleIntersection.h>
#include "glm/detail/type_vec.hpp"

class RaytraceRenderer: public Renderer {
    public:
        RaytraceRenderer(Camera &c) : camera(c) {}
        void draw(DrawingWindow &window) override;

        std::vector<ModelTriangle> triangles;
        std::vector<std::array<glm::vec3, 3>> vertexNormals;
        // bool loaded = false;
        glm::vec3 lightPos = glm::vec3(0, 0.4, 1);
        enum class ShadingMethod { Flat = 0, Gouraud = 1, Phong = 2 };
        ShadingMethod shading = ShadingMethod::Flat;
    private:
        Camera &camera;
        float renderDistance = FLT_MAX;
        float lightStrength = 20.0f;
        int specularExponent = 256;
        float minBrightnessThreshold = 0.2f;

        glm::vec3 lastPos;
        glm::mat3 lastOri;
        bool hasRendered = false;

        float lightLevel(const glm::vec3 &point);
        bool inShadow(const glm::vec3 &point, const glm::vec3 &normal, int triIndex);
        float angleOfIncidence(const glm::vec3 &point, glm::vec3 normal);
        float specularLight(const glm::vec3 &point, glm::vec3 normal, int N, glm::vec3 view);
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
        
        // TODO: DELETE BEFORE SUBMISSION
        int seenVersion = -1;
};
