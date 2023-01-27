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
    vec4 positionCam;
    vec4 posProjected;
    vec4 lightPos;
} to_fs;

uniform sampler2D textures;
out vec4 fragColor;
uniform vec3 camPosition;
uniform float far_plane;

uniform vec3 ambient_texture;
uniform vec3 diffuse_texture;
uniform vec3 specular_texture;
uniform float shininess;

// uniform sampler2D shadowMap;
uniform samplerCube shadowMap;

float CalcShadowFactor(sampler2D shadowMap, vec4 LightSpacePos)
{
    vec3 ProjCoords = LightSpacePos.xyz / LightSpacePos.w;
    vec2 UVCoords;
    UVCoords.x = 0.5 * ProjCoords.x + 0.5;
    UVCoords.y = 0.5 * ProjCoords.y + 0.5;
    float z = 0.5 * ProjCoords.z + 0.5;
    float Depth = texture(shadowMap, UVCoords).x;
    // return Depth;
    if (Depth < z - 0.00001)
        return 0.2;
    else
        return 1.0;
}

float PointsShadowCalculation(samplerCube shadowMap, vec3 fragPos)
{
    // get vector between fragment position and light position
    vec3 fragToLight = fragPos - to_fs.lightPos.xyz;
    // ise the fragment to light vector to sample from the depth map
    float closestDepth = texture(shadowMap, fragToLight).x;
    // it is currently in linear range between [0,1], let's re-transform it back to original depth value
    closestDepth *= far_plane;
    // now get current linear depth as the length between the fragment and light position
    float currentDepth = length(fragToLight);
    // test for shadows
    float bias = 0.05; // we use a much larger bias since depth is now in [near_plane, far_plane] range
    float shadow = currentDepth -  bias > closestDepth ? 1.0 : 0.0;
    // display closestDepth as debug (to visualize depth cubemap)
    // FragColor = vec4(vec3(closestDepth / far_plane), 1.0);

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
//! [0]
void main()
{
    vec3 LightPosition = {0, 0, 0};
    vec3 lightColor = {1, 1, 1};
    vec4 temp = texture(textures, to_fs.v_texcoord);
    texture_Pr texture_properties;
    texture_properties.ambient = temp.xyz * 0.1;
    texture_properties.diffuse = temp.xyz * 0.45 * PointsShadowCalculation(shadowMap, to_fs.positionWorld.xyz);
    texture_properties.specular = temp.xyz * 0.45;
    texture_properties.shininess = 8;

    fragColor = calcLight(to_fs.normal, LightPosition, to_fs.positionWorld.xyz, lightColor, camPosition, texture_properties);
    // fragColor = vec4(to_fs.posProjected.z, to_fs.posProjected.z, to_fs.posProjected.z, 1);
    // fragColor = texture(shadowMap, to_fs.v_texcoord);
    // fragColor = to_fs.lightPos;
    // fragColor = vec4(vec3(CalcShadowFactor(to_fs.lightPos)), 1);
    fragColor = vec4(vec3(PointsShadowCalculation(shadowMap, to_fs.positionWorld.xyz)), 1);
}
//! [0]

