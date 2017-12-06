///////////////////////////////////////////////////////////////////////
// A slight encapsulation of a shader program. This contains methods
// to build a shader program from multiples files containing vertex
// and pixel shader code, and a method to link the result.  When
// loaded (method "Use"), its vertex shader and pixel shader will be
// invoked for all geometry passing through the graphics pipeline.
// When done, unload it with method "Unuse".
//
// Copyright 2013 DigiPen Institute of Technology
////////////////////////////////////////////////////////////////////////


#include <fstream>

#include <glbinding/gl/gl.h>
#include <glbinding/Binding.h>
using namespace gl;


#include <freeglut.h>

#include "shader.h"
#include <glu.h>                // For gluErrorString
#define CHECKERRORNOX {GLenum err = glGetError(); if (err != GL_NO_ERROR) { fprintf(stderr, "OpenGL Serror (at line %d): %s\n", __LINE__, gluErrorString(err)); } }
#define ARRAY_SIZE_IN_ELEMENTS(a) (sizeof(a)/sizeof(a[0]))

// Reads a specified file into a string and returns the string.
char* ReadFile(const char* name)
{
	std::ifstream f;
	f.open(name, std::ios_base::binary); // Open
	f.seekg(0, std::ios_base::end);      // Position at end
	int length = f.tellg();              //   to get the length

	char* content = new char[length + 1]; // Create buffer of needed length
	f.seekg(0, std::ios_base::beg);     // Position at beginning
	f.read(content, length);            //   to read complete file
	f.close();                           // Close

	content[length] = char(0);           // Finish with a NULL
	return content;
}

// Creates an empty shader program.
ShaderProgram::ShaderProgram()
{
	programId = glCreateProgram();
}

// Use a shader program
void ShaderProgram::Use()
{
	glUseProgram(programId);
	CHECKERRORNOX
}

// Done using a shader program
void ShaderProgram::Unuse()
{
	glUseProgram(0);
}

// Read, send to OpenGL, and compile a single file into a shader program.
void ShaderProgram::AddShader(const char* fileName, GLenum type)
{
	// Read the source from the named file
	char* src = ReadFile(fileName);
	const char* psrc[1] = { src };

	// Create a shader and attach, hand it the source, and compile it.
	int shader = glCreateShader(type);
	glAttachShader(programId, shader);
	glShaderSource(shader, 1, psrc, NULL);
	glCompileShader(shader);
	delete src;

	// Get the compilation status
	int status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

	// If compilation status is not OK, get and print the log message.
	if (status != 1) {
		int length;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
		char* buffer = new char[length];
		glGetShaderInfoLog(shader, length, NULL, buffer);
		printf("Compile log for %s:\n%s\n", fileName, buffer);
		delete buffer;
	}
}

void ShaderProgram::LinkProgram()
{
	// Link program and check the status
	glLinkProgram(programId);
	int status;
	glGetProgramiv(programId, GL_LINK_STATUS, &status);

	// If link failed, get and print log
	if (status != 1) {
		int length;
		glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &length);
		char* buffer = new char[length];
		glGetProgramInfoLog(programId, length, NULL, buffer);
		printf("Link log:\n%s\n", buffer);
		delete buffer;
	}
}

void ShaderProgram::SetUniformi(const char* uniform, int val)
{
	int loc = glGetUniformLocation(programId, uniform);
	glUniform1i(loc, val);
	CHECKERRORNOX
}
void ShaderProgram::SetUniformf(const char* uniform, float val)
{
	int loc = glGetUniformLocation(programId, uniform);
	glUniform1f(loc, val);
	CHECKERRORNOX
}
void IBLProgram::Initialize()
{
	AddShader("IBL.vs", GL_VERTEX_SHADER);
	AddShader("IBL.fs", GL_FRAGMENT_SHADER);
	LinkProgram();

	envMapTextUnitLoc = glGetUniformLocation(programId, "environment");
	CHECKERRORNOX
	irradTextUnitLoc = glGetUniformLocation(programId, "irradiance");
	CHECKERRORNOX
	gVPLoc = glGetUniformLocation(programId, "gVP");
	CHECKERRORNOX
	worldMatrixLoc = glGetUniformLocation(programId, "gWorld");
	CHECKERRORNOX
	eyePosLoc = glGetUniformLocation(programId, "eyePos");
	CHECKERRORNOX
	screenDimLoc = glGetUniformLocation(programId, "screenDim");
	CHECKERRORNOX
	metallicLoc = glGetUniformLocation(programId, "metallicness");
	CHECKERRORNOX
	roughnessLoc = glGetUniformLocation(programId, "roughness");
	CHECKERRORNOX
		randomnessLoc = glGetUniformLocation(programId, "randomness");
	normalBufferLoc = glGetUniformLocation(programId, "normalMap");
	posBufferLoc = glGetUniformLocation(programId, "positionMap");
	depthMapLoc = glGetUniformLocation(programId, "depthMap");
	specMapLoc = glGetUniformLocation(programId, "aoMap");
}

void IBLProgram::SetRandomness(float r)
{
	glUniform1f(randomnessLoc, r);
}
void IBLProgram::SetRoughness(float r)
{
	glUniform1f(roughnessLoc, r);
	CHECKERRORNOX
}

void IBLProgram::SetScreenDim()
{
	glUniform2f(screenDimLoc, (float)592, (float)592);
	CHECKERRORNOX
}
void IBLProgram::setMetallic(float m)
{
	glUniform1f(metallicLoc, m);
	CHECKERRORNOX
}

void IBLProgram::SetEyePos(vec3 eye)
{
	glUniform3fv(eyePosLoc, 1, &eye[0]);
	CHECKERRORNOX
}

void IBLProgram::SetEnvMapTU(unsigned int textureUnit)
{
	glUniform1i(envMapTextUnitLoc, textureUnit);
	CHECKERRORNOX
}

void IBLProgram::SetIrradTU(unsigned int textureUnit)
{
	glUniform1i(irradTextUnitLoc, textureUnit);
	CHECKERRORNOX
}

void IBLProgram::SetWorldMatrix(const MAT4& w)
{
	glUniformMatrix4fv(worldMatrixLoc, 1, GL_TRUE, (const GLfloat*)w.M);
	CHECKERRORNOX
}

void IBLProgram::SetVP(const MAT4& transform)
{
	glUniformMatrix4fv(gVPLoc, 1, GL_TRUE, (const GLfloat*)transform.M);
	CHECKERRORNOX
}

void IBLProgram::SetPosTU(unsigned int TU)
{
	glUniform1i(posBufferLoc, TU);
	CHECKERRORNOX
}
void IBLProgram::SetNormalTU(unsigned int TU)
{
	glUniform1i(normalBufferLoc, TU);
	CHECKERRORNOX
}

void IBLProgram::SetDepthMap(unsigned int TU)
{
	glUniform1i(depthMapLoc, TU);
	CHECKERRORNOX
}

void IBLProgram::SetAOMap(unsigned int TU)
{
	glUniform1i(specMapLoc, TU);
	CHECKERRORNOX
}
void ShadowProgram::Initialize()
{
	AddShader("shadowmap.vs", GL_VERTEX_SHADER);
	AddShader("shadowmap.fs", GL_FRAGMENT_SHADER);
	CHECKERRORNOX
	LinkProgram();

	wvpLocation = glGetUniformLocation(programId, "gWVP");
	textureUnitLoc = glGetUniformLocation(programId, "gShadowMap");
}

void ShadowProgram::SetWVP(const MAT4& transform)
{
	glUniformMatrix4fv(wvpLocation, 1, GL_TRUE, (const GLfloat*)transform.M);
	CHECKERRORNOX
}

void ShadowProgram::SetTextureUnit(unsigned int textureUnit)
{
	glUniform1i(textureUnitLoc, textureUnit);
	CHECKERRORNOX
}

SkinProgram::SkinProgram()
{
	programId = glCreateProgram();
}
void SkinProgram::Initialize()
{
	AddShader("skinningShader.vs", GL_VERTEX_SHADER);
	AddShader("skinningShader.fs", GL_FRAGMENT_SHADER);
	CHECKERRORNOX
	LinkProgram();
	for (unsigned int i = 0; i < ARRAY_SIZE_IN_ELEMENTS(boneLocation); i++)
	{
		char name[128];
		memset(name, 0, sizeof(name));
		_snprintf_s(name, sizeof(name), "gBones[%d]", i);
		boneLocation[i] = glGetUniformLocation(programId, name);
	}
	eyeWorldPosLocation = glGetUniformLocation(programId, "gEyeWorldPos"); 
	colorTextureLocation = glGetUniformLocation(programId, "gColorMap");
}

void SkinProgram::SetBoneTransform(unsigned int boneIndex, MAT4 transform)
{
	glUniformMatrix4fv(boneLocation[boneIndex], 1, GL_TRUE, transform.Pntr());
}
void SkinProgram::SetColorTextureUnit(uint textureUnit)
{
	glUniform1i(colorTextureLocation, textureUnit);
}
void SkinProgram::SetEyeWorldPos(float x, float y, float z)
{
	printf("set eye wold pos?");
}

SpotLightProgram::SpotLightProgram()
{
	programId = glCreateProgram();
}

void SpotLightProgram::Initialize()
{
	AddShader("spotlight.vs", GL_VERTEX_SHADER);
	AddShader("spotlight.fs", GL_FRAGMENT_SHADER);
	LinkProgram();
	CHECKERRORNOX
	spotLightLoc.Color = glGetUniformLocation(programId, "gSpotLight.Base.Base.Color");
	CHECKERRORNOX
	spotLightLoc.AmbientIntensity = glGetUniformLocation(programId, "gSpotLight.Base.Base.AmbientIntensity");
	spotLightLoc.Position = glGetUniformLocation(programId, "gSpotLight.Base.Position");
	CHECKERRORNOX
	spotLightLoc.DiffuseIntensity = glGetUniformLocation(programId, "gSpotLight.Base.Base.DiffuseIntensity");
	spotLightLoc.Atten.Constant = glGetUniformLocation(programId, "gSpotLight.Base.Atten.Constant");
	spotLightLoc.Atten.Linear = glGetUniformLocation(programId, "gSpotLight.Base.Atten.Linear");
	spotLightLoc.Atten.Exp = glGetUniformLocation(programId, "gSpotLight.Base.Atten.Exp");
	spotLightLoc.Cutoff = glGetUniformLocation(programId, "gSpotLight.Cutoff");
	spotLightLoc.Direction = glGetUniformLocation(programId, "gSpotLight.Direction");
	eyePos = glGetUniformLocation(programId, "gEyeWorldPos");
	worldMatrixLoc = glGetUniformLocation(programId, "gWorld");
	shadowMapLoc = glGetUniformLocation(programId, "gShadowMap");
	samplerLoc =  glGetUniformLocation(programId, "gSampler");
	wvpLoc = glGetUniformLocation(programId, "gWVP");
	gLightWVP = glGetUniformLocation(programId, "gLightWVP");
	CHECKERRORNOX

	matSpecularIntensityLocation = glGetUniformLocation(programId, "gMatSpecularIntensity");
	matSpecularPowerLocation = glGetUniformLocation(programId, "gSpecularPower");
	screenSizeLocation = glGetUniformLocation(programId, "gScreenSize");

	shadowMapLoc = glGetUniformLocation(programId, "gShadowMap");;
	LightProgram::Initialize();
}

void SpotLightProgram::SetLightWVP(const MAT4& wvp)
{
	glUniformMatrix4fv(gLightWVP, 1, GL_TRUE, (const GLfloat*)wvp.M);
	CHECKERRORNOX
}

void SpotLightProgram::SetWorldMatrix(const MAT4& w)
{
	glUniformMatrix4fv(worldMatrixLoc, 1, GL_TRUE, (const GLfloat*)w.M);
}
void SpotLightProgram::SetSpotLight(const SpotLight& light)
{
	glUniform3f(spotLightLoc.Color, light.color.x, light.color.y, light.color.z);
	CHECKERRORNOX
	glUniform3f(spotLightLoc.Direction, light.direction.x, light.direction.y, light.direction.z);
	CHECKERRORNOX
	glUniform1f(spotLightLoc.Cutoff, cosf(ToRadian(light.cutoff)));
	glUniform1f(spotLightLoc.AmbientIntensity, light.ambientIntensity);
	glUniform1f(spotLightLoc.DiffuseIntensity, light.diffuseIntensity);
	glUniform3f(spotLightLoc.Position, light.pos.x, light.pos.y, light.pos.z);
	glUniform1f(spotLightLoc.Atten.Constant, light.atten.constant);
	glUniform1f(spotLightLoc.Atten.Linear, light.atten.linear);
	glUniform1f(spotLightLoc.Atten.Exp, light.atten.exp);
	CHECKERRORNOX
}

void SpotLightProgram::SetShadowMapTextUnit(unsigned int textureUnit)
{
	glUniform1i(shadowMapLoc, textureUnit);
}

void SpotLightProgram::SetTextUnit(int textUnit)
{
	glUniform1i(samplerLoc, textUnit);
}

PLightShaderProgram::PLightShaderProgram()
{
	programId = glCreateProgram();
}

void PLightShaderProgram::Initialize()
{


	CHECKERRORNOX
	AddShader("lightPass.vs", GL_VERTEX_SHADER);
	AddShader("pointLight.fs", GL_FRAGMENT_SHADER);
	CHECKERRORNOX
		LinkProgram();
	CHECKERRORNOX
	pointLightLoc.Color = glGetUniformLocation(programId, "gPointLight.Base.Color");
	CHECKERRORNOX
	pointLightLoc.AmbientIntensity = glGetUniformLocation(programId, "gPointLight.Base.AmbientIntensity");
	pointLightLoc.Position = glGetUniformLocation(programId, "gPointLight.Position");
	CHECKERRORNOX
	pointLightLoc.DiffuseIntensity = glGetUniformLocation(programId, "gPointLight.Base.DiffuseIntensity");
	pointLightLoc.Atten.Constant = glGetUniformLocation(programId, "gPointLight.Atten.Constant");
	pointLightLoc.Atten.Linear = glGetUniformLocation(programId, "gPointLight.Atten.Linear");
	pointLightLoc.Atten.Exp = glGetUniformLocation(programId, "gPointLight.Atten.Exp");

	CHECKERRORNOX
	posTextureUnitLocation = glGetUniformLocation(programId, "gPositionMap");
	CHECKERRORNOX
	colorTextureUnitLocation = glGetUniformLocation(programId, "gColorMap");
	normalTextureUnitLocation = glGetUniformLocation(programId, "gNormalMap");

	matSpecularIntensityLocation = glGetUniformLocation(programId, "gMatSpecularIntensity");
	matSpecularPowerLocation = glGetUniformLocation(programId, "gSpecularPower");
	screenSizeLocation = glGetUniformLocation(programId, "gScreenSize");
	CHECKERRORNOX
	LightProgram::Initialize();
}





void PLightShaderProgram::SetCurrLight(const PointLight& light)
{
	glUniform3f(pointLightLoc.Color, light.color.x, light.color.y, light.color.z);
	glUniform1f(pointLightLoc.AmbientIntensity, light.ambientIntensity);
	glUniform1f(pointLightLoc.DiffuseIntensity, light.diffuseIntensity);
	glUniform3f(pointLightLoc.Position, light.pos.x, light.pos.y, light.pos.z);
	glUniform1f(pointLightLoc.Atten.Constant, light.atten.constant);
	glUniform1f(pointLightLoc.Atten.Linear, light.atten.linear);
	glUniform1f(pointLightLoc.Atten.Exp, light.atten.exp);
	CHECKERRORNOX
}

DirLightProgram::DirLightProgram()
{
	programId = glCreateProgram();
	CHECKERRORNOX
}

void DirLightProgram::Initialize()
{
	AddShader("dirLightPass.fs", GL_FRAGMENT_SHADER);
	CHECKERRORNOX
	AddShader("lightPass.vs", GL_VERTEX_SHADER);
	CHECKERRORNOX
	LinkProgram();
	CHECKERRORNOX
	dirLightLocation.Color = glGetUniformLocation(programId, "gDirectionalLight.Base.Color");
	dirLightLocation.AmbientIntensity = glGetUniformLocation(programId, "gDirectionalLight.Base.AmbientIntensity");
	dirLightLocation.Direction = glGetUniformLocation(programId, "gDirectionalLight.Direction");
	dirLightLocation.DiffuseIntensity = glGetUniformLocation(programId, "gDirectionalLight.Base.DiffuseIntensity");
	CHECKERRORNOX
		
		LightProgram::Initialize();
	CHECKERRORNOX

}

void DirLightProgram::SetDirLight(const DirectionalLight& light)
{
	glUniform3f(dirLightLocation.Color, light.color.x, light.color.y,light.color.z);
	glUniform1f(dirLightLocation.AmbientIntensity, light.ambientIntensity);
	vec3 Direction = light.direction;
	Direction == glm::normalize(Direction);
	glUniform3f(dirLightLocation.Direction, Direction.x, Direction.y, Direction.z);
	glUniform1f(dirLightLocation.DiffuseIntensity, light.diffuseIntensity);
	CHECKERRORNOX
}
void LightProgram::Initialize()
{

	posTextureUnitLocation = glGetUniformLocation(programId, "gPositionMap");
	colorTextureUnitLocation = glGetUniformLocation(programId, "gColorMap");
	normalTextureUnitLocation = glGetUniformLocation(programId, "gNormalMap");
	eyeWorldPosLocation = glGetUniformLocation(programId, "gEyeWorldPos");
	CHECKERRORNOX
	WVPLocation = glGetUniformLocation(programId, "gWVP");
	CHECKERRORNOX
	screenSizeLocation = glGetUniformLocation(programId, "gScreenSize");
	CHECKERRORNOX
}
void LightProgram::SetWVP(const MAT4& wvp)
{
	glUniformMatrix4fv(WVPLocation, 1, GL_TRUE, (const GLfloat*)wvp.M);
	CHECKERRORNOX
}


void LightProgram::SetEyeWorldPos(float x, float y, float z)
{
	glUniform3f(eyeWorldPosLocation, x, y, z);
	CHECKERRORNOX
}

void LightProgram::SetPosTextureUnit(unsigned int TextureUnit)
{
	glUniform1i(posTextureUnitLocation, TextureUnit);
	CHECKERRORNOX
}

void LightProgram::SetColorTextureUnit(unsigned int TextureUnit)
{
	glUniform1i(colorTextureUnitLocation, TextureUnit);
	CHECKERRORNOX
}

void LightProgram::SetNormalTextureUnit(unsigned int TextureUnit)
{
	glUniform1i(normalTextureUnitLocation, TextureUnit);
	CHECKERRORNOX
}

void LightProgram::SetScreenSize(int width, int height)
{
	glUniform2f(screenSizeLocation, (float)width, (float)height);
	CHECKERRORNOX
}

