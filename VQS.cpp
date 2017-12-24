#include "VQS.h"
#include "utility.h"

VQS::VQS() 
	: s(1.f) {

}

VQS::VQS(vec3 v_, Quaternion q_, float s_)
	: v(v_), q(q_), s(s_) {
//	qt.w = q_.s;
	//qt.x = q_.v.x;
	//qt.y = q_.v.y;
	//qt.z = q_.v.z;
}

// ctor assume no scaling in transformations
VQS::VQS(MAT4 const& m)
{
	v = vec3(m[0][3], m[1][3], m[2][3]);
	q = Quaternion(m).Normalize();
	//qt = aiQuaternion(q.s, v.x, v.y, v.z);
//	qt = qt.Normalize();

	s = 1.f; // assume no scaling in transformations
}



// concatenates VQS's together
VQS VQS::operator*(VQS const& rhs) 
{
	/*VQS ans;
	ans.v = *this * rhs.v;

	aiQuaternion testq;
	testq.w = rhs.qt.w;
	testq.x = rhs.qt.x;
	testq.y = rhs.qt.y;
	testq.z = rhs.qt.z;
	testq = qt * testq;
	ans.q = Quaternion(testq.w, vec3(testq.x, testq.y, testq.z));
	ans.qt = testq;

	ans.s = s * rhs.s;
	*/
	VQS ans2;
	VQS& lhs = *this;
	ans2.s = lhs.s*rhs.s;
	ans2.v = lhs * rhs.v;
	ans2.q = lhs.q * rhs.q;
	return ans2;
}

// transforms vector by this VQS
vec3 VQS::operator*(vec3 const& v_) 
{

/*	aiVector3t<float> vec = aiVector3t<float>(v_.x, v_.y, v_.z);
	qt.Rotate(vec);
	vec = vec + aiVector3t<float>(v.x, v.y, v.z);
	return vec3(vec.x, vec.y, vec.z);
	*/
	return q.Rotate(v_ * s) + v;
}

VQS VQS::Interpolate(VQS const& q1, VQS const& q2, float t)
{
	VQS ans;
	ans.s = Util::Lerp(q1.s, q2.s, t);
	ans.v = Util::Lerp(q1.v, q2.v, t);
	ans.q = Quaternion::Slerp(q1.q, q2.q, t);
//	ans.qt = aiQuaternion(ans.q.s, ans.q.v.x, ans.q.v.y, ans.q.v.z);
	return ans;
}

VQS VQS::operator+(VQS const& rhs) const
{
	return VQS(v + rhs.v, q + rhs.q, s + rhs.s);
}

// convert VQS to a matrix
MAT4 VQS::ToMat4() const
{
	MAT4 ans = q.ToMat4();//.ScalarMultiply(s);
	ans[0][3] = v.x;
	ans[1][3] = v.y;
	ans[2][3] = v.z;
	return ans;
}

// convert VQS to a colum major matrix
MAT4 VQS::ToMat4cm() const
{
	MAT4 ans = q.ToMat4();//.ScalarMultiply(s);
	ans[3][0] = v.x;
	ans[3][1] = v.y;
	ans[3][2] = v.z;
	return ans;
}