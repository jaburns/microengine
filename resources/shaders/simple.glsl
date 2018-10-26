#version 150

V2F vec3 v_normal;
V2F vec2 v_tex_coords;

uniform mat4 perspective;
uniform mat4 matrix;
uniform sampler2D tex;

#ifdef VERTEX

    in vec3 position;
    in vec3 normal;

    void main() {
        v_normal = transpose(inverse(mat3(matrix))) * normal;
        v_tex_coords = vec2(position.x * 0.01, position.y * 0.01);
        gl_Position = perspective * matrix * vec4(position, 1.0);
    }

#endif
#ifdef FRAGMENT

    const vec3 light_x = vec3(-1.0, 0.4, 0.9);

    out vec4 color;

    void main() {
        float brightness = dot(normalize(v_normal), normalize(light_x));
        color = (0.5 + 0.5 * brightness) * texture(tex, v_tex_coords);
    }

#endif
