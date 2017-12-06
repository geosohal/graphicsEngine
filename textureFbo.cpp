//#include "textureFbo.h"
//#include <glbinding/gl/gl.h>
//#include <glu.h>
//using namespace gl;
//#define CHECKERROR {GLenum err = glGetError(); if (err != GL_NO_ERROR) { fprintf(stderr, "shOpenGL error (at line %d): %s\n", __LINE__, gluErrorString(err)); } }
//TextureFbo::TextureFbo()
//{
//	textureId = 0;
//	textureFbo = 0;
//}
//
//TextureFbo::~TextureFbo()
//{
//	if (textureFbo != 0)
//		glDeleteFramebuffers(1, &textureFbo);
//	if (textureId != 0)
//		glDeleteTextures(1, &textureId);
//}
//
//
//void TextureFbo::CreateFBO(const int w, const int h)
//{
//	glGenFramebuffers(1, &textureFbo); //create shadowmap fbo
//	glBindFramebuffer(GL_FRAMEBUFFER, textureFbo);	// set shadowmap fbo to current fbo
//
//
//													// normalized depth buffer texture, store depth in R component
//	glGenTextures(1, &textureId);
//	glBindTexture(GL_TEXTURE_2D, textureId);
//	glTexImage2D(GL_TEXTURE_2D, 0, (int)GL_RGB32F, w, h, 0, GL_RGB, GL_FLOAT, NULL);
//	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (int)GL_LINEAR);
//	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (int)GL_LINEAR);
//	// attach phase 2 shadow map texture to color attachment point
//	// gbuffer leaves off at GL_COLOR_ATTACHMENT3 and it did textures 0-3
//	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_TEXTURE_2D, textureNormalized, 0);
//
//	// create  depth buffer texture
//	glGenTextures(1, &sm_Texture);
//	glBindTexture(GL_TEXTURE_2D, sm_Texture);	// todo: GL_DEPTH_COMPONENT32F
//	glTexImage2D(GL_TEXTURE_2D, 0, (int)GL_DEPTH_COMPONENT, w, h, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
//	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, (int)GL_CLAMP_TO_EDGE);
//	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, (int)GL_CLAMP_TO_EDGE);
//	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (int)GL_LINEAR);
//	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (int)GL_LINEAR);
//	// attach shadow map texture to dfepth attachment point of the fbo
//	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, sm_Texture, 0);
//
//
//
//
//
//
//	/////////////////////////////
//	// generate textyre for blurred shadowmap
//	glGenTextures(1, &blurredTexture);
//	glBindTexture(GL_TEXTURE_2D, blurredTexture);
//	glTexImage2D(GL_TEXTURE_2D, 0, (int)GL_R32F, w, h, 0, GL_RGB, GL_FLOAT, NULL);
//	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (int)GL_LINEAR);
//	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (int)GL_LINEAR);
//	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT5, GL_TEXTURE_2D, blurredTexture, 0);
//
//
//	GLenum DrawBuffers[] = { GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT5 };
//	glDrawBuffers(1, DrawBuffers);
//	//glReadBuffer(GL_NONE);
//
//	// Check for completeness/correctness
//	int status = (int)glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
//	if (status != int(GL_FRAMEBUFFER_COMPLETE_EXT))
//		printf("smFBO Error: %d\n", status);
//	// restore default FBO  (so that further changes will not affect our G buffer)
//	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
//}
//
//void TextureFbo::BindForWrite()
//{
//	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, TextureFbo);
//	CHECKERROR
//}
//
//void TextureFbo::BindForRead(GLenum textureUnit, int smStage)
//{
//	glActiveTexture(textureUnit);
//	if (smStage == 1)
//		glBindTexture(GL_TEXTURE_2D, sm_Texture);
//	else if (smStage == 2)
//		glBindTexture(GL_TEXTURE_2D, textureNormalized);
//	else if (smStage == 3)
//		glBindTexture(GL_TEXTURE_2D, blurredTexture);
//	CHECKERROR
//}
//
//void TextureFbo::BindBlurredForRead(GLenum textureUnit)
//{
//	glActiveTexture(textureUnit);
//	glBindTexture(GL_TEXTURE_2D, blurredTexture);
//	CHECKERROR
//}