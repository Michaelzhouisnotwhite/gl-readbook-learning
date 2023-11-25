#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out vec3 ourColor;
out vec3 Normal;
out vec3 FragPos;
uniform mat4 model; 
uniform mat4 view; 
uniform mat4 projection; 


void main()
{

    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal; // 所有的计算都在片段着色器中，所以需要把法向量传入着色器中
    gl_Position = projection * view * model * vec4(FragPos, 1.0);
}