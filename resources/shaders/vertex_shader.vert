#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texture_coordinate;
layout (location = 2) in vec3 normal;

// attribute vec4 a_position;

uniform mat4 mvp_matrix;

varying vec2 v_texcoord;

void main()
{
    gl_Position = mvp_matrix * vec4(position, 1.0);
    // gl_Position = mvp_matrix * a_position;
    v_texcoord = texture_coordinate;
    // color = vec4( 0, 0, 1.0, 1.0);//vec4(clamp(gl_Position[0],0.1,1.0),clamp(gl_Position[1],0.1,1.0),clamp(gl_Position[2],0.1,1.0),1);
}
