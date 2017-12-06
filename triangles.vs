#version 330

layout (location = 0) in vec3 pos;

uniform mat4 M;
uniform mat4 PV;

void main()
{
	vec4 pos4 = vec4(pos, 1);
	pos4.y += .5;	// center origin on triangles center
	gl_Position = PV*M*pos4;
}