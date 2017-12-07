#version 430 
//combines ambient occlusion and IBL using deferred rendering.

#define pi   3.1415926535897932384626433832795
#define pi2  6.2831853071795864769252867665590
const float MIN_DEPTH = .3f;
const float MAX_DEPTH = 100.0f; 

// ambient occlusion constants
const float ROI = 1.f;	// range of influence
const float AOC = .1f;	// .1 * ROI, const for falloff function
const float DELTA = 0.001f;


uniform sampler2D environment;
uniform sampler2D irradiance;
uniform sampler2D positionMap;// from gbuffer
uniform sampler2D normalMap;	// from gbuffer
//uniform sampler2D depthMap;	//from gbuffer dont think we need this here
uniform sampler2D colorMap;
uniform sampler2D aoMap;	// this is the specular map, it stores the AO factor in y component


uniform vec3 eyePos;
uniform vec2 screenDim;
uniform vec4 objColor = vec4(1,1,1,1);
uniform float metallicness = .95f;
uniform float roughness = 0.01f;
uniform int randomness = 10;

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
	

out vec4 FragColor;

in vec3 Normal0;  
in vec3 WorldPos0;      

vec2 SphereMap(vec3 N)
{
  return vec2(0.5 - atan(N.y, N.x) / pi2, acos(N.z) / pi);
}

// returns random low discrepency points which functions well in our
// quasi-monte carlo distribution method
vec2 Hammersley(uint i, uint N)
{
  return vec2( 
	float(i) / float(N), 
	float(bitfieldReverse(i)) * 2.3283064365386963e-10 );
}

float randAngle()
{
  uint x = uint(gl_FragCoord.x);
  uint y = uint(gl_FragCoord.y);
  return (30u* x ^ y + 10u * x * y);
}



vec3 MakeSample(float Theta, float Phi)
{
  Phi += randAngle();
  float SineTheta = sin(Theta);

  float x = cos(Phi) * SineTheta;
  float y = sin(Phi) * SineTheta;
  float z = cos(Theta);

  return vec3(x, y, z);
}

// D() used for sampling random points
vec3 DGgxSample(vec2 E)
{
  float a = roughness * roughness;
  float Theta = atan(sqrt((a * E.x) / (1.0 - E.x)));
  float Phi = pi2 * E.y;
  return MakeSample(Theta, Phi);
}

float DGgx(float NdotH)
{
	float NdotH2 = NdotH * NdotH;
	float k = roughness * roughness;
	float dn = 1.0 + NdotH2 * (k - 1.0);
	return k / (pi * dn * dn);
}

// F() fresnel factor
float Schlick(float VdotH)
{
  // todo: get metallicness from surface texture instead of constant
  return metallicness + (1.0 - metallicness) * pow(1.0 - VdotH, 5.0);
}

// G() geometric occlusion factor
float SchlickBeckmann(float NdotV, float NdotH, float LdotN, float VdotH)
{
	float k = roughness * roughness * sqrt(pi2);
	return (NdotV / (NdotV * (1.0-k) + k)) * (LdotN /(LdotN * (1.0-k) + k));
}

float CalcLOD(uint numSamples, float NdotH)
{
	return 0.5 * (log2( (screenDim.x *  screenDim.y) / numSamples) - log2(DGgx(NdotH)));
}



vec3 CalcSpecular(vec3 V, vec3 N)
{
	vec3 up;
	if (abs(N.z) < .99f)
		up = vec3(0,0,1.f);
	else
		up = vec3(1.f,0,0);
	vec3 orthoX = normalize( cross(up, N));
	vec3 orthoY = cross(N, orthoX);

	float NdotV = abs(dot(N, V));

	vec3 totalSpecular = vec3(0);
	int numSamples = 20;
	// do a sample for each random light direction and add to total specular
	for (uint i = 0; i < numSamples; i++)
	{
		// calculate random light direction that comes from distribution
		// that has probability that matches the D() function
		vec2 randPoint = Hammersley(i,numSamples);
		vec3 Wi = DGgxSample(randPoint);
		// half vector
		vec3 H = normalize(Wi.x *orthoX + Wi.y * orthoY + Wi.z * N);
		vec3 L = normalize(-reflect(V,H));

		float NdotH = abs(dot(N,H));
		float VdotH = abs(dot(V,H));
		float LdotN = abs(dot(N, L));
		float lod = CalcLOD(numSamples,  NdotH); 

		float G = SchlickBeckmann(NdotV, NdotH, LdotN, VdotH);
		float F = Schlick(VdotH);
		vec3 Lcolor = textureLod(environment, SphereMap(L), lod).rgb;
		// distribution factor is cacnelled from the BRDF due to sample skewing.
		totalSpecular += F * G * Lcolor * VdotH / (NdotH * NdotV);
		
	}
	return totalSpecular / numSamples;
}

vec4 CombineLights(vec4 diffuse, vec4 spec)
{
	float mrange = smoothstep(0.25, 0.45, metallicness);
	vec4 dielectric = diffuse+spec;
	vec4 metal = objColor * spec;
	return mix(dielectric, metal, mrange);
}

vec2 CalcTexCoord()
{
    return gl_FragCoord.xy / screenDim;
}

// pixelDepth is camera space depth
float CalcAOFactor(vec3 N, vec3 P, float pixelDepth)
{
	int numSamples = 20;
	float occlSum = 0;
	for (int i = 0; i < numSamples; i++)
	{
		float alpha = (i + 0.5f) / numSamples;
		float h = alpha * ROI / pixelDepth;

		uint x = uint(gl_FragCoord.x);
		uint y = uint(gl_FragCoord.y);
		float phi = (30u* x ^ y + 10u * x * y);
		float theta = pi2 * alpha * (7*numSamples/9) + phi;

		vec2 pointiCoord = vec2(x,y) + h*vec2(cos(theta), sin(theta));
		vec3 pointi = texture(positionMap, pointiCoord).xyz;
		vec3 omegai = pointi - P;

		// heaviside step function
		if (ROI - length(omegai) < 0)
			continue;
		else
		{
			float NoW = dot(N, omegai);
			occlSum += max(0, NoW-DELTA*pixelDepth) / max(AOC*AOC, dot(omegai, omegai));
		}
	}
	occlSum *= (pi2*AOC)/numSamples;
	float scale = 10.f;
	float contrast = 2.5f;
	return clamp( pow(1-scale*occlSum, contrast), 0, 1 );
}

void main()
{
	vec2 TexCoord = CalcTexCoord();
	
		// perspective division to transform vector to NDC space.                            
    vec3 pixPos = texture(positionMap, TexCoord).xyz;       
	//pixPos = texture(aoMap, TexCoord).xyz;
	if (pixPos.x < 20f && pixPos.x > -20.f && pixPos.y < 20f && pixPos.y > -20.f &&
		pixPos.z < 20f && pixPos.z > -20.f)
	{
		vec4 kd = vec4(1.f,1.f,1.f,1.f);

		vec3 N = texture(normalMap, TexCoord).xyz;
		vec3 V = normalize(pixPos - eyePos);
		vec3 L = normalize(-reflect(V, N));
		vec4 irrMapColor = textureLod(irradiance, SphereMap(N), 0 );
		vec4 diffuse = irrMapColor * objColor / pi;
		vec4 spec = vec4(CalcSpecular(V, N), 1);
		vec4 finalColor =  CombineLights(diffuse, spec);//(spec+diffuse) * objColor;
	
			// apply tone mapping linear to rgb
		finalColor *= Exposure;
		finalColor = vec4(ToneMap(vec3(finalColor.x,finalColor.y, finalColor.z)),1);
		finalColor = l2rgb(finalColor);
	
		float depth = ((texture(aoMap, TexCoord).x-MIN_DEPTH) / (MAX_DEPTH - MIN_DEPTH))*10.f;
		depth = 1f;
		float AOfactor = texture(aoMap, TexCoord).x;
		FragColor = finalColor * AOfactor;
		//FragColor = vec4(texture(positionMap, TexCoord));
	}
	else
	{
		// draw sky dome
		//vec3 Color = texture(colorMap, TexCoord).xyz;
		//FragColor = vec4(Color, 1.0) ;
	}
	/*
	vec2 uv = gl_FragCoord.xy / vec2(592,592);
	float col = texture(aoMap, TexCoord).x;
	FragColor = vec4(col,col,col,1);	*/
}

