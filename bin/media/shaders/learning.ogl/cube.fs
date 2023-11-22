#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
uniform sampler2D ourTexture; 
uniform vec3 objColor = vec3(1.0f, 0.5f, 0.31f); 
uniform vec3 lightColor = vec3(1.0f, 1.0f, 1.0f);

void main()
{   
    FragColor = texture(ourTexture, TexCoord);
}