#version 330                                                                        
                                                                                    
layout (location = 0) in vec3 Position;                                             
layout (location = 1) in vec2 TexCoord;                                             
layout (location = 2) in vec3 Normal;                                               

uniform mat4 gWVP;	// todog actually gVP
uniform mat4 gWorld;
             
out vec3 WorldPos0;                              
out vec2 TexCoord0;                                                                 
out vec3 Normal0;                                                                   
                                                              


void main()
{       
    gl_Position    =  gWVP * gWorld * vec4(Position, 1.0); //todo, check if we can put depth in here.
    TexCoord0      = TexCoord;                  
    Normal0        = (gWorld * vec4(Normal, 0.0)).xyz;   
    WorldPos0      = (gWorld * vec4(Position, 1.0)).xyz;
}