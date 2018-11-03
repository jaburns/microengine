v2f vec3 v_color;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

#ifdef VERTEX

    layout(location = 0) in vec3 position;
    layout(location = 1) in vec3 normal;

    void main()
    {
        v_color = normal*0.5 + 0.5;
        gl_Position = projection * view * model * vec4(position, 1.0f);
    }

#endif
#ifdef FRAGMENT

    out vec4 f_color;

    void main()
    {
        f_color = vec4(v_color, 1);
    }

#endif
