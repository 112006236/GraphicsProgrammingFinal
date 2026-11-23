#define GLM_ENABLE_EXPERIMENTAL
#include "Framework.h"
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <cmath>

namespace Framework {
    FrameInputs frame;
    bool rawDebug = false;
    GLuint outputFbo = 0;
    static GLuint hdr = 0, depth = 0, resolve = 0, vao = 0;
    static int width = 0, height = 0;

    static GLuint shader(GLenum type, const char* code) {
        GLuint s = glCreateShader(type);
        glShaderSource(s, 1, &code, nullptr);
        glCompileShader(s);
        GLint ok;
        glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
        if (!ok)
            throw std::runtime_error("Framework shader failed");
        return s;
    }

    void update(double t, MyTerrain* terrain) {
        frame.time = t;
        const glm::vec3 palette[] = {
            {1, .65f, .15f}, {.3f, 1, .2f}, {.2f, .65f, 1}, {1, .25f, .1f}};
        for (int i = 0; i < std::clamp(frame.lightCount, 0, MaxFireflies); i++) {
            float ax, az;
            if (i < 4) {
                ax = i % 2 ? 45.f : 15.f;
                az = i / 2 ? 60.f : 20.f;
            } else if (i < 100) {
                ax = -440 + 880.f * ((i - 4) % 12) / 11;
                az = -440 + 880.f * ((i - 4) / 12) / 7;
            } else {
                ax = -440 + 880.f * ((i - 100) % 30) / 29;
                az = -440 + 880.f * ((i - 100) / 30) / 29;
            }
            float phase = 2.39996323f * i, x = ax + 2.5f * std::sin(.7 * t + phase),
                  z = az + 2.5f * std::cos(.5 * t + phase);
            float y = terrain->terrainData()->height(x, z) + 4.0f + .5f * std::sin(1.1 * t + phase);
            auto& l = frame.lights[i];
            l.positionRadius = glm::vec4(x, y, z, 8);
            l.colorIntensity = glm::vec4(palette[i % 4], 25);
            l.model = glm::translate(glm::mat4(1), glm::vec3(x, y, z)) *
                      glm::rotate(glm::mat4(1), float(.8 * t + phase),
                                  glm::normalize(glm::vec3(1, 2, 3))) *
                      glm::scale(glm::mat4(1), glm::vec3(.2f));
        }
    }

    std::vector<int> visibleFireflies(const glm::mat4& playerViewProjection) {
        const glm::mat4 rows = glm::transpose(playerViewProjection);
        glm::vec4 planes[6];
        for (int i = 0; i < 6; i++) {
            planes[i] = rows[3] + (i % 2 == 0 ? 1.f : -1.f) * rows[i / 2];
            const float length = glm::length(glm::vec3(planes[i]));
            if (length > 0)
                planes[i] /= length;
        }
        std::vector<int> visible;
        visible.reserve(std::clamp(frame.lightCount, 0, MaxFireflies));
        for (int id = 0; id < std::clamp(frame.lightCount, 0, MaxFireflies); id++) {
            const glm::vec4 sphere = frame.lights[id].positionRadius;
            bool inside = true;
            for (const auto& plane : planes)
                if (glm::dot(plane, glm::vec4(glm::vec3(sphere), 1)) < -sphere.w) {
                    inside = false;
                    break;
                }
            if (inside)
                visible.push_back(id);
        }
        return visible;
    }

    void begin(int w, int h) {
        if (w < 1 || h < 1)
            return;
        if (w != width || h != height) {
            width = w;
            height = h;
            glDeleteTextures(1, &hdr);
            glDeleteTextures(1, &depth);
            if (!outputFbo)
                glGenFramebuffers(1, &outputFbo);
            glGenTextures(1, &hdr);
            glBindTexture(GL_TEXTURE_2D, hdr);
            glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA16F, w, h);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glGenTextures(1, &depth);
            glBindTexture(GL_TEXTURE_2D, depth);
            glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT32F, w, h);
            glBindFramebuffer(GL_FRAMEBUFFER, outputFbo);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, hdr, 0);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth, 0);
            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
                throw std::runtime_error("Framework output FBO incomplete");
        }
        if (!resolve) {
            const char* vs =
                "#version 460 core\nout vec2 uv;void main(){uv=vec2((gl_VertexID<<1)&2,gl_VertexID&2);gl_Position=vec4(uv*2-1,0,1);}";
            const char* fs =
                "#version 460 core\nin vec2 uv;layout(binding=15)uniform sampler2D scene;uniform int rawDebug;out vec4 color;void main(){vec3 c=max(texture(scene,uv).rgb,vec3(0));color=vec4(rawDebug!=0?c:pow(c/(1+c),vec3(1.0/2.2)),1);}";
            resolve = glCreateProgram();
            auto v = shader(GL_VERTEX_SHADER, vs), p = shader(GL_FRAGMENT_SHADER, fs);
            glAttachShader(resolve, v);
            glAttachShader(resolve, p);
            glLinkProgram(resolve);
            glDeleteShader(v);
            glDeleteShader(p);
            glGenVertexArrays(1, &vao);
        }
        glBindFramebuffer(GL_FRAMEBUFFER, outputFbo);
        glViewport(0, 0, w, h);
        glClearColor(0, 0, 0, 0);
        glDepthMask(GL_TRUE);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void end(int w, int h) {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, w, h);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);
        glUseProgram(resolve);
        glUniform1i(glGetUniformLocation(resolve, "rawDebug"), rawDebug);
        glBindTextureUnit(15, hdr);
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glEnable(GL_DEPTH_TEST);
    }
}
