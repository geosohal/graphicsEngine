#include "aoBuffer.h"
#include <glbinding/gl/gl.h>
#include <glu.h>
using namespace gl;
#define CHECKERROR {GLenum err = glGetError(); if (err != GL_NO_ERROR) { fprintf(stderr, "shOpenGL error (at line %d): %s\n", __LINE__, gluErrorString(err)); } }


AoFbo::AoFbo()
{
	aoTexture = aoTextureF = 0;
	aoFbo = 0;
}
AoFbo::~AoFbo()
{
	if (aoFbo != 0)
		glDeleteFramebuffers(1, &aoFbo);
	if (aoTexture != 0)
		glDeleteTextures(1, &aoTexture);
	if (aoTextureF != 0)
		glDeleteTextures(1, &aoTextureF);
}
void AoFbo::CreateFBO(const int w, const int h)
{
	glGenFramebuffers(1, &aoFbo);
	glBindFramebuffer(GL_FRAMEBUFFER, aoFbo);

	// unfiltered ao texture, store AOfactors in R component
	glGenTextures(1, &aoTexture);
	glBindTexture(GL_TEXTURE_2D, aoTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, (int)GL_R32F, w, h, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (int)GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (int)GL_LINEAR);
	// gbuffer leaves off at GL_COLOR_ATTACHMENT3 and it did textures 0-3?
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_TEXTURE_2D, aoTexture, 0);

	glGenTextures(1, &aoTextureF);
	glBindTexture(GL_TEXTURE_2D, aoTextureF);
	glTexImage2D(GL_TEXTURE_2D, 0, (int)GL_R32F, w, h, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (int)GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (int)GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT5, GL_TEXTURE_2D, aoTextureF, 0);

	GLenum DrawBuffers[] = { GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT5 };
	glDrawBuffers(1, DrawBuffers);

	// Check for completeness/correctness
	int status = (int)glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	if (status != int(GL_FRAMEBUFFER_COMPLETE_EXT))
		printf("smFBO Error: %d\n", status);
	// restore default FBO  (so that further changes will not affect our G buffer)
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}
void AoFbo::BindForWrite()
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, aoFbo);
}

void AoFbo::UnbindWrite()
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}
void AoFbo::BindForRead(GLenum textureUnit)
{
	glActiveTexture(textureUnit);
	glBindTexture(GL_TEXTURE_2D, aoTexture);
}
void AoFbo::BindBlurredForRead(GLenum textureUnit)
{
	glActiveTexture(textureUnit);
	glBindTexture(GL_TEXTURE_2D, aoTextureF);
}