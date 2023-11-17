
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
#include <implot.h>

int SCR_WIDTH = 800;
int SCR_HEIGHT = 600;
void framebuffer_size_callback(GLFWwindow* window, int width, int height);

void imgui_init();

void init() {
    glEnable(GL_DEPTH_TEST);
}

void display() {
    glClearColor(0.2, 0.3, 0.3, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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
    ImPlot::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls

    io.FontGlobalScale = 2;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window,
                                 true);  // Second param install_callback=true will install
                                         // GLFW callbacks and chain to existing ones.
    ImGui_ImplOpenGL3_Init("#version 330");

    // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    // glfwSetScrollCallback(window, scroll_callback);
    // glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    init();

    while (!glfwWindowShouldClose(window)) {
        // processInput(window);
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
    ImPlot::DestroyContext();
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
    ImGui::ShowDemoWindow();  // Show demo window! :)
    ImGui::ShowDebugLogWindow();
    ImGui::ShowStackToolWindow();

    ImPlot::ShowDemoWindow();
    if (ImGui::Begin("Debug", NULL)) {
        ImGui::End();
    }
}