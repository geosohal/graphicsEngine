#pragma once
#include "Quaternion.h"

using namespace glm;
class VQS
{
public:
	VQS();
	VQS(MAT4 const& m);
	VQS(vec3 v, Quaternion q, float s);
	MAT4 ToMat4() const;
	static VQS Interpolate(VQS const& q1, VQS const& q2, float t);
	//vec3 Rotate(vec3 const& v) const;

	
	vec3 operator*(vec3 const& v) ;
	VQS operator *(VQS const& rhs) ;
	VQS operator+(VQS const& rhs) const;
	
	vec3 v;
	Quaternion q;
	float s;

//	aiQuaternion qt;
};