#version 330                 
                                                       
const float MIN_DEPTH = .3f;
const float MAX_DEPTH = 30.5f;    
const float e = 2.71828182846f;
const float c = 30.f;
		                                                                        
in vec2 TexCoordOut;                                                                
uniform sampler2D gShadowMap;                                                       
                                                                                    
layout (location = 0) out float shadowmap2Out;                                                               
                                                                                    
void main()                                                                         
{                                                                                   
    float Depth = texture(gShadowMap, TexCoordOut).x;                               
    Depth = (Depth-MIN_DEPTH) / (MAX_DEPTH - MIN_DEPTH);    
	Depth = pow(e, c*Depth );                                     
    shadowmap2Out = Depth;                                                        
}
