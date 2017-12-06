////////////////////////////////////////////////////////////////////////
// A small library of object shapes (ground plane, sphere, and the
// famous Utah teapot), each created as a batch of Vertex Buffer
// Objects collected under a Vertex Array Object umbrella.  This is
// the latest and most efficient way to get geometry into the OpenGL
// graphics pipeline.
//
// Each vertex is specified as four attributes which are made
// available in a vertes shader in the following attribute slots.
//
// position,        vec4,   attribute #0
// normal,          vec3,   attribute #1
// texture coord,   vec3,   attribute #2
// tangent,         vec3,   attribute #3
//
// An instance of any of these shapes is create with a single call:
//    unsigned int obj = CreateSphere(divisions, &quadCount);
// and drawn by:
//    glBindVertexArray(obj);
//    glDrawElements(GL_QUADS, quadCount, GL_UNSIGNED_INT, 0);
//    glBindVertexArray(0);
//
// Copyright 2013 DigiPen Institute of Technology
////////////////////////////////////////////////////////////////////////

#ifndef _SHAPES
#define _SHAPES

#include "transform.h"
#include "rply.h"

#include <vector>

class Shape
{
public:

    Shape() :animate(false) {}
    virtual ~Shape() {}

    // Data arrays
    std::vector<vec4> Pnt;
    std::vector<vec3> Nrm;
    std::vector<vec2> Tex;
    std::vector<vec3> Tan;

    // Lighting information
    vec3 diffuseColor, specularColor;
    float shininess;

    // Geometry defined by indices into data arrays
    std::vector<ivec4> Quad;
    std::vector<ivec3> Tri;
    unsigned int count;
    unsigned int shape;

    // Defined by SetTransform by scanning data arrays
    vec3 minP, maxP;
    vec3 center;
    float size;
    MAT4 modelTr;
    bool animate;

    // Defined by MakeVAO when/if sending to OpenGL
    unsigned int vao;

    virtual void ComputeSize();
    virtual void MakeVAO();
    virtual void DrawVAO();
};

class Box: public Shape
{
  void face(const mat4x4 tr);
public:
    Box();
};

class Sphere: public Shape
{
public:
    Sphere(const int n);
};

class Disk: public Shape
{
public:
    Disk(const int n);
};

class Cylinder: public Shape
{
public:
    Cylinder(const int n);
};

class Torus: public Shape
{
public:
    Torus(const float r1, const float r2, const int n);
};

class Teapot: public Shape
{
public:
    Teapot(const int n);
};

class Plane: public Shape
{
public:
    Plane(const float range, const int n);
};

class ProceduralGround: public Shape
{
public:
    float range;
    float octaves;
    float persistence;
    float scale;
    float low;
    float high;
    float xoff;
    vec3 highPoint;

    ProceduralGround(const float _range, const int n,
                     const float _octaves, const float _persistence, const float _scale,
                     const float _low, const float _high, const bool randomize);
    float HeightAt(const float x, const float y);
};

class Quad: public Shape
{
public:
    Quad(const int n=1);
};

class Ply: public Shape
{
public:
    Ply(const char* name, const bool reverse=false);
    virtual ~Ply() {printf("destruct Ply\n");};
    static int vertex_cb(p_ply_argument argument);
    static int normal_cb(p_ply_argument argument);
    static int texture_cb(p_ply_argument argument);
    static int face_cb(p_ply_argument argument);
};

#endif
