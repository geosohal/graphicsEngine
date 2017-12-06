#version 330

layout(location = 0) in vec3 vertex;

uniform mat4 PV;

void main()
{    
    gl_Position = PV*vec4(vertex,1.0);
}


