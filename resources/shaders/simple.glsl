v2f vec3 v_normal;
v2f vec2 v_tex_coords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform sampler2D tex;

#ifdef VERTEX

    in vec3 position;
    in vec3 normal;
    in vec2 uv;

    void main() 
    {
        v_normal = inverse(mat3(model)) * normal;
        v_tex_coords = uv;
        gl_Position = projection * view * model * vec4(position, 1.0f);
    }

#endif
#ifdef FRAGMENT

    const vec3 light_x = vec3(-1.0, 0.4, 0.9);

    out vec4 color;

    void main() 
    {
        float brightness = dot(normalize(v_normal), normalize(light_x));
        color = (0.75 + 0.25 * brightness) * texture(tex, v_tex_coords);
    }

#endif
