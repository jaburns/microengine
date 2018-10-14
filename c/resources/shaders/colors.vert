#version 410

in vec3 position;
in vec3 color;

out vec3 v_color;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    v_color = color;
    gl_Position = projection * view * model * vec4(position, 1.0f);
}