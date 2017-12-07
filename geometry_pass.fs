#version 330
               
const float MIN_DEPTH = .3f;
const float MAX_DEPTH = 9.5f;    

in vec2 TexCoord0;                                                                  
in vec3 Normal0;                                                                    
in vec3 WorldPos0;   
                                                               
layout (location = 0) out vec3 NormalOut;   
layout (location = 1) out vec3 TexCoordOut; 
layout (location = 2) out vec3 DiffuseOut;   
layout (location = 3) out vec3 WorldPosOut;   

uniform sampler2D depthMap;   
  
  
   
										
uniform sampler2D gColorMap;                
											
void main()									
{											
	WorldPosOut     = WorldPos0; //100.0;					
	DiffuseOut      = texture(gColorMap, TexCoord0).xyz;
	NormalOut       = normalize(Normal0);		
	
    float Depth = texture(depthMap, TexCoord0).x;                               
    Depth = clamp( (Depth-MIN_DEPTH) / (MAX_DEPTH - MIN_DEPTH), 0,1)*5.f;          
	
		TexCoordOut     = vec3(TexCoord0, Depth);				
}
