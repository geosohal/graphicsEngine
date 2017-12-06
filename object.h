////////////////////////////////////////////////////////////////////////
// A lightweight class representing an instance of an object that can
// be drawn onscreen, consisting of a shape (batch of triangles), and
// various transformation, color and texture parameters.  
//
// Methods consist of a constructor, and a Draw procedure, and an
// append for building hierarchies of objects.

#ifndef _OBJECT
#define _OBJECT

#include "shapes.h"
#include "texture.h"
#include <utility>              // for pair<Object*,MAT4>

class Shader;
class Object;

// Object:: A shape, and its transformations, colors, and textures and sub-objects.
class Object
{
 public:
    Shape* shape;               // Polygons 
    MAT4 animTr;                // This model's animation transformation
    int objectId;               // Object id to be sent to the shader

    vec3 diffuseColor;          // Diffuse color of object
    vec3 specularColor;         // Specular color of object
    float shininess;            // Surface roughness value

    int textureId;              // Color texture id
    int normalId;               // Normal texture Id
    MAT4 texTr;                 // Texture coordinate transformation

    std::vector<std::pair<Object*,MAT4>> instances; // Pairs of sub-objects and transformations 

    Object(Shape* _shape, const int objectId,
           const vec3 _d=vec3(), const vec3 _s=vec3(), const float _n=1);

    void SetTexture(Texture* _texture, const MAT4 _textr, Texture* _normal=NULL);

    void Draw(ShaderProgram* program, MAT4& objectTr);

    void add(Object* m, MAT4 tr=MAT4()) { instances.push_back(std::make_pair(m,tr)); }
};

#endif
