 #include <fstream>

#include "math.h"
#include "transform.h"
#define SOLUTION
// This is used to communicate a MAT4's address to OpenGL
float* MAT4::Pntr()
{
    return &(M[0][0]);
}

// Return a rotation matrix around an axis (0:X, 1:Y, 2:Z) 
// by an angle measured in degrees.
// NOTE:  Make sure to convert degrees to radians before using sin and cos:
//        radians = degrees*PI/180
const float pi = 3.14159f;
MAT4 Rotate(const int i, const float theta)
{
    MAT4 R;
#ifdef SOLUTION
    int j = (i+1) % 3;
    int k = (j+1) % 3;
    R[j][j] = R[k][k] = cos(theta*pi/180.0f);
    R[k][j] = sin(theta*pi/180.0f);
    R[j][k] = -R[k][j];
#endif
    return R;
}

// Return a scale matrix
MAT4 Scale(const float x, const float y, const float z)
{
    MAT4 S;
//#ifdef SOLUTION
    S[0][0] = x;
    S[1][1] = y;
    S[2][2] = z;
//#endif
    return S;
}

// Return a translation matrix
MAT4 Translate(const float x, const float y, const float z)
{
    MAT4 T;
//#ifdef SOLUTION
    T[0][3] = x;
    T[1][3] = y;
    T[2][3] = z;
//#endif
    return T;
}

MAT4 Translate(vec3 t)
{
	return Translate(t.x, t.y, t.z);
}


// Returns a perspective projection matrix
MAT4 Perspective(const float rx, const float ry,
             const float front, const float back)
{
    MAT4 P;
#ifdef SOLUTION
    P[0][0] = 1.0/rx;
    P[1][1] = 1.0/ry;
    P[2][2] = -(back+front)/(back-front);
    P[2][3] = -(2.0f*front*back)/(back-front);
    P[3][2] = -1;
    P[3][3] = 0;
#endif
    return P;
}
#define ToRadian(x) (float)(((x) * M_PI / 180.0f))
MAT4 Perspective2(const float front, const float back, float fov, float w, float h)
{
	const float ar = w / h;
	const float zRange =front - back;
	const float tanHalfFOV = tanf(ToRadian(fov / 2.0f));
	MAT4 m;
	m[0][0] = 1.0f / (tanHalfFOV * ar); m[0][1] = 0.0f;            m[0][2] = 0.0f;            m[0][3] = 0.0;
	m[1][0] = 0.0f;                   m[1][1] = 1.0f / tanHalfFOV; m[1][2] = 0.0f;            m[1][3] = 0.0;
	m[2][0] = 0.0f;                   m[2][1] = 0.0f;            m[2][2] = (-front - back) / zRange; m[2][3] = 2.0f*back*front / zRange;
	m[3][0] = 0.0f;                   m[3][1] = 0.0f;            m[3][2] = 1.0f;            m[3][3] = 0.0;
	return m;
}

MAT4 SwitchRowOrder(const MAT4& mm)
{
	MAT4 m;
	m[0][0] = mm[0][0]; m[0][1] = mm[1][0]; m[0][2] = mm[2][0]; m[0][3] = mm[3][0];
	m[1][0] = mm[0][1]; m[1][1] = mm[1][1]; m[1][2] = mm[2][1]; m[1][3] = mm[3][1];
	m[2][0] = mm[0][2]; m[2][1] = mm[1][2]; m[2][2] = mm[2][2]; m[2][3] = mm[3][2];
	m[3][0] = mm[0][3]; m[3][1] = mm[1][3]; m[3][2] = mm[2][3]; m[3][3] = mm[3][3];
	return m;
}
glm::mat4 MAT4toGLM(const MAT4& M)
{
	glm::mat4 mm;
	mm[0][0] = M[0][0]; mm[0][1] = M[0][1]; mm[0][2] = M[0][2]; mm[0][3] = M[0][3];
	mm[1][0] = M[1][0]; mm[1][1] = M[1][1]; mm[1][2] = M[1][2]; mm[1][3] = M[1][3];
	mm[2][0] = M[2][0]; mm[2][1] = M[2][1]; mm[2][2] = M[2][2]; mm[2][3] = M[2][3];
	mm[3][0] = M[3][0]; mm[3][1] = M[3][1]; mm[3][2] = M[3][2]; mm[3][3] = M[3][3];
	return mm;
}

MAT4 GLMtoMAT4(const glm::mat4& M)
{
	MAT4 mm;
	mm[0][0] = M[0][0]; mm[0][1] = M[0][1]; mm[0][2] = M[0][2]; mm[0][3] = M[0][3];
	mm[1][0] = M[1][0]; mm[1][1] = M[1][1]; mm[1][2] = M[1][2]; mm[1][3] = M[1][3];
	mm[2][0] = M[2][0]; mm[2][1] = M[2][1]; mm[2][2] = M[2][2]; mm[2][3] = M[2][3];
	mm[3][0] = M[3][0]; mm[3][1] = M[3][1]; mm[3][2] = M[3][2]; mm[3][3] = M[3][3];
	return mm;
}
// Multiplies two 4x4 matrices
MAT4 operator* (const MAT4 A, const MAT4 B)
{  
    MAT4 M;
//#ifdef SOLUTION
    for (int i=0; i<4; ++i)
        for (int j=0; j<4; ++j ) {
            M[i][j] = 0.0;
            for (int k=0;  k<4;  ++k)
                M[i][j] += A[i][k] * B[k][j]; }
//#endif
    return M;
}

MAT4 GetView(vec3 pos, vec3 dir, vec3 up)
{
	MAT4 camTranslation = Translate(-pos.x, -pos.y, -pos.z);
	MAT4 camRotation;
	vec3 N = dir;
	N = glm::normalize(N);
	vec3 U = up;
	U = glm::cross(U, N);
	U = glm::normalize(U);
	vec3 V = glm::cross(N, U);

	camRotation[0][0] = U.x;   camRotation[0][1] = U.y;   camRotation[0][2] = U.z;   camRotation[0][3] = 0.0f;
	camRotation[1][0] = V.x;   camRotation[1][1] = V.y;   camRotation[1][2] = V.z;   camRotation[1][3] = 0.0f;
	camRotation[2][0] = N.x;   camRotation[2][1] = N.y;   camRotation[2][2] = N.z;   camRotation[2][3] = 0.0f;
	camRotation[3][0] = 0.0f;  camRotation[3][1] = 0.0f;  camRotation[3][2] = 0.0f;  camRotation[3][3] = 1.0f;

	MAT4 Vtrans = camRotation * camTranslation;
	return Vtrans;

}

MAT4 glTOMAT4(const glm::mat4 gM)
{
	MAT4 mm;
	mm[0][0] = gM[0][0]; mm[0][1] = gM[0][1]; mm[0][2] = gM[0][2]; mm[0][3] = gM[0][3];
	mm[1][0] = gM[1][0]; mm[1][1] = gM[1][1]; mm[1][2] = gM[1][2]; mm[1][3] = gM[1][3];
	mm[2][0] = gM[2][0]; mm[2][1] = gM[2][1]; mm[2][2] = gM[2][2]; mm[2][3] = gM[2][3];
	mm[3][0] = gM[3][0]; mm[3][1] = gM[3][1]; mm[3][2] = gM[3][2]; mm[3][3] = gM[3][3];
	return mm;
}

MAT4 glTOMAT4(const glm::mat3 gM)
{
	MAT4 mm;
	mm[0][0] = gM[0][0]; mm[0][1] = gM[0][1]; mm[0][2] = gM[0][2]; mm[0][3] = 0;
	mm[1][0] = gM[1][0]; mm[1][1] = gM[1][1]; mm[1][2] = gM[1][2]; mm[1][3] = 0;
	mm[2][0] = gM[2][0]; mm[2][1] = gM[2][1]; mm[2][2] = gM[2][2]; mm[2][3] =0;
	mm[3][0] = 0; mm[3][1] = 0; mm[3][2] = 0; mm[3][3] = 1;
	return mm;
}

MAT4 aiToMAT4(const aiMatrix4x4& aiMatrix)
{
	MAT4 m;
	m[0][0] = aiMatrix.a1; m[0][1] = aiMatrix.a2; m[0][2] = aiMatrix.a3; m[0][3] = aiMatrix.a4;
	m[1][0] = aiMatrix.b1; m[1][1] = aiMatrix.b2; m[1][2] = aiMatrix.b3; m[1][3] = aiMatrix.b4;
	m[2][0] = aiMatrix.c1; m[2][1] = aiMatrix.c2; m[2][2] = aiMatrix.c3; m[2][3] = aiMatrix.c4;
	m[3][0] = aiMatrix.d1; m[3][1] = aiMatrix.d2; m[3][2] = aiMatrix.d3; m[3][3] = aiMatrix.d4;
	return m;
}

////////////////////////////////////////////////////////////////////////
// Calculates the inverse of a matrix by performing the gaussian
// matrix reduction with partial pivoting followed by
// back/substitution with the loops manually unrolled.
//
// Taken from Mesa implementation of OpenGL:  http://mesa3d.sourceforge.net/
////////////////////////////////////////////////////////////////////////
#define MAT(m,r,c) ((*m)[r][c])
#define SWAP_ROWS(a, b) { double *_tmp = a; (a)=(b); (b)=_tmp; }

int invert(const MAT4* mat, MAT4* inv)
{
   double wtmp[4][8];
   double m0, m1, m2, m3, s;
   double *r0, *r1, *r2, *r3;

   r0 = wtmp[0], r1 = wtmp[1], r2 = wtmp[2], r3 = wtmp[3];

   r0[0] = MAT(mat,0,0);
   r0[1] = MAT(mat,0,1);
   r0[2] = MAT(mat,0,2);
   r0[3] = MAT(mat,0,3);
   r0[4] = 1.0;
   r0[5] = r0[6] = r0[7] = 0.0;
   
   r1[0] = MAT(mat,1,0);
   r1[1] = MAT(mat,1,1);
   r1[2] = MAT(mat,1,2);
   r1[3] = MAT(mat,1,3);
   r1[5] = 1.0, r1[4] = r1[6] = r1[7] = 0.0;

   r2[0] = MAT(mat,2,0);
   r2[1] = MAT(mat,2,1);
   r2[2] = MAT(mat,2,2);
   r2[3] = MAT(mat,2,3);
   r2[6] = 1.0, r2[4] = r2[5] = r2[7] = 0.0;

   r3[0] = MAT(mat,3,0);
   r3[1] = MAT(mat,3,1);
   r3[2] = MAT(mat,3,2);
   r3[3] = MAT(mat,3,3);
   r3[7] = 1.0, r3[4] = r3[5] = r3[6] = 0.0;

   /* choose pivot - or die */
   if (fabs(r3[0])>fabs(r2[0])) SWAP_ROWS(r3, r2);
   if (fabs(r2[0])>fabs(r1[0])) SWAP_ROWS(r2, r1);
   if (fabs(r1[0])>fabs(r0[0])) SWAP_ROWS(r1, r0);
   if (0.0 == r0[0])  return 0;

   /* eliminate first variable     */
   m1 = r1[0]/r0[0]; m2 = r2[0]/r0[0]; m3 = r3[0]/r0[0];
   s = r0[1]; r1[1] -= m1 * s; r2[1] -= m2 * s; r3[1] -= m3 * s;
   s = r0[2]; r1[2] -= m1 * s; r2[2] -= m2 * s; r3[2] -= m3 * s;
   s = r0[3]; r1[3] -= m1 * s; r2[3] -= m2 * s; r3[3] -= m3 * s;
   s = r0[4];
   if (s != 0.0) { r1[4] -= m1 * s; r2[4] -= m2 * s; r3[4] -= m3 * s; }
   s = r0[5];
   if (s != 0.0) { r1[5] -= m1 * s; r2[5] -= m2 * s; r3[5] -= m3 * s; }
   s = r0[6];
   if (s != 0.0) { r1[6] -= m1 * s; r2[6] -= m2 * s; r3[6] -= m3 * s; }
   s = r0[7];
   if (s != 0.0) { r1[7] -= m1 * s; r2[7] -= m2 * s; r3[7] -= m3 * s; }

   /* choose pivot - or die */
   if (fabs(r3[1])>fabs(r2[1])) SWAP_ROWS(r3, r2);
   if (fabs(r2[1])>fabs(r1[1])) SWAP_ROWS(r2, r1);
   if (0.0 == r1[1])  return 0;

   /* eliminate second variable */
   m2 = r2[1]/r1[1]; m3 = r3[1]/r1[1];
   r2[2] -= m2 * r1[2]; r3[2] -= m3 * r1[2];
   r2[3] -= m2 * r1[3]; r3[3] -= m3 * r1[3];
   s = r1[4]; if (0.0 != s) { r2[4] -= m2 * s; r3[4] -= m3 * s; }
   s = r1[5]; if (0.0 != s) { r2[5] -= m2 * s; r3[5] -= m3 * s; }
   s = r1[6]; if (0.0 != s) { r2[6] -= m2 * s; r3[6] -= m3 * s; }
   s = r1[7]; if (0.0 != s) { r2[7] -= m2 * s; r3[7] -= m3 * s; }

   /* choose pivot - or die */
   if (fabs(r3[2])>fabs(r2[2])) SWAP_ROWS(r3, r2);
   if (0.0 == r2[2])  return 0;

   /* eliminate third variable */
   m3 = r3[2]/r2[2];
   r3[3] -= m3 * r2[3];
   r3[4] -= m3 * r2[4];
   r3[5] -= m3 * r2[5];
   r3[6] -= m3 * r2[6];
   r3[7] -= m3 * r2[7];

   /* last check */
   if (0.0 == r3[3]) return 0;

   s = 1.0F/r3[3];             /* now back substitute row 3 */
   r3[4] *= s; r3[5] *= s; r3[6] *= s; r3[7] *= s;

   m2 = r2[3];                 /* now back substitute row 2 */
   s  = 1.0F/r2[2];
   r2[4] = s * (r2[4] - r3[4] * m2);
   r2[5] = s * (r2[5] - r3[5] * m2);
   r2[6] = s * (r2[6] - r3[6] * m2);
   r2[7] = s * (r2[7] - r3[7] * m2);
   m1 = r1[3];
   r1[4] -= r3[4] * m1;
   r1[5] -= r3[5] * m1;
   r1[6] -= r3[6] * m1;
   r1[7] -= r3[7] * m1;
   m0 = r0[3];
   r0[4] -= r3[4] * m0;
   r0[5] -= r3[5] * m0;
   r0[6] -= r3[6] * m0;
   r0[7] -= r3[7] * m0;

   m1 = r1[2];                 /* now back substitute row 1 */
   s  = 1.0F/r1[1];
   r1[4] = s * (r1[4] - r2[4] * m1);
   r1[5] = s * (r1[5] - r2[5] * m1);
   r1[6] = s * (r1[6] - r2[6] * m1);
   r1[7] = s * (r1[7] - r2[7] * m1);
   m0 = r0[2];
   r0[4] -= r2[4] * m0;
   r0[5] -= r2[5] * m0;
   r0[6] -= r2[6] * m0;
   r0[7] -= r2[7] * m0;

   m0 = r0[1];                 /* now back substitute row 0 */
   s  = 1.0F/r0[0];
   r0[4] = s * (r0[4] - r1[4] * m0);
   r0[5] = s * (r0[5] - r1[5] * m0);
   r0[6] = s * (r0[6] - r1[6] * m0);
   r0[7] = s * (r0[7] - r1[7] * m0);

   MAT(inv,0,0) = r0[4];
   MAT(inv,0,1) = r0[5],
   MAT(inv,0,2) = r0[6];
   MAT(inv,0,3) = r0[7],
   MAT(inv,1,0) = r1[4];
   MAT(inv,1,1) = r1[5],
   MAT(inv,1,2) = r1[6];
   MAT(inv,1,3) = r1[7],
   MAT(inv,2,0) = r2[4];
   MAT(inv,2,1) = r2[5],
   MAT(inv,2,2) = r2[6];
   MAT(inv,2,3) = r2[7],
   MAT(inv,3,0) = r3[4];
   MAT(inv,3,1) = r3[5],
   MAT(inv,3,2) = r3[6];
   MAT(inv,3,3) = r3[7];

   return 1;
}


void MAT4::ScalarMultiply(float s)
{
	for (int i = 0; i < 4; ++i)
		for (int j = 0; j < 4; ++j) {
			M[i][j] = M[i][j] * s;
		}
}

void MAT4::Print()
{
	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 4; ++j) {
			printf("[ %f  ", M[i][j]);
		}
		printf("]\n");
	}
	printf("\n");
}
//Code to implement ease-in/ease-out using sinusoidal ease-in, followed by
//constant velocity and sinusoidal ease - out. returns position as a value
// in range [0,1] where 1 is the max arc length
float EaseInOut(float t, float k1, float k2)
{
	float f = k1 * 2 / M_PI + k2 - k1 + (1.0 - k2) * 2 / M_PI;	//total arc len
	float s;
	if (t < k1) {	// ease in velocity
		s = k1*(2 / M_PI)*(sin((t / k1)*M_PI / 2 - M_PI / 2) + 1);
	}
	else if (t < k2) { // constant velocity
		s = (2 * k1 / M_PI + t - k1);
	}
	else {	// ease out velocity
		s = 2 * k1 / M_PI + k2 - k1 + ((1 - k2)*(2 / M_PI))*sin(((t - k2) / (1.0 - k2))*M_PI / 2);
	}
	return (s / f);
}

// take derivative of EaseInOut function to get velocity. returns velocity
// in range from 0 to 1
float EaseInOutVelocity(float t, float k1, float k2)
{
	if (t < k1)	// starting segment
		return t / k1;
	else if (t < k2)	// middle segment
		return 1;
	else
		return (1-t) / (1-k2) ;	// end segment
}

/*
glm::mat4 MAT4::ToGLM()
{
	mm[0][0] = M[0][0]; mm[0][1] = M[0][1]; mm[0][2] = M[0][2]; mm[0][3] = M[0][3];
	mm[1][0] = M[1][0]; mm[1][1] = M[1][1]; mm[1][2] = M[1][2]; mm[1][3] = M[1][3];
	mm[2][0] = M[2][0]; mm[2][1] = M[2][1]; mm[2][2] = M[2][2]; mm[2][3] = M[2][3];
	mm[3][0] = M[3][0]; mm[3][1] = M[3][1]; mm[3][2] = M[3][2]; mm[3][3] = M[3][3];
	return mm;
}*/