#include <GL3/gl3.h>
#include <GL3/gl3w.h>

#include "LoadShaders.h"
#include "vgl.h"
#include <GLFW/glfw3.h>
#include <array>
#include <cmath>
#include <iostream>
#include <stdint.h>
#include <toy/type.h>
#include "Shader.hpp"

const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;   // 位置变量的属性位置值为 0 
layout (location = 1) in vec3 aColor; // 颜色变量的属性位置值为 1
uniform float xOffset; // 水平偏移
out vec3 ourColor; // 向片段着色器输出一个颜色 可以是任意一个变量都可以输送
void main()
{
    gl_Position = vec4(aPos.x + xOffset, -aPos.y, aPos.z,  1.0);
    ourColor = aColor; // 将ourColor设置为我们从顶点数据那里得到的输入颜色
}
)";

const char* fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;
in vec3 ourColor; // 从顶点着色器得到一个变量
uniform vec4 outColor;
void main() {
    FragColor = vec4(ourColor, 1.0);
}
)";

u32 vertexShader;
u32 fragmentShader;
u32 shaderProgram;

ShaderBase* shader_program_;

u32 VBOs[2];
u32 VAOs[2];
void init() {
    shader_program_ = new ShaderCompiler(vertexShaderSource, fragmentShaderSource);

    glGenVertexArrays(1, VAOs);
    glGenBuffers(1, VBOs);
}
void display() {
    float vertices[] = {
        // 位置              // 颜色
        0.5f,
        -0.5f,
        0.0f,
        1.0f,
        0.0f,
        0.0f,  // 右下
        -0.5f,
        -0.5f,
        0.0f,
        0.0f,
        1.0f,
        0.0f,  // 左下
        0.0f,
        0.5f,
        0.0f,
        0.0f,
        0.0f,
        1.0f  // 顶部
    };

    glBindVertexArray(VAOs[0]);
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_COPY);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1,
                          3,
                          GL_FLOAT,
                          GL_FALSE,
                          6 * sizeof(float),
                          (void*)(0 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    shader_program_->setFloat("xOffset", 0.0);
    shader_program_->use();

    glBindVertexArray(VAOs[0]);
    glDrawArrays(GL_TRIANGLES, 0, 3);  // 第三个参数是要绘制多少个顶点
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

    GLFWwindow* window = glfwCreateWindow(800, 600, "Triangles", NULL, NULL);

    glfwMakeContextCurrent(window);
    gl3wInit();

    init();

    while (!glfwWindowShouldClose(window)) {
        display();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);

    glfwTerminate();

    return 0;
}