#include "Quaternion.h"

Quaternion::Quaternion()	// set to identity
	: s(1.0f), v(0, 0, 0) {}

Quaternion::Quaternion(float s_, vec3 const& v_)
	: s(s_), v(v_) {}

Quaternion::Quaternion(float s_, float x, float y, float z)
	: s(s_), v(x, y, z) {}

// build quaternion from 4x4 row major matrix
Quaternion::Quaternion(MAT4 m)
{
	s = .5f * sqrtf(m[0][0] + m[1][1] + m[2][2] + 1.0f);
	v = vec3((m[2][1] - m[1][2]) / 4.f*s,
		(m[0][2] - m[2][0]) / 4.f*s,
		(m[1][0] - m[0][1]) / 4.f*s);
}

Quaternion Quaternion::operator+(Quaternion const& q1) const
{
	Quaternion ans;
	ans.s = q1.s + s;
	ans.v = q1.v + v;
	return ans;
}
Quaternion Quaternion::operator*(float c) const
{
	return Quaternion(c*s, c*v);
}
Quaternion Quaternion::operator*(Quaternion const& q) const
{
	Quaternion ans;
	ans.s = q.s*s - dot(q.v, v);
	ans.v = s*q.v + q.s*v + cross(v, q.v);
	return ans;
}

Quaternion operator/(Quaternion const &q, float c)
{
	return Quaternion(q.s / c, q.v / c);
}

// standard euclidean dot product in 4d
float Quaternion::Dot(Quaternion const& q1) const
{
	return s*q1.s + dot(q1.v,v);
}
Quaternion Quaternion::Conjugate() const
{
	return Quaternion(s, -v);
}

// invert quaternion
Quaternion Quaternion::Inverse() const
{
	return Conjugate() / (s*s + v.x*v.x + v.y*v.y + v.z*v.z);
}

/* axis must be normalized */
Quaternion Quaternion::FromAngleAxis(float angle, vec3 axis)
{
	float alpha2 = angle / 2.f;
	return Quaternion(cos(alpha2),  sin(alpha2)*axis);
}

// normalize quaternion to unit length
Quaternion& Quaternion::Normalize()
{
	float mag = Magnitude();
	s /= mag;
	v /= mag;
	return *this;
}

// return magnitude or length of quaternion
float Quaternion::Magnitude() const
{
	return glm::sqrt(s*s + v.x*v.x + v.y*v.y + v.z*v.z);
}

// return the quaternion that rotates from vec z to vec a
Quaternion Quaternion::FromZtoA(vec3 z, vec3 a)
{
	Quaternion ans;
	ans.v = cross(z, a);
	ans.s = glm::sqrt(length(z)*length(a)) + dot(z, a);
	ans.Normalize();
	return ans;
}

// caller must verify this is a unit quaternion. row major output
MAT4 Quaternion::ToMat4() const
{
	MAT4 ans;
	ans[0][0] = 1.f - 2.f * (v.y*v.y + v.z*v.z);
	ans[1][0] = (v.x*v.y - s*v.z) * 2.f;
	ans[2][0] = (v.x*v.z + s*v.y) * 2.f;

	ans[0][1] = 2.f* (v.x*v.y + s*v.z);
	ans[1][1] = 1.f - 2.f * (v.x*v.x + v.z*v.z);
	ans[2][1] = 2.f * (v.y*v.z - s*v.x);

	ans[0][2] = 2.f * (v.x*v.z - s*v.y);
	ans[1][2] = 2.f * (v.y*v.z + s*v.x);
	ans[2][2] = 1.f - 2.f * (v.x*v.x + v.y*v.y);

	return ans;
}

// rotates the vector with this quaternion
vec3 Quaternion::Rotate(vec3 const& v_) const
{
	return (s*s - dot(v, v)) * v_ + 2 * dot(v, v_) * v + 2 * s*cross(v, v_);
}

// spherical linear interpolation, t is in [0:1]
Quaternion Quaternion::Slerp(Quaternion const& q1, Quaternion const& q2, float t)
{
	if (t >= 1)
		return q2;
	else if (t <= 0)
		return q1;

	float dot = q1.Dot(q2);

	assert(dot >= 0);
	if (dot >= 1.0f)
	{	// interpolation is still at first quaternion so return that
		return q1;
	}
	//qt = [ sin(a-ta)q1 + sin(ta)q2) ] / sin(a)
	Quaternion ans;
	float alpha = acos(dot); // is angle between quaternions
	float ta = t*alpha;
	float sa = sin(alpha);
	float q1c = sin(alpha - ta) / sa;
	float q2c = sin(ta) / sa;
	ans = q1 * q1c + q2 * q2c ;

	// ans will already be normalized if first two quaternions already are, but
	// in case not:
	ans.Normalize();
	return ans;
}

// when t's [0:1] can be divided into n fixed steps we can use iSlerp
// this is just an algorithm, to actually use it the function would need some changes
Quaternion Quaternion::iSlerp(Quaternion const& q1, Quaternion const& q2,float k, float t)
{
	//each interval is a constant: deltaT = 1/n
	// t changes by steps of deltaT
	// t = k * deltaT, ta = k*beta, where beta = alpha / n
	if (t >= 1)
		return q2;
	else if (t <= 0)
		return q1;

	float dot = q1.Dot(q2);
	Quaternion q0 = q2 +  (q1 * dot * -1);
	float b = acos(dot);
	float kt = b / k;
	for (int i = 2; k + 1; i++)
	{
		q0 = q1*cos(b) + q0*sin(b);
		b += kt;
	}

}