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
    vec4 positionWorld;
    vec4 posProjected;
    vec4 lightSystemCoordinateFragPos;

    vec4 positionCam;
} to_fs;

uniform sampler2D textures;
out vec4 fragColor;
uniform vec3 camPosition;

uniform float near_plane;
uniform float far_plane;

uniform vec3 ambient_texture;
uniform vec3 diffuse_texture;
uniform vec3 specular_texture;
uniform float shininess;
vec3 LightPosition = {0, 0, 0};

uniform sampler2D shadowMap;
uniform samplerCube PointShadowMap;

vec3 gridSamplingDisk[20] = vec3[]
(
   vec3(1, 1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1, 1,  1),
   vec3(1, 1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
   vec3(1, 1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1, 1,  0),
   vec3(1, 0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1, 0, -1),
   vec3(0, 1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0, 1, -1)
);

float CalcShadowFactor(sampler2D shadowMap, vec4 LightSpacePos)
{
    vec3 ProjCoords = LightSpacePos.xyz / LightSpacePos.w;
    vec2 UVCoords;
    UVCoords.x = 0.5 * ProjCoords.x + 0.5;
    UVCoords.y = 0.5 * ProjCoords.y + 0.5;
    float z = 0.5 * ProjCoords.z + 0.5;
    float Depth = texture(shadowMap, UVCoords).x;
    // Depth = texture(shadowMap, vec2(0.5, 0.5)).x;
    // return Depth * 1000 - 999.2;
    return (Depth < z - 0.00001) ? 0: 1;
}

float PointsShadowCalculation(samplerCube shadowMap, vec3 fragPos)
{
    vec3 fragToLight = fragPos - LightPosition.xyz;
    float closestDepth = texture(shadowMap, fragToLight).x;
    closestDepth *= 25;
    float currentDepth = length(fragToLight);

    float bias = max(0.05 * (1.0 - dot(to_fs.normal, fragToLight)), 0.005);
    float shadow = currentDepth -  bias > closestDepth ? 0.0 : 1.0;

    // float shadow = 0.0;
    // float bias = 0.15;
    // int samples = 20;
    // float viewDistance = length(to_fs.positionCam.xyz - fragPos);
    // float diskRadius = (1.0 + (viewDistance / far_plane)) / 25.0;
    // for(int i = 0; i < samples; ++i)
    // {
    //     float closestDepth = texture(shadowMap, fragToLight + gridSamplingDisk[i] * diskRadius).r;
    //     closestDepth *= far_plane;   // undo mapping [0;1]
    //     if(currentDepth - bias > closestDepth)
    //         shadow += 1.0;
    // }
    // shadow /= float(samples);

    return shadow;
}

vec4 PhongLight(vec3 Normal, vec3 lightDir, vec3 FragPos, vec3 lightColor, vec3 viewPos, texture_Pr material)
{
    // ambient
    vec3 ambient = lightColor;

    // diffuse
    vec3 norm = normalize(Normal);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = lightColor * diff;

    // specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = lightColor * spec;

    vec3 result = ambient * material.ambient + diffuse * material.diffuse + specular * material.specular;
    vec4 FragColor = vec4(result, 1.0);
    return FragColor;
}

vec4 BllinPhongLight(vec3 Normal, vec3 lightDir, vec3 FragPos, vec3 lightColor, vec3 viewPos, texture_Pr material)
{
    // ambient
    vec3 ambient = lightColor;

    // diffuse
    vec3 norm = normalize(Normal);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = lightColor * diff;

    // specular
    vec3 viewDir    = normalize(viewPos - FragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, halfwayDir), 0.0), material.shininess);
    vec3 specular = lightColor * spec;

    vec3 result = ambient * material.ambient + diffuse * material.diffuse + specular * material.specular;
    vec4 FragColor = vec4(result, 1.0);
    return FragColor;
}

vec4 calcLight(vec3 Normal, vec3 lightPos, vec3 FragPos, vec3 lightColor, vec3 viewPos, texture_Pr material)
{
    vec3 lightDir = normalize(lightPos - FragPos);
    return PhongLight(Normal, lightDir, FragPos, lightColor, viewPos, material);
}

void main()
{
    vec3 lightColor = {near_plane, 1, 1};
    vec4 temp = texture(textures, to_fs.v_texcoord);
    texture_Pr texture_properties;
    texture_properties.ambient = temp.xyz * ambient_texture;
    texture_properties.diffuse = temp.xyz * diffuse_texture * PointsShadowCalculation(PointShadowMap, to_fs.positionWorld.xyz);
    texture_properties.specular = temp.xyz * specular_texture * PointsShadowCalculation(PointShadowMap, to_fs.positionWorld.xyz);
    texture_properties.shininess = shininess;

    fragColor = calcLight(to_fs.normal, LightPosition, to_fs.positionWorld.xyz, lightColor, camPosition, texture_properties);
    // fragColor = vec4(to_fs.posProjected.z, to_fs.posProjected.z, to_fs.posProjected.z, 1);
    // fragColor = texture(shadowMap, to_fs.v_texcoord);
    // fragColor = to_fs.lightPos;
    // fragColor = vec4(vec3(CalcShadowFactor(shadowMap, to_fs.lightSystemCoordinateFragPos)), 1);
    // fragColor = vec4(vec3(PointsShadowCalculation(PointShadowMap, to_fs.positionWorld.xyz)), 1);
}

