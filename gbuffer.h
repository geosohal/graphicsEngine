#ifndef GBuffer_H
#define GBuffer_H
//#define BUFFERDEBUG
class GBuffer
{
public:

	enum GBUFFER_TYPE
	{
		GBUFFER_TYPE_NORMAL,
		GBUFFER_TYPE_SPEC,
		GBUFFER_TYPE_DIFFUSE,
		GBUFFER_TYPE_POS,
		GBUFFER_NUMTEXTURES
	};

	GBuffer();
	~GBuffer();
	bool Initialize(int width, int height);
	void DrawBind();
	void ReadBind();
	void SetReadBuffer(GBUFFER_TYPE TextureType);
	void SetDepthReadBuffer();
	void BindDepthForRead(GLenum TU);

	GLuint depthTexture;

private:

	GLuint frameBufferObj;
	GLuint textures[GBUFFER_NUMTEXTURES] = { (GLuint)GBUFFER_TYPE_NORMAL, (GLuint)GBUFFER_TYPE_SPEC, (GLuint)GBUFFER_TYPE_DIFFUSE,
		(GLuint)GBUFFER_TYPE_POS };
	

};

#endif