#version 150

in vec3 position;
in vec3 normal;

out vec3 v_normal;
out vec2 v_tex_coords;

uniform mat4 perspective;
uniform mat4 matrix;

void main() {
    v_normal = transpose(inverse(mat3(matrix))) * normal;
    v_tex_coords = vec2(position.x * 0.01, position.y * 0.01);
    gl_Position = perspective * matrix * vec4(position, 1.0);
}
