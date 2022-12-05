#version 450 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texture_coordinate;
layout (location = 2) in vec3 normal;

// attribute vec4 a_position;
uniform mat4 m_matrix;
uniform mat4 vp_matrix;

out struct data_to_pass
{
    vec2 v_texcoord;
    vec3 normal;
    vec3 position;
} to_fs;

void main()
{
    vec4 worldPos = m_matrix * vec4(position, 1.0);

    gl_Position = vp_matrix * worldPos;

    to_fs.v_texcoord = texture_coordinate;
    to_fs.normal     = mat3(transpose(inverse(m_matrix))) * normal;
    to_fs.position   = worldPos.xyz;
    // color = vec4( 0, 0, 1.0, 1.0);//vec4(clamp(gl_Position[0],0.1,1.0),clamp(gl_Position[1],0.1,1.0),clamp(gl_Position[2],0.1,1.0),1);
}
