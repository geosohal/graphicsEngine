#include <glbinding/Binding.h>
using namespace gl;

// originally made so that i could have a fbo wrapper for reading pixels from my hdr texture
#ifndef TEXTUREFBO_H
#define	TEXTUREFBO_H
class TextureFbo
{
public:
	TextureFbo();
	~TextureFbo();
	void CreateFBO(const int w, const int h); /* takes w and h of texture */
	void BindForWrite();
	void BindForRead(GLenum textureUnit);

	unsigned int textureFbo;

	unsigned int textureId;	// shadowmap texture

	int width, height;  // Size of the texture.
};

#endif