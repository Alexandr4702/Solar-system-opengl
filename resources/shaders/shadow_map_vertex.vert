#version 450 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texture_coordinate;
layout (location = 2) in vec3 normal;

uniform mat4 gWVP;

out vec2 TexCoordOut;

void main()
{
    gl_Position = gWVP * vec4(position, 1.0);
    TexCoordOut = texture_coordinate;
}