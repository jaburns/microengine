uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 line_color;

#ifdef VERTEX

    layout(location = 0) in vec3 position;

    void main()
    {
        gl_Position = projection * view * model * vec4(position, 1.0f);
    }  

#endif
#ifdef FRAGMENT

    out vec4 f_color;

    void main()
    {    
        f_color = vec4(line_color, 1);
    }

#endif
