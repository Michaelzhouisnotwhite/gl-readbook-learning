#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

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
#include "path_macro.h"

const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aTexCoord;

out vec3 ourColor;
out vec2 TexCoord; // 把纹理坐标输出到片段着色器

void main()
{
    gl_Position = vec4(aPos, 1.0);
    ourColor = aColor;
    TexCoord = aTexCoord;
}
)";

const char* fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;

in vec3 ourColor;
in vec2 TexCoord;

/*
激活纹理函数: glActiveTexture(GL_TEXTURE0);
因为系统会自动激活第一个纹理，所以当只有一个纹理时，不需要调用这个函数，系统总共有16个纹理单元
*/
uniform sampler2D ourTexture; // sampler采样器，内部数据类型

uniform sampler2D ourTexture2; // 第二个纹理

void main()
{   
    // FragColor = texture(ourTexture, TexCoord);
    // FragColor = texture(ourTexture, TexCoord) * vec4(ourColor, 1.0); // 纹理颜色与顶点颜色混合
    FragColor = mix(texture(ourTexture, TexCoord), texture(ourTexture2, TexCoord), 0.2) * vec4(ourColor, 1.0); // 8:2的方式混合两个纹理
}
)";

u32 vertexShader;
u32 fragmentShader;
u32 shaderProgram;

ShaderBase* shader_program_;
u32 texture1;
u32 texture2;
u32 VBOs[1];
u32 VAOs[1];
u32 EBOs[1];  // element buffer
void init() {
    shader_program_ = new ShaderCompiler(vertexShaderSource, fragmentShaderSource);

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

    // 为当前绑定的纹理对象设置环绕、过滤方式
    // set the texture wrapping parameters
    // s、t（如果是使用3D纹理那么还有一个r）它们和x、y、z是等价的）
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // 第二个参数是多级渐远纹理级别，0为基本级别
    // 第三个参数是存储的格式
    // 第六个没用，总是0
    // 第七第八个参数定义了源图的格式和数据类型。我们使用RGB值加载这个图像，并把它们储存为char(byte)数组，我们将会传入对应值。
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
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

     // note that the awesomeface.png has transparency and thus an alpha channel, so make sure to tell OpenGL the data type is of GL_RGBA
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);

    glGenVertexArrays(1, VAOs);
    glGenBuffers(1, VBOs);
    glGenBuffers(1, EBOs);

    shader_program_->use();
    shader_program_->setInt("ourTexture", 0);  // 告诉着色器那个纹理是第0号纹理

    shader_program_->setInt("ourTexture2", 1);
}
void display() {
    // clang-format off
    float vertices[] = {
        //     ---- 位置 ----              ---- 颜色 ----                 - 纹理坐标 -
        0.5f,  -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.f, 0.f,  // 右下
        0.5f,  0.5f,  0.0f, 1.0f, 0.0f, 0.0f, 1.f, 1.f,  // 右上
        -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,  // 左下
        -0.5f, 0.5f,  0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.f   // 左上
    };
    // 上面三角形顶点的顺序和下面的indices绘图顺序有关系
    // clang-format off
    unsigned int indices[] = {
        //      //       //
        1, 0, 3,  // first triangle
        0, 2, 3  // second triangle
    };
    // clang-format on
    glBindVertexArray(VAOs[0]);
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOs[0]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), NULL);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1,
                          3,
                          GL_FLOAT,
                          GL_FALSE,
                          8 * sizeof(float),
                          (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2,
                          2,
                          GL_FLOAT,
                          GL_FALSE,
                          8 * sizeof(float),
                          (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glClearColor(0.2, 0.3, 0.3, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    glActiveTexture(GL_TEXTURE0);  // 先激活纹理单元，把纹理绑定到片段着色器采样器0上
    // 它会自动把纹理赋值给片段着色器的采样器
    glBindTexture(GL_TEXTURE_2D, texture1);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture2);

    shader_program_->use();
    glBindVertexArray(VAOs[0]);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
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