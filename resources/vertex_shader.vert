#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in float in_color;
uniform mat4 mvp_matrix;

varying vec4 color;
void main()
{
    gl_Position = mvp_matrix * vec4(position, 1.0);
    color = vec4(in_color / 255, in_color/ 255, in_color / 255, 1.0);//vec4(clamp(gl_Position[0],0.1,1.0),clamp(gl_Position[1],0.1,1.0),clamp(gl_Position[2],0.1,1.0),1);
}
