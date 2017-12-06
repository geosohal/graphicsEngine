#version 430

layout (location = 0) in vec3 Position;
layout (location = 2) in vec3 Normal;

uniform mat4 gVP;
uniform mat4 gWorld;

out vec3 Normal0;
out vec3 WorldPos0;

void main()
{
    gl_Position = vec4(Position, 1.0);
}