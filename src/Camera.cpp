#include "Camera.h"
#include <cmath>

extern const int WIDTH;
extern const int HEIGHT;

/// Look at a point in space
glm::mat3 Camera::lookAt(const glm::vec3 &from, const glm::vec3 &point){
    glm::vec3 forward = glm::normalize(from - point);
    glm::vec3 right = glm::normalize(glm::cross(glm::vec3(0, 1, 0), forward));
    glm::vec3 up = glm::cross(forward, right);
    return glm::mat3(right, up, forward);
}

/// Rotate around the X axis
glm::mat3 Camera::rotX(float t) {
    return glm::mat3(1, 0, 0,
                     0, std::cos(t), -std::sin(t),
                     0, std::sin(t),  std::cos(t));
}

/// Rotate around the Y axis
glm::mat3 Camera::rotY(float t) {
    return glm::mat3(std::cos(t), 0, std::sin(t),
                     0, 1, 0,
                     -std::sin(t), 0, std::cos(t));
}

/// Move the camera
void Camera::translate(const glm::vec3 &delta) { 
    position += delta;
    if (isLookingAt) orientation = lookAt(position);
}

/// Rotate the camera
void Camera::pan(float t)  { orientation = rotY(t) * orientation; }
void Camera::tilt(float t) { orientation = rotX(t) * orientation; }

/// Orbit around the Y axis
void Camera::orbitY(float t) {
    position = rotY(t) * position;
    if (isLookingAt) orientation = lookAt(position);
}

/// Orbit around the X axis 
/// Make sure it rotates around the cameras own right side, to avoid problems (hacky?)
void Camera::orbitUp(float t) {
    glm::vec3 k = glm::normalize(glm::cross(glm::vec3(0, 1, 0), position));
    glm::vec3 np = position * std::cos(t) - glm::cross(k, position) * std::sin(t);
    if (std::fabs(np.y) < 0.99f * glm::length(np)) {
        position = np;
        if (isLookingAt) orientation = lookAt(np);
    }
}

/// Reset camera position
void Camera::reset() {
    position = glm::vec3(0, 0, 3.5f);
    orientation = glm::mat3(1.0f);
    spinning = !spinning;
}

/// Called every frame
void Camera::update() {
    // Constantly spins around the Y axis
    float theta = 0.005f;
    if (spinning) {
        glm::mat3 R = rotY(theta);
        position = R * position;
        orientation = R * orientation;
    }
}

CanvasPoint Camera::projectVertexOntoCanvasPoint(glm::vec3 vertexPosition) {
    glm::vec3 rel = glm::inverse(orientation) * (vertexPosition - position);
    float depth = -rel.z;
    if (depth <= 0.1) depth = 0.1;

    float u = planeScale * focalLength * (rel.x / depth) + (float(WIDTH) / 2);
    float v = -planeScale * focalLength * (rel.y / depth) + (float(HEIGHT) / 2);

    CanvasPoint point(u, v);
    point.depth = 1 / depth;

    return point;
}

/// Sends ray through a certain pixel
glm::vec3 Camera::pixelToRayDir(int x, int y) {
    glm::vec3 d((x - float(WIDTH) / 2) / (planeScale * focalLength),
               -(y - float(HEIGHT) / 2) / (planeScale * focalLength),
                -1.0f);
    return glm::normalize(orientation * d);
}
