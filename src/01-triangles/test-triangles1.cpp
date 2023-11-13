#include <iostream>
#include "vgl.h"
#include "LoadShaders.h"
const char* vertexShaderSource =
    "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos, 1.0);\n"
    "}\0";
const char* fragmentShaderSource =
    "#version 330 core\n"
    "out vec4 TriColor;\n"
    "void main()\n"
    "{\n"
    "   TriColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
    "}\n\0";

uint32_t vao, vbo;
unsigned int shaderProgram;
void init() {
    uint32_t vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertex_shader);
    // check for shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertex_shader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    // fragment shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    // check for shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    // link shaders
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertex_shader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    // check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    glDeleteShader(vertex_shader);
    glDeleteShader(fragmentShader);

    glUseProgram(shaderProgram);

    float vertices[] = {
        // first triangle
        -0.9f,
        -0.5f,
        0.0f,  // left
        -0.0f,
        -0.5f,
        0.0f,  // right
        -0.45f,
        0.5f,
        0.0f,  // top
               // second triangle
        0.0f,
        -0.5f,
        0.0f,  // left
        0.9f,
        -0.5f,
        0.0f,  // right
        0.45f,
        0.5f,
        0.0f  // top
    };

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    // glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBufferStorage(GL_ARRAY_BUFFER, sizeof(vertices), vertices, 0);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
    glEnableVertexAttribArray(0);

    // 防止后面操作的时候不小心操作到了vio 和vbo，让他unbound一下
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}
void display() {
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glBindVertexArray(vao);

    glDrawArrays(GL_TRIANGLES, 0, 6);
}

int main(int argc, char** argv) {
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
}

