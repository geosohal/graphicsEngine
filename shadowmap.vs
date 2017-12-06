#version 330
// simple vertex shader for shadow map's. in the first pass we use WVP matrix
// on this shader using the shadow casting light, on the second pass we use WVP
// of the camera

layout (location = 0) in vec3 Position;
layout (location = 1) in vec2 TexCoord;
layout (location = 2) in vec3 Normal;

uniform mat4 gWVP;

out vec2 TexCoordOut;

void main()
{
    gl_Position = gWVP * vec4(Position, 1.0);
    TexCoordOut = TexCoord;
}