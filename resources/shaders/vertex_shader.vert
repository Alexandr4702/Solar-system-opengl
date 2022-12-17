#version 450 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texture_coordinate;
layout (location = 2) in vec3 normal;

// attribute vec4 a_position;
uniform mat4 m_matrix;
uniform mat4 v_matrix;
uniform mat4 p_matrix;
// uniform mat4 vp_matrix;

out struct data_to_pass
{
    vec2 v_texcoord;
    vec3 normal;
    vec4 positionWorld;
    vec4 positionCam;
    vec4 posProjected;
} to_fs;

void main()
{
    vec4 worldPos = m_matrix * vec4(position, 1.0);
    vec4 posCam   = v_matrix * worldPos;
    gl_Position = p_matrix * posCam;

    to_fs.v_texcoord = texture_coordinate;
    to_fs.normal     = mat3(m_matrix) * normal;
    to_fs.positionWorld   = worldPos;
    to_fs.positionCam     = posCam;
    to_fs.posProjected    = gl_Position;
    // color = vec4( 0, 0, 1.0, 1.0);//vec4(clamp(gl_Position[0],0.1,1.0),clamp(gl_Position[1],0.1,1.0),clamp(gl_Position[2],0.1,1.0),1);
}
