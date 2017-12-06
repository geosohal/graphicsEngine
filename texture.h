///////////////////////////////////////////////////////////////////////
// A slight encapsulation of an OpenGL texture. This contains a method
// to read an image file into a texture, and methods to bind a texture
// to a shader for use, and unbind when done.
////////////////////////////////////////////////////////////////////////

#ifndef _TEXTURE_
#define _TEXTURE_


#include <glbinding/gl/gl.h>
#include <glbinding/Binding.h>
class Texture
{
 public:
    unsigned int textureId;
	std::string fileName;
    
    Texture(const std::string &filename);
	Texture(const unsigned int id, const std::string &filename);
    void Bind(gl::GLenum unit);
	bool Load();
    void Unbind();
};

#endif
