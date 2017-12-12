
#ifndef _TRANSFORM_
#define _TRANSFORM_

#include <assimp/Importer.hpp>     
#include <glm/glm.hpp>

#include <math.h>

#define ToRadian(x) (float)(((x) * 3.14159265358979323846 / 180.0f))
#define M_PI 3.14159265358979323846264338327950288
using namespace glm;
typedef float ROW4[4];


class MAT4
{
public:
    float M[4][4];
	//glm::mat4 mm;
    // Initilaize all matrices to the identity
    MAT4()
    {
        for( int i=0; i<4; i++ )
            for( int j=0; j<4; j++)
                M[i][j] = i==j ? 1.0 : 0.0;
    }

    // Some indexing operations for the matrix
    ROW4& operator[](const int i)  { return M[i]; }
    const ROW4& operator[](const int i) const { return M[i]; }
	void ScalarMultiply(float s);
	void Print();
	//glm::mat4 ToGLM();

    // Used to communicate to OpenGL
    float* Pntr();
};
MAT4 aiToMAT4(const aiMatrix4x4& aiMatrix);
MAT4 Rotate(const int i, const float theta);
MAT4 Scale(const float x, const float y, const float z);
MAT4 Translate(const float x, const float y, const float z);
MAT4 Translate(vec3 t);
MAT4 Perspective(const float rx, const float ry,
                 const float front, const float back);
MAT4 Perspective2(const float front, const float back, 
	float fov, float w, float h);
MAT4 GetView(vec3 pos, vec3 dir, vec3 up);
MAT4 operator* (const MAT4 A, const MAT4 B);

int invert(const MAT4* mat, MAT4* inv);

//Code to implement ease-in/ease-out using sinusoidal ease-in, followed by
//constant velocity and sinusoidal ease - out. returns position as a value
// in range [0,1] where 1 is the max arc length
float EaseInOut(float t, float k1, float k2);
// take derivative of EaseInOut function to get velocity. returns velocity
// in range from 0 to 1
float EaseInOutVelocity(float t, float k1, float k2);
#endif
