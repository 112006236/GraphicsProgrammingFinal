#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include "../MyCameraManager.h"
#include "../terrain/MyTerrain.h"
#include <vector>

namespace Framework {
    inline constexpr int MaxFireflies = 1000;

    struct Light {
        glm::vec4 positionRadius;
        glm::vec4 colorIntensity;
        glm::mat4 model;
    };

    struct FrameInputs {
        unsigned frame = 0;
        double time = 0;
        int lightCount = 4;
        Light lights[MaxFireflies];
    };

    extern FrameInputs frame;
    extern bool rawDebug;
    extern GLuint outputFbo;
    void update(double time, MyTerrain* terrain);
    // Conservative sphere/frustum test: returns original firefly IDs, in ascending order.
    std::vector<int> visibleFireflies(const glm::mat4& playerViewProjection);
    void begin(int width, int height);
    void end(int width, int height);
}
