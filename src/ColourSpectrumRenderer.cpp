#include "ColourSpectrumRenderer.h"
#include "Renderer.h"
#include "Utils.h"
#include "glm/detail/type_vec.hpp"
#include <cstdio>
#include <vector>

void ColourSpectrumRenderer::draw(DrawingWindow &window) {
    window.clearPixels();

//    glm::vec3 topLeft(255, 0, 0);        // red 
//    glm::vec3 topRight(0, 0, 255);       // blue 
//    glm::vec3 bottomRight(0, 255, 0);    // green 
//    glm::vec3 bottomLeft(255, 255, 0);   // yellow

    glm::vec2 bottomLeft_(0, HEIGHT);
    glm::vec2 bottomRight_(WIDTH, HEIGHT);
    glm::vec2 topMiddle_(WIDTH / 2, 0);

    // Write some drawing code in here !
//    std::vector<glm::vec3> leftCol = ColourSpectrumRenderer::interpolateThreeElementValues(topLeft, bottomLeft, HEIGHT);
//    std::vector<glm::vec3> rightCol = ColourSpectrumRenderer::interpolateThreeElementValues(topRight, bottomRight, HEIGHT);

	for (size_t y = 0; y < window.height; y++) {
//        std::vector<glm::vec3> row = ColourSpectrumRenderer::interpolateThreeElementValues(leftCol[y],  rightCol[y], WIDTH);
		for (size_t x = 0; x < window.width; x++) {     
            glm::vec2 point(x, y);
            glm::vec3 barry = convertToBarycentricCoordinates(bottomLeft_, bottomRight_, topMiddle_, point); 

            float u = barry[0];
            float v = barry[1];
            float w = barry[2];

            float red = 0;
            float green = 0;
            float blue = 0;

            if (w >= 0 && u >= 0 && v >= 0){;
                red = (w * 255);
                green = (v * 255);
                blue = (u * 255);
            }
            else {
                red = (rand() % 256) * w;
                green = (rand() % 256) * w;
                blue = (rand() % 256) * w;
            }

			uint32_t colour = (255 << 24) + (int(red) << 16) + (int(green) << 8) + int(blue);
			window.setPixelColour(x, y, colour);
		}
	}
}

std::vector<float> ColourSpectrumRenderer::interpolateSingleFloats(
    float from, 
    float to, 
    size_t numberOfValues
) {
    std::vector<float> nums;
    float diff = (to - from) / (numberOfValues - 1);

    for (unsigned int i = 0; i < numberOfValues; i++) {
        nums.push_back(from + (diff * i));
    }

    return nums;
}

std::vector<glm::vec3> ColourSpectrumRenderer::interpolateThreeElementValues(
    glm::vec3 from, 
    glm::vec3 to, 
    size_t numberOfValues
) {
    std::vector<glm::vec3> vecs;

    std::vector<float> nums0 = interpolateSingleFloats(from[0], to[0], numberOfValues);
    std::vector<float> nums1 = interpolateSingleFloats(from[1], to[1], numberOfValues);
    std::vector<float> nums2 = interpolateSingleFloats(from[2], to[2], numberOfValues);

    for (unsigned int i = 0; i < numberOfValues; i++) {
        glm::vec3 v(nums0[i], nums1[i], nums2[i]);
        vecs.push_back(v);
    }

    return vecs;
}

