#version 330


in vec2 TexCoord0;
in vec3 Normal0;                                                                   
in vec3 WorldPos0;      
                                                           
struct VSOutput
{
    vec2 TexCoord;
    vec3 Normal;                                                                   
    vec3 WorldPos;                                                                 
};
uniform sampler2D gColorMap; 
              
//out vec4 FragColor;

// deferred rendering outputs
layout (location = 0) out vec3 NormalOut;   
layout (location = 1) out vec3 TexCoordOut; 
layout (location = 2) out vec3 DiffuseOut;   
layout (location = 3) out vec3 WorldPosOut; 
                                                                
void main()
{                                    
    vec4 FragColor = vec4(.1,.1,.6,.4) + texture(gColorMap, TexCoord0.xy)*.1;     
	                                                                                                            
    NormalOut = normalize(Normal0);
	DiffuseOut = FragColor.xyz;
	WorldPosOut = WorldPos0;
	TexCoordOut = vec3(TexCoord0,0);//not used                                                                            
}
