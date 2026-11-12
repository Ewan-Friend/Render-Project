#pragma once

#include "Renderer.h"
#include "glm/detail/type_vec.hpp"
#include <cstddef>

class ColourSpectrumRenderer: public Renderer {
   public:
      void draw(DrawingWindow &window) override;
      static std::vector<float> interpolateSingleFloats(float from, float to, size_t numberOfValues);
      static std::vector<glm::vec3> interpolateThreeElementValues(glm::vec3 from, glm::vec3 to, size_t numberOfValues);
};
