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

const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aTexCoord;

out vec3 ourColor;
out vec2 TexCoord; 
uniform mat4 model; 
uniform mat4 view; 
uniform mat4 projection; 

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    // gl_Position = vec4(aPos, 1.0);
    ourColor = aColor;
    TexCoord = vec2(aTexCoord.x, aTexCoord.y);
}
)";

const char* fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;

in vec3 ourColor;
in vec2 TexCoord;
uniform float mixValue;
uniform sampler2D ourTexture; // sampler采样器，内部数据类型

uniform sampler2D ourTexture2; // 第二个纹理

void main()
{   
    FragColor = mix(texture(ourTexture, TexCoord), texture(ourTexture2, TexCoord), mixValue); // 8:2的方式混合两个纹理
}
)";

std::function<void(GLFWwindow*)> processInput = [](GLFWwindow*) {
};

u32 vertexShader;
u32 fragmentShader;
u32 shaderProgram;

ShaderBase* shader_program_;
u32 texture1;
u32 texture2;
u32 VBOs[1];
u32 VAOs[1];
unsigned int SCR_WIDTH = 800;
unsigned int SCR_HEIGHT = 600;

Camera* camera;

static float deltaTime = 0.0f; // 当前帧与上一帧的时间差
static float lastFrame = 0.0f; // 上一帧的时间
static float mixValue = 0.1;

float lastX = SCR_WIDTH / 2, lastY = SCR_HEIGHT / 2;

float yaw = -90, pitch = 0;

float fov = 45;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    camera->ProcessMouseScroll(yoffset);
}

void imgui_init();

void (*mouse_callback)(GLFWwindow*, double, double) =
        [](GLFWwindow* window, double xpos, double ypos) {
    static bool firstMouse = true;
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }
    float xoffset = xpos - lastX; // x, ypos都是相对于窗口左上角的坐标
    float yoffset = lastY - ypos; // 注意这里是相反的，因为y坐标是从底部往顶部依次增大的
    lastX = xpos;
    lastY = ypos;

    camera->ProcessMouseMovement(xoffset, yoffset);
};

void init() {
    glEnable(GL_DEPTH_TEST);

    shader_program_ = new ShaderCompiler(vertexShaderSource, fragmentShaderSource);
    camera = new Camera({0, 0, 3});

    int width, height, n_channels;
    stbi_set_flip_vertically_on_load(true);
    u8* data = stbi_load(CONTAINER_JPG, &width, &height, &n_channels, 0);
    if (!data) {
        throw std::runtime_error("Failed to load texture");
    }
    glGenTextures(1, &texture1);
    // 它会自动把纹理赋值给片段着色器的采样器
    glBindTexture(GL_TEXTURE_2D, texture1);
    shader_program_->use();

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);

    glGenTextures(1, &texture2);
    glBindTexture(GL_TEXTURE_2D, texture2);

    data = stbi_load(AWESOMEFACE_PNG, &width, &height, &n_channels, 0);
    if (!data) {
        throw std::runtime_error("Failed to load texture");
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);

    glGenVertexArrays(1, VAOs);
    glGenBuffers(1, VBOs);

    // clang-format off
    float vertices[] = {
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,
        0.5f, -0.5f, -0.5f, 1.0f, 0.0f,
        0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
        0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,

        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
        0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 1.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 1.0f,
        -0.5f, 0.5f, 0.5f, 0.0f, 1.0f,
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,

        -0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
        -0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
        -0.5f, 0.5f, 0.5f, 1.0f, 0.0f,

        0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
        0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
        0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
        0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
        0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
        0.5f, -0.5f, -0.5f, 1.0f, 1.0f,
        0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
        0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,

        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f,
        0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
        -0.5f, 0.5f, 0.5f, 0.0f, 0.0f,
        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f
    };
    // clang-format on
    glBindVertexArray(VAOs[0]);
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[0]);

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), NULL);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(2,
                          2,
                          GL_FLOAT,
                          GL_FALSE,
                          5 * sizeof(float),
                          (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    shader_program_->use();
    shader_program_->setInt("ourTexture", 0);
    shader_program_->setInt("ourTexture2", 1);

    processInput = [](GLFWwindow* window) {
        float currentTime = glfwGetTime();
        deltaTime = currentTime - lastFrame;
        float cameraSpeed = 2.5f * deltaTime; // 当时间差大（电脑性能差）就位移的更多
        lastFrame = currentTime;
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            camera->ProcessKeyboard(FORWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            camera->ProcessKeyboard(BACKWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            camera->ProcessKeyboard(LEFT, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            camera->ProcessKeyboard(RIGHT, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
            mixValue += 0.005f; // change this value accordingly (might be too slow or too
            // fast based on system hardware)
            if (mixValue >= 1.0f)
                mixValue = 1.0f;
        }
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
            mixValue -= 0.005f; // change this value accordingly (might be too slow or too
            // fast based on system hardware)
            if (mixValue <= 0.0f)
                mixValue = 0.0f;
        }
    };
}

void display() {
    glClearColor(0.2, 0.3, 0.3, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture1);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture2);

    // 更多的立方体，每个立方体在世界坐标中的位置
    Vec<glm::vec3> cubePositions = {
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(2.0f, 5.0f, -15.0f),
        glm::vec3(-1.5f, -2.2f, -2.5f),
        glm::vec3(-3.8f, -2.0f, -12.3f),
        glm::vec3(2.4f, -0.4f, -3.5f),
        glm::vec3(-1.7f, 3.0f, -7.5f),
        glm::vec3(1.3f, -2.0f, -2.5f),
        glm::vec3(1.5f, 2.0f, -2.5f),
        glm::vec3(1.5f, 0.2f, -1.5f),
        glm::vec3(-1.3f, 1.0f, -1.5f)
    };
    shader_program_->use();
    shader_program_->setFloat("mixValue", mixValue);

    glBindVertexArray(VAOs[0]);
    for (auto i = 0ul; i < cubePositions.size(); i++) {
        glm::mat4 model(1.0);
        // 旋转，绕x周，-55度
        // model = glm::rotate(model, glm::radians(-55.0f), glm::vec3(1.f, 1.0f, 0.0f));
        model = glm::translate(model, cubePositions.at(i));

        float angle = 20.0f * i;
        if (i % 3 == 0) {
            // 第三个参数是旋转的轴向量
            model = glm::rotate(model,
                                (float)glfwGetTime() * glm::radians(50.0f),
                                glm::vec3(0.5f, 1.f, 0.0f));
        }
        else {
            // 不旋转
        }
        model = glm::rotate(model, glm::radians(angle), glm::vec3(0.f, 1.f, 1.0f));

        glm::mat4 view = camera->GetViewMatrix();

        glm::mat4 projection(1.0);
        int screenWidth = SCR_WIDTH, screenHeight = SCR_HEIGHT;
        // 1. fov角度通常为45 (Field of View)
        // 宽高比
        // 最近裁剪平面和最远裁剪平面
        projection =
                glm::perspective(glm::radians(fov),
                                 screenHeight ? ((float)(screenWidth) / (float)screenHeight) : 0.0001f,
                                 0.1f,
                                 100.0f);

        shader_program_->setMatrix4f("model", model);

        shader_program_->setMatrix4f("view", view);
        shader_program_->setMatrix4f("projection", projection);

        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
}

#ifdef _WIN32
int CALLBACK WinMain(_In_ HINSTANCE hInstance,
                     _In_ HINSTANCE hPrevInstance,
                     _In_ LPSTR lpCmdLine,
                     _In_ int nCmdShow)
#else
int main(int argc, char** argv)
#endif
{
    glfwInit();

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Triangles", NULL, NULL);

    glfwMakeContextCurrent(window);
    gl3wInit();

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO&io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls

    io.FontGlobalScale = 2;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window,
                                 true); // Second param install_callback=true will install
    // GLFW callbacks and chain to existing ones.
    ImGui_ImplOpenGL3_Init("#version 330");

    // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    // glfwSetScrollCallback(window, scroll_callback);
    // glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    init();

    while (!glfwWindowShouldClose(window)) {
        processInput(window);
        display();
        // glfwSwapBuffers(window);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        imgui_init();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwPollEvents();
        glfwSwapBuffers(window);
    }
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);

    glfwTerminate();

    return 0;
}

// glfw: whenever the window size changed (by OS or user resize) this callback function
// executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
    SCR_WIDTH = width;
    SCR_HEIGHT = height;
}

void imgui_init() {
    ImGui::NewFrame();
    ImGui::ShowDemoWindow(); // Show demo window! :)
    ImGui::ShowDebugLogWindow();
    ImGui::ShowStackToolWindow();
    float currentTime = glfwGetTime();
    deltaTime = currentTime - lastFrame;
    float cameraSpeed = 2.5f * deltaTime; // 当时间差大（电脑性能差）就位移的更多
    lastFrame = currentTime;
    if (ImGui::Begin("Debug", NULL)) {
        ImGui::SetWindowPos(ImVec2(0, 0), ImGuiCond_Once);
        ImGui::Text("front: | %.3f, %.3f, %.3f |",
                    camera->Front.x,
                    camera->Front.y,
                    camera->Front.z);
        ImGui::Text("Up: | %.3f, %.3f, %.3f |", camera->Up.x, camera->Up.y, camera->Up.z);
        ImGui::Text("position: | %.3f, %.3f, %.3f | (camera pos)",
                    camera->Position.x,
                    camera->Position.y,
                    camera->Position.z);
        auto center = camera->Position + camera->Front;
        ImGui::Text("Position + Front: | %.3f, %.3f, %.3f | (center)",
                    center.x,
                    center.y,
                    center.z);
        ImGui::Text("right: | %.3f, %.3f, %.3f |",
                    camera->Right.x,
                    camera->Right.y,
                    camera->Right.z);
        ImGui::Text("pitch: | %.3f |", camera->Pitch);
        ImGui::Text("yaw: | %.3f |", camera->Yaw);
        ImGui::PushButtonRepeat(true);
        if (ImGui::ArrowButton("#a", ImGuiDir_Left)) {
            camera->ProcessKeyboard(LEFT, deltaTime);
        }
        ImGui::SameLine();
        if (ImGui::ArrowButton("#w", ImGuiDir_Up)) {
            camera->ProcessKeyboard(FORWARD, deltaTime);
        }
        ImGui::SameLine();
        if (ImGui::ArrowButton("#s", ImGuiDir_Down)) {
            camera->ProcessKeyboard(BACKWARD, deltaTime);
        }
        ImGui::SameLine();
        if (ImGui::ArrowButton("#d", ImGuiDir_Right)) {
            camera->ProcessKeyboard(RIGHT, deltaTime);
        }
        ImGui::PopButtonRepeat();

        ImGui::PushButtonRepeat(true);
        if (ImGui::ArrowButton("##a", ImGuiDir_Left)) {
            camera->ProcessMouseMovement(-13, 0);
        }
        ImGui::SameLine();
        if (ImGui::ArrowButton("##w", ImGuiDir_Up)) {
            camera->ProcessMouseMovement(-0, 13);
        }
        ImGui::SameLine();
        if (ImGui::ArrowButton("##s", ImGuiDir_Down)) {
            camera->ProcessMouseMovement(-0, -13);
        }
        ImGui::SameLine();
        if (ImGui::ArrowButton("##d", ImGuiDir_Right)) {
            camera->ProcessMouseMovement(13, 0);
        }

        ImGui::PopButtonRepeat();
        if (ImGui::Button("reset")) {
            delete camera;
            camera = new Camera({0, 0, 3});
        }
        ImGui::End();
    }
}
