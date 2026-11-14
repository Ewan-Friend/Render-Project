#pragma once
#include "glm/detail/type_vec.hpp"
#include <glm/glm.hpp>
#include <CanvasPoint.h>

class Camera {
public:
    /// Where the camera is in world space
    glm::vec3 position = glm::vec3(0.0f, 0.0f, 3.5f);
    /// Where the camera is facing in world space :)
    glm::mat3 orientation = glm::mat3(1.0f);
    /// Distance from camera to image plane
    float focalLength = 2.0f;
    /// Size of image plane
    float planeScale = 160.0f;
    /// auto orbiting flag
    bool spinning = false;
    /// Flag if camera movement defaults too looking at a position
    bool isLookingAt = true;
    /// Position the camera should be looking at when isLookingAt is true
    glm::vec3 lookTarget = glm::vec3(0);

    static glm::mat3 rotX(float theta);
    static glm::mat3 rotY(float theta);
    static glm::mat3 lookAt(const glm::vec3 &from, const glm::vec3 &point);
    void toggleLookAt();
    void snapLook();
    
    void translate(const glm::vec3 &delta);
    void pan(float theta);
    void tilt(float theta);
    void orbitY(float theta);
    void orbitUp(float theta);
    void reset();
    void update();

    bool inFront(glm::vec3 vertexPosition);
    CanvasPoint projectVertexOntoCanvasPoint(glm::vec3 vertexPosition);
    glm::vec3 pixelToRayDir(int x, int y);
};
