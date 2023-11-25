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
    float deltaTime, lastFrame;
    u32 light_vao_;
    glm::vec3 light_pos_ = {1.2f, 1.0f, 2.0f};
    // clang-format off
    Vec<float> cube_vertexes_ = {
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,

        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
    };
    static void onFrameResizeCallBack(GLFWwindow* window, int width, int height){
        glViewport(0, 0, width, height);
        TestLightWindow* self = reinterpret_cast<TestLightWindow*>(glfwGetWindowUserPointer(window));
        self->width_ = width;
        self->height_ = height;
    }
    // clang-format on

    void init() override {
        IMGUI_CHECKVERSION();

        glfwSetWindowUserPointer(window_, reinterpret_cast<void*>(this));
        glfwSetFramebufferSizeCallback(window_, onFrameResizeCallBack);

        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls

        io.FontGlobalScale = 1.0;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

        // Setup Platform/Renderer backends
        ImGui_ImplGlfw_InitForOpenGL(window_,
                                     true);  // Second param install_callback=true will install
        // GLFW callbacks and chain to existing ones.
        ImGui_ImplOpenGL3_Init("#version 330");
        glEnable(GL_DEPTH_TEST);
        lamp_shader_.reset(new Shader("./media/shaders/learning.ogl/cube_light.vs",
                                      "./media/shaders/learning.ogl/cube_light.fs"));

        cube_shader_.reset(new Shader("./media/shaders/learning.ogl/cube_notexture.vs",
                                      "./media/shaders/learning.ogl/cube_notexture.fs"));
        camera_.reset(new Camera({0, 0, 3}));
        glGenVertexArrays(1, &cube_vao_);
        glGenBuffers(1, &cube_vbo_);

        glBindBuffer(GL_ARRAY_BUFFER, cube_vbo_);
        glBufferData(GL_ARRAY_BUFFER,
                     cube_vertexes_.size() * sizeof(float),
                     cube_vertexes_.data(),
                     GL_STATIC_DRAW);

        glBindVertexArray(cube_vao_);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), 0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1,
                              3,
                              GL_FLOAT,
                              GL_FALSE,
                              6 * sizeof(float),
                              (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glGenVertexArrays(1, &light_vao_);
        glBindVertexArray(light_vao_);
        glBindBuffer(GL_ARRAY_BUFFER, cube_vbo_);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), 0);
        glEnableVertexAttribArray(0);
    }
    void display() override {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        glClearColor(0.2, 0.3, 0.3, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        cube_shader_->use();
        glm::mat4 cube_model(1.0);
        // cube_model = glm::rotate(cube_model, glm::radians(15.0f), {0, 1, 0});
        // cube_model = glm::rotate(cube_model, glm::radians(15.0f), {1, 0, 0});
        auto view = camera_->GetViewMatrix();
        auto projection = glm::perspective(glm::radians(camera_->Zoom),
                                           (float)width_ / (float)height_,
                                           0.1f,
                                           100.0f);
        cube_shader_->setMatrix4f("model", cube_model);
        cube_shader_->setMatrix4f("view", view);
        cube_shader_->setMatrix4f("projection", projection);
        cube_shader_->setVec3f("lightPos", light_pos_);
        cube_shader_->setVec3f("viewPos", camera_->Position);

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

        ImGui::NewFrame();
        ImGui::ShowDemoWindow();  // Show demo window! :)
        ImGui::ShowDebugLogWindow();
        ImGui::ShowStackToolWindow();
        float currentTime = glfwGetTime();
        deltaTime = currentTime - lastFrame;
        float cameraSpeed = 4.0f * deltaTime;  // 当时间差大（电脑性能差）就位移的更多
        lastFrame = currentTime;
        if (ImGui::Begin("Debug", NULL)) {
            if (ImGui::CollapsingHeader("data")) {
                ImGui::SetWindowPos(ImVec2(0, 0), ImGuiCond_Once);
                ImGui::Text("front: | %.3f, %.3f, %.3f |",
                            camera_->Front.x,
                            camera_->Front.y,
                            camera_->Front.z);
                ImGui::Text("Up: | %.3f, %.3f, %.3f |",
                            camera_->Up.x,
                            camera_->Up.y,
                            camera_->Up.z);
                ImGui::Text("position: | %.3f, %.3f, %.3f | (camera pos)",
                            camera_->Position.x,
                            camera_->Position.y,
                            camera_->Position.z);
                auto center = camera_->Position + camera_->Front;
                ImGui::Text("Position + Front: | %.3f, %.3f, %.3f | (center)",
                            center.x,
                            center.y,
                            center.z);
                ImGui::Text("right: | %.3f, %.3f, %.3f |",
                            camera_->Right.x,
                            camera_->Right.y,
                            camera_->Right.z);
                ImGui::Text("pitch: | %.3f |", camera_->Pitch);
                ImGui::Text("yaw: | %.3f |", camera_->Yaw);
            }

            ImGui::PushButtonRepeat(true);
            ImGui::Text("cam pos");
            ImGui::SameLine();
            if (ImGui::ArrowButton("#a", ImGuiDir_Left)) {
                camera_->ProcessKeyboard(LEFT, deltaTime);
            }
            ImGui::SameLine();
            if (ImGui::ArrowButton("#w", ImGuiDir_Up)) {
                camera_->ProcessKeyboard(FORWARD, deltaTime);
            }
            ImGui::SameLine();
            if (ImGui::ArrowButton("#s", ImGuiDir_Down)) {
                camera_->ProcessKeyboard(BACKWARD, deltaTime);
            }
            ImGui::SameLine();
            if (ImGui::ArrowButton("#d", ImGuiDir_Right)) {
                camera_->ProcessKeyboard(RIGHT, deltaTime);
            }
            ImGui::PopButtonRepeat();

            ImGui::PushButtonRepeat(true);
            ImGui::Text("view pos");
            ImGui::SameLine();
            float velocity = camera_->MovementSpeed * deltaTime;
            if (ImGui::ArrowButton("##a", ImGuiDir_Left)) {
                camera_->ProcessMouseMovement(-13, 0);
            }
            ImGui::SameLine();
            if (ImGui::ArrowButton("##w", ImGuiDir_Up)) {
                camera_->ProcessMouseMovement(-0, 13);
            }
            ImGui::SameLine();
            if (ImGui::ArrowButton("##s", ImGuiDir_Down)) {
                camera_->ProcessMouseMovement(-0, -13);
            }
            ImGui::SameLine();
            if (ImGui::ArrowButton("##d", ImGuiDir_Right)) {
                camera_->ProcessMouseMovement(13, 0);
            }

            ImGui::PopButtonRepeat();

            ImGui::PushButtonRepeat(true);
            ImGui::Text("light pos");
            ImGui::SameLine();
            if (ImGui::ArrowButton("##1a", ImGuiDir_Left)) {
                light_pos_.x -= velocity;
            }
            ImGui::SameLine();
            if (ImGui::ArrowButton("##1w", ImGuiDir_Up)) {
                light_pos_.z += velocity;
            }
            ImGui::SameLine();
            if (ImGui::ArrowButton("##1s", ImGuiDir_Down)) {
                light_pos_.z -= velocity;
            }
            ImGui::SameLine();
            if (ImGui::ArrowButton("##1d", ImGuiDir_Right)) {
                light_pos_.x += velocity;
            }
            ImGui::SameLine(0, 20);
            if (ImGui::Button("Up")) {
                light_pos_.y += velocity;
            }
            ImGui::SameLine();
            if (ImGui::Button("down")) {
                light_pos_.y -= velocity;
            }
            ImGui::PopButtonRepeat();

            static bool fps = false;
            static bool use_custom_lookat = false;
            if (ImGui::Button("reset")) {
                camera_.reset(new Camera({0, 0, 3}));
                camera_->setFpsCamera(fps);
                camera_->useMyLookAt(use_custom_lookat);
            }
            ImGui::SameLine();
            if (ImGui::Checkbox("fps", &fps)) {
                camera_->setFpsCamera(fps);
            }
            ImGui::SameLine();
            if (ImGui::Checkbox("use custom lookat", &use_custom_lookat)) {
                camera_->useMyLookAt(true);
            }
        }
        ImGui::End();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
};
int main(int args, char* argv[]) {
    auto window = TestLightWindow();

    window.show();
    return 0;
}
