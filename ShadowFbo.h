#include <glbinding/Binding.h>
using namespace gl;

#ifndef SHADOWFBO_H
#define	SHADOWFBO_H
class ShadowFbo
{
public:
	ShadowFbo();
	~ShadowFbo();
	void CreateFBO(const int w, const int h);
	void BindForWrite();
	void BindForRead(GLenum textureUnit, int smStage);
	void BindBlurredForRead(GLenum textureUnit);

	unsigned int shadowFbo;
	// modified depth fbo uses color attachment's x component
	unsigned int sm_Texture;	// shadowmap texture
	GLuint textureNormalized; // normalized and exponentialized shadowmap texture
	GLuint blurredTexture;	// blurred shadowmap texture

	int width, height;  // Size of the texture.
};

#endif
