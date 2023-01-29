#version 450 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texture_coordinate;
layout (location = 2) in vec3 normal;

// attribute vec4 a_position;
uniform mat4 world_matrix;
uniform mat4 view_matrix;
uniform mat4 projective_matrix;
uniform mat4 light_matrix;
// uniform mat4 vp_matrix;

out struct data_to_pass
{
    vec2 v_texcoord;
    vec3 normal;
    vec4 positionWorld;
    vec4 positionCam;
    vec4 posProjected;
    vec4 lightSystemCoordinateFragPos;
} to_fs;

void main()
{
    vec4 worldPos = world_matrix * vec4(position, 1.0);
    vec4 posCam   = view_matrix * worldPos;
    gl_Position = projective_matrix * posCam;

    to_fs.lightSystemCoordinateFragPos = projective_matrix * light_matrix * worldPos;

    to_fs.v_texcoord = texture_coordinate;
    to_fs.normal     = mat3(world_matrix) * normal;
    to_fs.positionWorld   = worldPos;
    to_fs.positionCam     = posCam;
    to_fs.posProjected    = gl_Position;
    // color = vec4( 0, 0, 1.0, 1.0);//vec4(clamp(gl_Position[0],0.1,1.0),clamp(gl_Position[1],0.1,1.0),clamp(gl_Position[2],0.1,1.0),1);
}
