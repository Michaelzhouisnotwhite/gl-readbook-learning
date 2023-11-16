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
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aTexCoord;

out vec3 ourColor;
out vec2 TexCoord; // 把纹理坐标输出到片段着色器

uniform mat4 transform; // 变换矩阵

void main()
{
    gl_Position = transform * vec4(aPos, 1.0);
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
    FragColor = mix(texture(ourTexture, TexCoord), texture(ourTexture2, TexCoord), mixValue) * vec4(ourColor, 1.0); // 8:2的方式混合两个纹理
}
)";

std::function<void(GLFWwindow*)> processInput = [](GLFWwindow*) {};

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
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
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
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // note that the awesomeface.png has transparency and thus an alpha channel, so make sure
    // to tell OpenGL the data type is of GL_RGBA
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);

    glGenVertexArrays(1, VAOs);
    glGenBuffers(1, VBOs);
    glGenBuffers(1, EBOs);
    // clang-format off
    float vertices[] = {
        //     ---- 位置 ----              ---- 颜色 ----                 - 纹理坐标 -
        0.5f,  -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 2.f, 0.f,  // 右下
        0.5f,  0.5f,  0.0f, 1.0f, 0.0f, 0.0f, 2.f, 2.f,  // 右上
        -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.f, 0.f,  // 左下
        -0.5f, 0.5f,  0.0f, 1.0f, 1.0f, 0.0f, 0.f, 2.f   // 左上
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
    shader_program_->use();
    shader_program_->setInt("ourTexture", 0);  // 告诉着色器那个纹理是第0号纹理

    shader_program_->setInt("ourTexture2", 1);
}

float mixValue = 0.5;
void display() {
    glClearColor(0.2, 0.3, 0.3, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    glActiveTexture(GL_TEXTURE0);  // 先激活纹理单元，把纹理绑定到片段着色器采样器0上
    // 它会自动把纹理赋值给片段着色器的采样器
    glBindTexture(GL_TEXTURE_2D, texture1);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture2);

    shader_program_->use();
    glm::mat4 trans(1.0);

    // 先缩放 再旋转，后位移 （防止旋转和位移的量受到缩放的影响）
    // trans = glm::rotate(trans, glm::radians(90.0f), glm::vec3(0.0, 0.0, 1.0)); //
    // 把z轴旋转90度
    trans = glm::scale(trans, glm::vec3(1.5, 1.5, 1.5));  // x,y,z放大1.5倍
    trans = glm::translate(trans, glm::vec3(0.5, -0.5, 0.0));
    trans = glm::rotate(trans, (float)glfwGetTime(), glm::vec3(0.0, 0.0, 1.0));
    u32 transformLoc = glGetUniformLocation(shader_program_->ID, "transform");

    // 第二个参数告诉OpenGL我们将要发送多少个矩阵
    // 第三个参数询问我们是否希望对我们的矩阵进行转置(Transpose)
    // PS: OpenGL开发者通常使用一种内部矩阵布局，叫做列主序(Column-major
    // Ordering)布局。GLM的默认布局就是列主序，所以并不需要转置矩阵
    glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(trans));

    processInput = [](GLFWwindow* window) {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
            mixValue += 0.005f;  // change this value accordingly (might be too slow or too
                                 // fast based on system hardware)
            if (mixValue >= 1.0f)
                mixValue = 1.0f;
        }
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
            mixValue -= 0.005f;  // change this value accordingly (might be too slow or too
                                 // fast based on system hardware)
            if (mixValue <= 0.0f)
                mixValue = 0.0f;
        }
    };

    shader_program_->setFloat("mixValue", mixValue);
    glBindVertexArray(VAOs[0]);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    trans = glm::mat4(1.0);
    trans = glm::translate(trans, glm::vec3(-0.5, 0.5, 0.0));
    trans = glm::rotate(trans, (float)glfwGetTime(), glm::vec3(0.0, 0.0, 1.0));
    shader_program_->setMatrix4f("transform", trans);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    trans = glm::mat4(1.0);
    trans = glm::scale(trans, glm::abs(glm::sin(glm::vec3(glfwGetTime()))));
    shader_program_->setMatrix4f("transform", trans);
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
        processInput(window);
        display();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);

    glfwTerminate();

    return 0;
}