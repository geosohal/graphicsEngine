#pragma once
#include <glm/glm.hpp>
#include "transform.h"
using namespace glm;
// i^2 = j^2 = k^2 = ijk = −1
class Quaternion
{
public:
	Quaternion();
	Quaternion(float s, vec3 const& v);
	Quaternion(float s, float x, float y, float z);
	Quaternion(MAT4 m);
	MAT4 ToMat4() const;
	mat4 ToMat4g() const;

	Quaternion operator+(Quaternion const& q1) const;
	Quaternion operator*(Quaternion const& q1) const;
	Quaternion operator*(float c) const;
	friend Quaternion operator/(Quaternion const &q, float c);
	float Dot(Quaternion const& q1) const;

	Quaternion Conjugate() const;
	Quaternion Inverse() const;
	
	
	Quaternion& Normalize();

	float Magnitude() const;
	vec3 Rotate(vec3 const& v_) const;

	static Quaternion Slerp(Quaternion const& q1, Quaternion const& q2, float t);
	static Quaternion iSlerp(Quaternion const& q1, Quaternion const& q2, float n, float t);
	static Quaternion FromZtoA(vec3 z, vec3 a);
	static Quaternion FromAngleAxis(float angle, vec3 axis);
	static Quaternion EulerToQuaternion(float roll, float pitch, float yaw);
	static vec3 ToEulerAngle(Quaternion const& q); // vec returns roll, pitch and yaw respectively
	float s;
	vec3 v;
};


