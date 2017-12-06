#include <glbinding/Binding.h>
using namespace gl;

#ifndef AOFBO_H
#define	AOFBO_H
class AoFbo
{
public:
	AoFbo();
	~AoFbo();
	void CreateFBO(const int w, const int h);
	void BindForWrite();
	void BindForRead(GLenum textureUnit);
	void BindBlurredForRead(GLenum textureUnit);
	void UnbindWrite();

	unsigned int aoFbo;
	// modified depth fbo uses color attachment's x component
	GLuint aoTexture;	// ambiient occl texture
	GLuint aoTextureF; // blurred/filtered texture

	int width, height;  // Size of the texture.
};

#endif
