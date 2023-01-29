#version 450 core

layout (location = 0) in vec3 position;
// layout (location = 1) in vec2 texture_coordinate;
// layout (location = 2) in vec3 normal;

uniform mat4 world_matrix;
uniform mat4 projective_matrix;
uniform mat4 light_matrix;

void main()
{
    gl_Position = projective_matrix * light_matrix * world_matrix * vec4(position, 1.0);

    // gl_Position = vec4(0.5, 0.5, 0.0, 1);
    // gl_Position[2] = -0.8;
    // gl_Position[3] = 1;
    // TexCoordOut = texture_coordinate;
}