#version 330                                                                        

layout (location = 0) in vec3 Position; 
layout (location = 1) in vec2 TexCoord;    
layout (location = 2) in vec3 Normal; 

out vec2 TexCoord0;  
out vec3 Normal0;   

uniform mat4 gWorld;

void main()
{          
    gl_Position = vec4(Position, 1.0);
	TexCoord0      = TexCoord;   
	Normal0 = Normal;
}
