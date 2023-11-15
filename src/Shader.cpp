#include "Shader.hpp"
#include <GL3/gl3.h>
#include <GL3/gl3w.h>
#include <fstream>
#include <iostream>
#include <sstream>

Shader::Shader(const char* vertexPath, const char* fragmentPath)
{
    String vertexCode;
    String fragmentCode;
    std::ifstream vShaderFile;
    std::ifstream fShaderFile;
    vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try {
        // 打开文件
        vShaderFile.open(vertexPath);
        fShaderFile.open(fragmentPath);
        std::stringstream vShaderStream, fShaderStream;
        // 读取文件的缓冲内容到数据流中
        vShaderStream << vShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();
        // 关闭文件处理器
        vShaderFile.close();
        fShaderFile.close();
        // 转换数据流到string
        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();
    } catch (std::ifstream::failure e) {
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ" << std::endl;
    }

    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();

    // 2. 编译着色器
    unsigned int vertex, fragment;
    int success;
    char infoLog[512];

    // 顶点着色器
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompileShader(vertex);
    // 打印编译错误（如果有的话）
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertex, 512, NULL, infoLog);
        std::stringstream output("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n");
        output << infoLog;
        throw std::runtime_error(output.str());
    };

    // 片段着色器
    fragment = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    glCompileShader(fragment);
    // 打印编译错误（如果有的话）
    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragment, 512, NULL, infoLog);
        std::stringstream output("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n");
        output << infoLog;
        throw std::runtime_error(output.str());
    };

    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);

    // 打印连接错误（如果有的话）
    glGetProgramiv(ID, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(ID, 512, NULL, infoLog);
        std::stringstream output("ERROR::SHADER::PROGRAM::LINKING_FAILED\n");
        output << infoLog;
        throw std::runtime_error(output.str());
    }

    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

