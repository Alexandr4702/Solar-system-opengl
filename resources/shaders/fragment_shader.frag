#version 450 core

struct texture_Pr {
    vec3 ambient;
    vec3 diffuse;
   vec3 specular;
   float shininess;
};

in struct data_to_pass
{
    vec2 v_texcoord;
    vec3 normal;
    vec3 position;
} to_fs;

uniform sampler2D textures;
out vec4 fragColor;

vec4 calcLight(vec3 Normal, vec3 lightPos, vec3 FragPos, vec3 lightColor, vec3 viewPos, texture_Pr material)
{
    // ambient
    vec3 ambient = lightColor * material.ambient;

    // diffuse
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = lightColor * (diff * material.diffuse);

    // specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = lightColor * (spec * material.specular);

    vec3 result = ambient + diffuse + specular;
    vec4 FragColor = vec4(result, 1.0);
    return FragColor;
}
//! [0]
void main()
{
    vec3 LightPosition = {0, 0, 0};
    vec3 lightColor = {1, 1, 1};
    vec3 CameraPos = {0, 0, 0};
    vec4 temp = texture(textures, to_fs.v_texcoord);
    texture_Pr texture_properties;
    texture_properties.ambient = temp.xyz * 0.1;
    texture_properties.diffuse = temp.xyz * 0.1;
    texture_properties.specular = temp.xyz * 0.4;
    texture_properties.shininess = 64;

    fragColor = calcLight(to_fs.normal, LightPosition, to_fs.position, lightColor, CameraPos, texture_properties);

}
//! [0]

