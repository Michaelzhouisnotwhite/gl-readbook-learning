#ifndef INCLUDE_SHADER
#define INCLUDE_SHADER
#include <toy/type.h>
#include <GL3/gl3.h>
#include <GL3/gl3w.h>
#include <string>
#include <sstream>
#include <iostream>
class ShaderBase {
public:
    u32 ID;
    void setBool(const String& name, bool value) const {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
    }
    void setInt(const String& name, int value) const {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
    }
    void setFloat(const String& name, float value) const {
        glUniform1f(glGetUniformLocation(ID, name.c_str()), (int)value);
    }
    void use() {
        glUseProgram(ID);
    }
};

class Shader : public ShaderBase {
public:
    Shader(const char* vertexPath, const char* fragmentPath);
};

class ShaderCompiler : public ShaderBase {
public:
    ShaderCompiler(const char* vertexSource, const char* fragmentSource) {
        auto vertexShader = glCreateShader(GL_VERTEX_SHADER);
        auto fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

        glShaderSource(vertexShader, 1, &vertexSource, NULL);
        glCompileShader(vertexShader);

        int success;
        char infoLog[512];
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
            std::stringstream str;
            str << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog;
            throw std::runtime_error(str.str());
        }
        glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
        glCompileShader(fragmentShader);
        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);

        if (!success) {
            glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
            std::stringstream str;
            str << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog;
            throw std::runtime_error(str.str());
        }

        auto program = glCreateProgram();
        glAttachShader(program, vertexShader);
        glAttachShader(program, fragmentShader);
        glLinkProgram(program);
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(program, 512, NULL, infoLog);
            std::stringstream str;
            str << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
            throw std::runtime_error(str.str());
        }
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        ID = program;
    }
};
#endif /* INCLUDE_SHADER */
