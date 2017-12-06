///////////////////////////////////////////////////////////////////////
// A slight encapsulation of an OpenGL texture. This contains a method
// to read an image file into a texture, and methods to bind a texture
// to a shader for use, and unbind when done.
////////////////////////////////////////////////////////////////////////

#include "math.h"
#include <fstream>
#include <stdlib.h>
              // For gluErrorString
#include "texture.h"


using namespace gl;

#define STB_IMAGE_IMPLEMENTATION
#define STBI_FAILURE_USERMSG
#include "stb_image.h"
//#define CHECKERRORNO {GLenum err = glGetError(); if (err != GL_NO_ERROR) { fprintf(stderr, "YTOpenGL error (at line %d): %s\n", __LINE__, gluErrorString(err)); } }
Texture::Texture(const std::string &path)
{
	fileName = path;
	textureId = 0;
}



bool Texture::Load()
{
	stbi_set_flip_vertically_on_load(true);
	int width, height, n;
	unsigned char* image = stbi_load(fileName.c_str(), &width, &height, &n, 4);
	if (!image) {
		printf("\nRead error on file %s:\n  %s\n\n", fileName.c_str(), stbi_failure_reason());
		return false;
		//exit(-1);
	}
	
	// Here we create MIPMAP and set some useful modes for the texture
	glGenTextures(1, &textureId);   // Get an integer id for thi texture from OpenGL
	glBindTexture(GL_TEXTURE_2D, textureId);
	
	glTexImage2D(GL_TEXTURE_2D, 0, 4, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 100);
	glGenerateMipmap(GL_TEXTURE_2D);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (int)GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (int)GL_LINEAR_MIPMAP_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
	stbi_image_free(image);
	
	return true;
}

void Texture::Bind(GLenum unit)
{
    glActiveTexture( unit);
    glBindTexture(GL_TEXTURE_2D, textureId);
}

void Texture::Unbind()
{  
    glBindTexture(GL_TEXTURE_2D, 0);
}
