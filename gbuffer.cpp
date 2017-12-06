// gbuffer.cpp
// contains 4 textures which are to be used as buffers
// tutorial used as reference: http://ogldev.atspace.co.uk/www/tutorial35/tutorial35.html


#include <glbinding/gl/gl.h>
#include <glbinding/Binding.h>
using namespace gl;

#include "gbuffer.h"
#include <glu.h>                // For gluErrorString
#define CHECKERRORNOX {GLenum err = glGetError(); if (err != GL_NO_ERROR) { fprintf(stderr, "OpenGL gerror (at line %d): %s\n", __LINE__, gluErrorString(err)); } }
//#define BUFFERDEBUG 
GBuffer::GBuffer()
{
	frameBufferObj = 0;
	depthTexture = 0;
//	textures[] = { (GLuint)GBUFFER_TYPE_NORMAL, (GLuint)GBUFFER_TYPE_SPEC, (GLuint)GBUFFER_TYPE_DIFFUSE,
	//	(GLuint)GBUFFER_TYPE_POS };
}

GBuffer::~GBuffer()
{
	if (frameBufferObj != 0) {
		glDeleteFramebuffers(1, &frameBufferObj);
	}
	if (textures[0] != 0) {
		glDeleteTextures(GBUFFER_NUMTEXTURES, textures);
	}
	if (depthTexture != 0) {
		glDeleteTextures(1, &depthTexture);
	}
}
bool GBuffer::Initialize(int WindowWidth, int WindowHeight)
{
	// generate the FBO
	glGenFramebuffers(1, &frameBufferObj);
#ifdef BUFFERDEBUG
	glBindFramebuffer(GL_FRAMEBUFFER, frameBufferObj);
#else
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, frameBufferObj);
#endif
	// generate the gbuffer textures for the vertex attributes
	glGenTextures(GBUFFER_NUMTEXTURES, textures);
	glGenTextures(1, &depthTexture);
	
	for (unsigned int i = 0; i < GBUFFER_NUMTEXTURES; i++) {
		glBindTexture(GL_TEXTURE_2D, textures[i]);

		glTexImage2D(GL_TEXTURE_2D, 0, (int)GL_RGB32F, WindowWidth, WindowHeight, 0, GL_RGB, GL_FLOAT, NULL);
#ifndef BUFFERDEBUG
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (GLfloat)GL_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (GLfloat)GL_NEAREST);
#endif // !BUFFERDEBUG
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, textures[i], 0);
	}


	// initialize depth buffer texture
	glBindTexture(GL_TEXTURE_2D, depthTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, (int)GL_DEPTH_COMPONENT32F, WindowWidth, WindowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT,
		NULL);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);
	
	GLenum DrawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
	glDrawBuffers(GBUFFER_NUMTEXTURES, DrawBuffers);	// enable writing to all 4 buffers

	GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

	if (Status != GL_FRAMEBUFFER_COMPLETE) {
		printf("FB error, status: 0x%x\n", Status);
		return false;
	}

	// restore default FBO  (so that further changes will not affect our G buffer)
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

	return true;
}

void GBuffer::DrawBind()
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, frameBufferObj);
}

void GBuffer::ReadBind()
{
#ifndef BUFFERDEBUG
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	for (unsigned int i = 0; i < GBUFFER_NUMTEXTURES; i++) {
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, textures[GBUFFER_TYPE_NORMAL + i]);

		GLenum err = glGetError();
		if (err != GL_NO_ERROR)
			continue;;

		CHECKERRORNOX
	}
#else
		glBindFramebuffer(GL_READ_FRAMEBUFFER, frameBufferObj);
		CHECKERRORNOX
#endif
}
void GBuffer::SetDepthReadBuffer()
{
	glReadBuffer(gl::GL_DEPTH_ATTACHMENT);
}
void GBuffer::SetReadBuffer(GBUFFER_TYPE TextureType)
{
	glReadBuffer(gl::GL_COLOR_ATTACHMENT0 + TextureType);
	CHECKERRORNOX
}

void GBuffer::BindDepthForRead(GLenum TU)
{
	glActiveTexture(TU);
	glBindTexture(GL_TEXTURE_2D, depthTexture);
	CHECKERRORNOX
}