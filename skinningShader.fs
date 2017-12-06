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
              
out vec4 FragColor;
                                                                
void main()
{                                    
   VSOutput In;
    In.TexCoord = TexCoord0;
    In.Normal   = normalize(Normal0);
    In.WorldPos = WorldPos0;
                                                                                                                        
                                                                                            
    FragColor = vec4(.3,.3,.9,.4) + texture(gColorMap, In.TexCoord.xy)*.001;     
}
