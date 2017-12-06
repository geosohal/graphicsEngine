#pragma once

#include <glm/glm.hpp>
#include "texture.h"

using namespace glm;
class BaseLight
{
public:
	std::string name;
	vec3 color;
	float ambientIntensity;
	float diffuseIntensity;

	BaseLight()
	{
		color = vec3(0.0f, 0.0f, 0.0f);
		ambientIntensity = 0.0f;
		diffuseIntensity = 0.0f;
	}

};
struct LightAttenuation
{
	float constant;
	float linear;
	float exp;

	LightAttenuation()
	{
		constant = 1.0f;
		linear = 0.0f;
		exp = 0.0f;
	}
};
class PointLight : public BaseLight
{
public:
	vec3 pos;
	LightAttenuation atten;

	PointLight()
	{
		pos = vec3(0.0f, 0.0f, 0.0f);
	}
};

class DirectionalLight : public BaseLight
{
public:
	vec3 direction;

	DirectionalLight()
	{
		direction = vec3(0.0f, 0.0f, 0.0f);
	}

};

class SpotLight : public BaseLight
{
public:
	vec3 direction;
	vec3 pos;
	float cutoff;
	LightAttenuation atten;

	SpotLight()
	{
		pos = vec3(0.0f, 0.0f, 0.0f);
		direction = vec3(0.0f, 0.0f, 0.0f);
		cutoff = 10.f;
	}
};