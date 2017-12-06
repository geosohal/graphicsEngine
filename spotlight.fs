#version 330                                                                        
                                                                                    
                                                   
const int MAX_SPOT_LIGHTS = 1;     
const float MIN_DEPTH = .3f;
const float MAX_DEPTH = 30.5f; 
const float M_E = 2.71828182846f;      
const float c = 30.f;                                       
                                  
								                                                    
in vec4 LightSpacePos;                                                              
in vec2 TexCoord0;                                                                  
in vec3 Normal0;                                                                    
in vec3 WorldPos0;                                                                  
                                                                                    
out vec4 FragColor;                                                                 
                                                                                    
struct BaseLight                                                                    
{                                                                                   
    vec3 Color;            //                                                         
    float AmbientIntensity;       //                                                  
    float DiffuseIntensity;      //                                                   
};                                                                                  
                                                                                                                                                                
struct Attenuation                                                                  
{                                                                                   
    float Constant;                                                                 
    float Linear;                                                                   
    float Exp;                                                                      
};                                                                                  
struct PointLight                                                                           
{                                                                                           
    BaseLight Base;                 //                                                 
    vec3 Position;              //                                                            
    Attenuation Atten;            //                                                          
};                                                                    
struct SpotLight                                                                            
{                                                                                           
    PointLight Base;                                                                 
    vec3 Direction;                   //                                                      
    float Cutoff;      //                                                                     
};                                                                                          
                                                                                            
                               
                                       
uniform SpotLight gSpotLight;                                             
uniform sampler2D gSampler;                                                                 
uniform sampler2D gShadowMap;                                                               
uniform vec3 gEyeWorldPos;                                                                  
uniform float gMatSpecularIntensity = 1.0f;
uniform float gSpecularPower =36;

uniform bool isQuad = false; 

                                                  
           // Tone Mapping Settings
uniform float A = 0.15;
uniform float B = 0.50;
uniform float C = 0.10;
uniform float D = 0.20;
uniform float E = 0.02;
uniform float F = 0.30;
uniform float W = 11.2;
uniform float Exposure = 1.0;
uniform float ExposureBias = 2.0;

vec3 _ToneMap(vec3 x)
{
  return ((x*(A*x+C*B)+D*E)/(x*(A*x+B)+D*F))-E/F;
}

vec3 ToneMap(vec3 color)
{
  return _ToneMap(ExposureBias*color) / _ToneMap(vec3(W));
}                                
vec4 l2rgb(vec4 c)
{
  return pow(c, vec4(1.0f / 2.2f));
}	   
										                                                    
float CalcShadowFactor(vec4 LightSpacePos, vec3 Normal, vec3 LightDirection)                                                  
{                                            
	// perspective division to transform vector to NDC space.                            
    vec3 ProjCoords = LightSpacePos.xyz / LightSpacePos.w;         
	
	// prepare 2d coordinate to be used as texture coordinate initialized
	// by transforming LightSpacePos from NDC to texture space.                          
    vec2 UVCoords;                                                                          
    UVCoords.x = 0.5 * ProjCoords.x + 0.5;                                                  
    UVCoords.y = 0.5 * ProjCoords.y + 0.5;        
	
                                        
    float z = 0.5 * ProjCoords.z + 0.5;  // pixel depth   
	// use above texture coordinates to fetch the depth from the shadow map
	// this is the depth of the closest location                                                   
    float Depth = texture(gShadowMap, UVCoords).x;  


//	float multiplier = clamp(dot(Normal, LightDirection) * 8.f,.8,1);

	z = (z-MIN_DEPTH) / (MAX_DEPTH - MIN_DEPTH);
	z = pow(M_E, -c*z );  
	float sf = Depth * z; // shadow factor. 1 - shadow.  0 - no shadow
	clamp(sf*1,0,1);
	return sf;
                                                            
}                                                                                           
  /*float bias = 0.05*tan(acos( dot(Normal, LightDirection) ) ); // cosTheta is dot( n,l ), clamped between 0 and 1
bias = clamp(bias, 0,0.01);

	if (Depth < z + bias)                                                                 
        return 0.5;                                                                         
    else                                                                                    
        return 1.0;     
	*/	    
	
	                                                                                
vec4 CalcLightInternal(BaseLight Light, vec3 LightDirection, vec3 Normal,            
                       float ShadowFactor)                                                  
{                                                                                           
    vec4 AmbientColor = vec4(Light.Color * Light.AmbientIntensity, 1.0f);
    float DiffuseFactor = dot(Normal, -LightDirection);                                     
                                                                                            
    vec4 DiffuseColor  = vec4(0, 0, 0, 0);                                                  
    vec4 SpecularColor = vec4(0, 0, 0, 0);                                                  
                                                                                            
    if (DiffuseFactor > 0) {                                                                
        DiffuseColor = vec4(Light.Color * Light.DiffuseIntensity * DiffuseFactor, 1.0f);    
                                                                                            
        vec3 VertexToEye = normalize(gEyeWorldPos - WorldPos0);                             
        vec3 LightReflect = normalize(reflect(LightDirection, Normal));                     
        float SpecularFactor = dot(VertexToEye, LightReflect);                                      
        if (SpecularFactor > 0) {                                                           
            SpecularFactor = pow(SpecularFactor, gSpecularPower);                               
            SpecularColor = vec4(Light.Color, 1.0f) * gMatSpecularIntensity * SpecularFactor;                         
        }                                                                                   
    }                                                                                       
                                                                                            
    return (AmbientColor + (1-ShadowFactor) * (DiffuseColor + SpecularColor));                 
}                                                                                           
                                                                                            
                                                                                       
                                                                                            
vec4 CalcPointLight(PointLight l, vec3 Normal, vec4 LightSpacePos)                   
{                                                                                           
    vec3 LightDirection = WorldPos0 - l.Position;                                           
    float Distance = length(LightDirection);                                                
    LightDirection = normalize(LightDirection);                                             
    float ShadowFactor = CalcShadowFactor(LightSpacePos, Normal, LightDirection);                                   
                                                                                            
    vec4 Color = CalcLightInternal(l.Base, LightDirection, Normal, ShadowFactor);           
    float Attenuation =  l.Atten.Constant +                                                 
                         l.Atten.Linear * Distance +                                        
                         l.Atten.Exp * Distance * Distance;                                 
                                                                                            
    return Color / 1.f;                                                             
}                                                                                           
                                                                                            
vec4 CalcSpotLight(SpotLight l, vec3 Normal, vec4 LightSpacePos)                     
{                                                                                           
    vec3 LightToPixel = normalize(WorldPos0 - l.Base.Position);                             
    float SpotFactor = dot(LightToPixel, l.Direction);                                      
                                                                                            
    if (SpotFactor > l.Cutoff) {    
                                                  
        vec4 Color = CalcPointLight(l.Base, Normal, LightSpacePos);                         
        return Color * (1.0 - (1.0 - SpotFactor) * 1.0/(1.0 - 0));                   
    }                                                                                       
    else {                                                                                  
        return vec4(0,0,0,0);                                                               
    }                                                                                       
}                                                                                
void main()                                                                                 
{                   
	  if (isQuad)
	{

	}                                                                        
   	  vec3 Normal = normalize(Normal0);                                                       
    vec4 TotalLight = vec4(1,1,1,0);                          
                                                                                
                                         
    //TotalLight += CalcSpotLight(gSpotLight, Normal, LightSpacePos);                 
                                                                                       
                                                                                            
    vec4 SampledColor = texture2D(gSampler, TexCoord0.xy);

	// apply tone mapping linear to rgb
	SampledColor *= Exposure;
	SampledColor = vec4(ToneMap(vec3(SampledColor.x,SampledColor.y, SampledColor.z)),1);
	SampledColor = l2rgb(SampledColor);
                                  
    FragColor = SampledColor;
              /*          	
	vec2 uv = gl_FragCoord.xy / vec2(592,592);
	float col = 1-texture(gShadowMap, uv).x;  	
	FragColor = vec4(col,col,col,1);	*/	  		           
}
