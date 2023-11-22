#ifndef SRC_LEARNING_GL_TEST_LIGHT1
#define SRC_LEARNING_GL_TEST_LIGHT1
#include <memory>
#include "vgl.h"
#if WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <windows.h>
#endif
#include <functional>
#include "Camera.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "LoadShaders.h"
#include <GLFW/glfw3.h>
#include <array>
#include <cmath>
#include <iostream>
#include <stdint.h>
#include <toy/type.h>
#include "Shader.hpp"
#include "path_macro.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include "OpenglWindow.hpp"

class TestLightWindow : public OpenglWindow {
public:
    std::shared_ptr<Shader> cube_shader_;
    std::shared_ptr<Shader> lamp_shader_;
    TestLightWindow()
        : OpenglWindow() {
        height_ = 600;
        width_ = 800;
    }
    u32 cube_vao_;
    u32 cube_vbo_;
    std::shared_ptr<Camera> camera_;
    u32 cube_texture_;
    u32 light_vao_;
    glm::vec3 light_pos_ = {1.2f, 1.0f, 2.0f};
    // clang-format off
    Vec<float> cube_vertexes_ = {
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
        0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
        0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

        0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
        0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
        0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
        0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
    };
    // clang-format on
    void init() override {
        glEnable(GL_DEPTH_TEST);
        glGenVertexArrays(1, &light_vao_);

        lamp_shader_.reset(new Shader("./media/shaders/learning.ogl/cube.vs",
                                      "./media/shaders/learning.ogl/cube_light.fs"));

        int width, height, n_channels;
        stbi_set_flip_vertically_on_load(true);
        u8* data = stbi_load(CONTAINER_JPG, &width, &height, &n_channels, 0);
        if (!data) {
            throw std::runtime_error("Failed to load texture");
        }
        glGenTextures(1, &cube_texture_);
        glBindTexture(GL_TEXTURE_2D, cube_texture_);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexImage2D(GL_TEXTURE_2D,
                     0,
                     GL_RGB,
                     width,
                     height,
                     0,
                     GL_RGB,
                     GL_UNSIGNED_BYTE,
                     data);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(data);
        cube_shader_.reset(new Shader("./media/shaders/learning.ogl/cube.vs",
                                      "./media/shaders/learning.ogl/cube.fs"));
        camera_.reset(new Camera({0, 0, 3}));
        glGenVertexArrays(1, &cube_vao_);
        glGenBuffers(1, &cube_vbo_);
        glBindVertexArray(cube_vao_);
        glBindBuffer(GL_ARRAY_BUFFER, cube_vbo_);

        glBufferData(GL_ARRAY_BUFFER,
                     cube_vertexes_.size() * sizeof(float),
                     cube_vertexes_.data(),
                     GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(2,
                              2,
                              GL_FLOAT,
                              GL_FALSE,
                              5 * sizeof(float),
                              (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);

        glBindVertexArray(light_vao_);
        glBindBuffer(GL_ARRAY_BUFFER, cube_vbo_);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
        glEnableVertexAttribArray(0);
    }
    void display() override {
        glClearColor(0.2, 0.3, 0.3, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        cube_shader_->use();
        glm::mat4 cube_model(1.0);
        cube_model = glm::rotate(cube_model, glm::radians(15.0f), {0, 1, 0});
        cube_model = glm::rotate(cube_model, glm::radians(15.0f), {1, 0, 0});
        auto view = camera_->GetViewMatrix();
        auto projection = glm::perspective(glm::radians(camera_->Zoom),
                                           (float)width_ / (float)height_,
                                           0.1f,
                                           100.0f);
        cube_shader_->setMatrix4f("model", cube_model);
        cube_shader_->setMatrix4f("view", view);
        cube_shader_->setMatrix4f("projection", projection);

        glBindVertexArray(cube_vao_);
        glBindTexture(GL_TEXTURE_2D, cube_texture_);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        lamp_shader_->use();
        auto light_model = glm::mat4(1.0f);
        light_model = glm::translate(light_model, light_pos_);
        light_model = glm::scale(light_model, glm::vec3(0.2f));

        lamp_shader_->setMatrix4f("model", light_model);
        lamp_shader_->setMatrix4f("view", view);
        lamp_shader_->setMatrix4f("projection", projection);
        glBindVertexArray(light_vao_);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
};

#endif /* SRC_LEARNING_GL_TEST_LIGHT1 */
