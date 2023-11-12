#ifndef INCLUDE_SHADER
#define INCLUDE_SHADER
#include <toy/type.h>
class Shader{
public:
    u32 ID;
    Shader(const char* vertexPath, const char* fragmentPath);
    void use();
    void setBool(const String& name, bool value) const;
    void setInt(const String& name, int value) const;
    void setFloat(const String& name, float value) const;
};

#endif /* INCLUDE_SHADER */
