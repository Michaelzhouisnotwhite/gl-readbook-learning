#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos; // 片段位置，光在cube身上的位置
uniform sampler2D ourTexture; 
uniform vec3 objColor = vec3(1.0f, 0.5f, 0.31f); 
uniform vec3 lightColor = vec3(1.0f, 1.0f, 1.0f);
uniform vec3 lightPos; // 光源位置
uniform vec3 viewPos; // 反光
void main()
{   // 环绕光
    float ambientStrength = 0.2;
    vec3 ambient = ambientStrength * lightColor; // 环境光照

    // diffuse扩散
    vec3 norm = normalize(Normal); // 光照只关心方向，需要对所有的向量进行标准化
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // specular 镜面反射
    float specularStrength =0.5;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm); // 获取反射向量
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32); // 32是反光度，反光度越高，散射越少，高光点越小
    vec3 specular = specularStrength * spec * lightColor;

    // vec3 result = (ambient + diffuse) * objColor; // 慢反射
    vec3 result = (ambient + diffuse + specular) * objColor; // 镜面反射加入到环境光分量和慢反射分量
    FragColor = vec4(result, 1.0);
    // FragColor =  vec4(result, 1.0);


}