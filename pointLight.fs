#version 330

struct BaseLight
{
    vec3 Color;
    float AmbientIntensity;
    float DiffuseIntensity;
};


struct Attenuation
{
    float Constant;
    float Linear;
    float Exp;
};

struct PointLight
{
    BaseLight Base;
    vec3 Position;
    Attenuation Atten;
};


uniform sampler2D gPositionMap;
uniform sampler2D gColorMap;
uniform sampler2D gNormalMap;
uniform PointLight gPointLight;
uniform vec3 gEyeWorldPos;
uniform float gMatSpecularIntensity = 1.0f;
uniform float gSpecularPower =36;
uniform int gLightType;
uniform vec2 gScreenSize;



vec4 GetPointLight(vec3 WorldPos, vec3 Normal)
{
    vec3 LightDirection = WorldPos - gPointLight.Position;
    float Distance = length(LightDirection);
    LightDirection = normalize(LightDirection);


	vec4 AmbientColor = vec4(gPointLight.Base.Color * gPointLight.Base.AmbientIntensity, 1.0);
    float DiffuseFactor = 1;
    vec4 DiffuseColor  = vec4(0, 0, 0, 0);
    vec4 SpecularColor = vec4(0, 0, 0, 0);

    if (DiffuseFactor > 0.0) {
        DiffuseColor = vec4(gPointLight.Base.Color * gPointLight.Base.DiffuseIntensity * DiffuseFactor, 1.0);

        vec3 VertexToEye = normalize(gEyeWorldPos - WorldPos);
        vec3 LightReflect = normalize(reflect(LightDirection, Normal));
        float SpecularFactor = dot(VertexToEye, LightReflect);        
        if (SpecularFactor > 0.0) {
            SpecularFactor = pow(SpecularFactor, gSpecularPower);
            SpecularColor = vec4(gPointLight.Base.Color * gMatSpecularIntensity * SpecularFactor, 1.0);
        }
    }
	vec4 Color = (AmbientColor + DiffuseColor + SpecularColor);

    float Attenuation =  gPointLight.Atten.Constant +
                         gPointLight.Atten.Linear * Distance +
                         gPointLight.Atten.Exp * Distance * Distance;
    Attenuation = max(1.0, Attenuation);
	//Attenuation = 1.0;
    return Color / Attenuation;
}

vec2 CalcTexCoord()
{
    return gl_FragCoord.xy / gScreenSize;
}

out vec4 FragColor;

void main()
{
    vec2 TexCoord = CalcTexCoord();
	vec3 WorldPos = texture(gPositionMap, TexCoord).xyz;
	vec3 Color = texture(gColorMap, TexCoord).xyz;
	vec3 Normal = texture(gNormalMap, TexCoord).xyz;
	Normal = normalize(Normal);

	FragColor = vec4(Color, 1.0) * GetPointLight(WorldPos, Normal);
}