//////////////////////////////////////////////////////////////////////
// Defines and draws a scene.  There are two main procedures here:
//
// Copyright 2013 DigiPen Institute of Technology
////////////////////////////////////////////////////////////////////////

#include "math.h"
#include <fstream>
#include <stdlib.h>

#include <glbinding/gl/gl.h>
#include <glbinding/Binding.h>
using namespace gl;

#include <freeglut.h>
#include <glu.h>                // For gluErrorString

#define GLM_FORCE_RADIANS
#define GLM_SWIZZLE

#include <glm/glm.hpp>
using namespace glm;

#include "shader.h"
#include "shapes.h"
#include "scene.h"
#include "object.h"
#include "texture.h"
#include "transform.h"
#include "gbuffer.h"
#include <glm/gtc/matrix_transform.hpp>
#include "ShadowFbo.h"
#include "SphericalHarmonics.h"

const float rad = PI / 180.0f;    // Convert degrees to radians

MAT4 Identity;

const float grndSize = 100.0;    // Island radius;  Minimum about 20;  Maximum 1000 or so
const int   grndTiles = int(grndSize);
const float grndOctaves = 4.0;  // Number of levels of detail to compute
const float grndFreq = 0.03;    // Number of hills per (approx) 50m
const float grndPersistence = 0.03; // Terrain roughness: Slight:0.01  rough:0.05
const float grndLow = -3.0;         // Lowest extent below sea level
const float grndHigh = 5.0;        // Highest extent above sea level

								   // Simple procedure to print a 4x4 matrix -- useful for debugging
void PrintMat(const MAT4& m)
{
	for (int i = 0; i<4; i++)
		for (int j = 0; j<4; j++)
			printf("%9.4f%c", m[i][j], j == 3 ? '\n' : ' ');
	printf("\n");
}

////////////////////////////////////////////////////////////////////////
// This macro makes it easy to sprinkle checks for OpenGL errors
// through out your code.  Most OpenGL calls can record errors, and a
// careful programmer will check the error status *often*, perhaps as
// often as right after every OpenGL call.  At the very least, once
// per refresh will tell you if something is going wrong.
#define CHECKERROR {GLenum err = glGetError(); if (err != GL_NO_ERROR) { fprintf(stderr, "ceOpenGL error (at line %d): %s\n", __LINE__, gluErrorString(err)); exit(-1);} }
#define CHECKERRORNOX {GLenum err = glGetError(); if (err != GL_NO_ERROR) { fprintf(stderr, "ceOpenGL error (at line %d): %s\n", __LINE__, gluErrorString(err)); } }
//#define BUFFERDEBUG	// also in gbuffer.cpp
//#define PROJECT1 // anim project 1
//#define SHADOWS
#define DEFERREDSHADING

vec3 HSV2RGB(const float h, const float s, const float v)
{
	if (s == 0.0)
		return vec3(v, v, v);

	int i = (int)(h*6.0);
	float f = (h*6.0f) - i;
	float p = v*(1.0f - s);
	float q = v*(1.0f - s*f);
	float t = v*(1.0f - s*(1.0f - f));
	if (i % 6 == 0)     return vec3(v, t, p);
	else if (i == 1)  return vec3(q, v, p);
	else if (i == 2)  return vec3(p, v, t);
	else if (i == 3)  return vec3(p, q, v);
	else if (i == 4)  return vec3(t, p, v);
	else if (i == 5)  return vec3(v, p, q);
}



////////////////////////////////////////////////////////////////////////
// Called regularly to update the atime global variable.
float atime = 0.0;

void animate(int value)
{
	atime = 360.0*glutGet((GLenum)GLUT_ELAPSED_TIME) / 36000;
	//printf("..%f..", atime);
	glutPostRedisplay();

	// Schedule next call to this function
	glutTimerFunc(30, animate, 1);
}

////////////////////////////////////////////////////////////////////////
// InitializeScene is called once during setup to create all the
// textures, shape VAOs, and shader programs as well as a number of
// other parameters.
void Scene::InitializeScene()
{

	//	glEnable(GL_DEPTH_TEST);
	CHECKERROR;
	Identity = MAT4();
	// FIXME: This is a good place for initializing the transformation
	// values.
	last_time = 0;
#ifdef SOLUTION
	mode = 9;
	key = 0;
	nav = true;
	spin = 179.63;
	tilt = 4.9259;
	eye = vec3(-0.26f, 66.8f, 3.7f);
	speed = .005f;
	last_time = glutGet((GLenum)GLUT_ELAPSED_TIME);
	tr = vec3(0.0, 0.0, 25.0);

	ry = 0.4;
	front = 0.3;
	back = 400.f;
#endif

	//	objectRoot = new Object(NULL, nullId);

	// Set the initial light position parammeters
	lightSpin = 90.0;
	lightTilt = -30.0;
	lightDist = 1000.0;

	// Enable OpenGL depth-testing
	glEnable(GL_DEPTH_TEST);



#ifndef PROJECT1
	for (int i = 0; i < numPointLights; i++)
	{
		float percent = (float)i / numPointLights;
		pointLights[i].pos = vec3(0, 0, -2.f);
		pointLights[i].diffuseIntensity = 0.6f;
		pointLights[i].color = vec3(((float)(rand() % 95)) / 60.f, ((float)(rand() % 90)) / 90.f, ((float)(rand() % 99)) / 10.f);
		pointLights[i].atten.constant = 0.0f;
		pointLights[i].atten.linear = 0.0f;
		pointLights[i].atten.exp = 0.9f;
		degreeIncCounter[i] = percent * 2 * PI;	// starting angle in carousel
	}
#endif


	dirLight.ambientIntensity = 0.4f;
	dirLight.color = vec3(1.0f, 1.0f, 1.0f);
	dirLight.diffuseIntensity = 0.5f;
	dirLight.direction = vec3(1.0f, 0.0f, 0.0f);

	spotLight.ambientIntensity = 0.0f;
	spotLight.diffuseIntensity = 0.90f;
	spotLight.color = vec3(.5f, .5f, 1.f);
	spotLight.pos = vec3(0.f, 0, 1.554f);
	spotLight.direction = vec3(1.f, 1.f, -1);
	spotLight.cutoff = 90.f;
	spotLight.atten.linear = .01f;
	spotLight.atten.constant = 0;
	spotLight.atten.exp =0;


#ifdef DEFERREDSHADING
		if (!gbuffer.Initialize(592, 592)) {
			printf("\nGbufferFailed to initialize");;
		}
#endif
#ifndef BUFFERDEBUG
	#ifdef DEFERREDSHADING
		CHECKERRORNOX
			pointlightShader = new PLightShaderProgram();
		pointlightShader->Initialize();
		pointlightShader->Use();
		CHECKERRORNOX
			pointlightShader->SetPosTextureUnit(GBuffer::GBUFFER_TYPE_POS);
		CHECKERRORNOX
			pointlightShader->SetColorTextureUnit(GBuffer::GBUFFER_TYPE_DIFFUSE);
		CHECKERRORNOX
			pointlightShader->SetNormalTextureUnit(GBuffer::GBUFFER_TYPE_NORMAL);
		CHECKERRORNOX
			pointlightShader->SetScreenSize(592, 592);
		CHECKERRORNOX

			dirLightShader = new DirLightProgram();
		CHECKERRORNOX
			dirLightShader->Initialize();
		dirLightShader->Use();
		CHECKERRORNOX
			dirLightShader->SetPosTextureUnit(GBuffer::GBUFFER_TYPE_POS);
		CHECKERRORNOX
			dirLightShader->SetColorTextureUnit(GBuffer::GBUFFER_TYPE_DIFFUSE);
		dirLightShader->SetNormalTextureUnit(GBuffer::GBUFFER_TYPE_NORMAL);
		dirLightShader->SetScreenSize(592, 592);
		dirLightShader->SetDirLight(dirLight);
		CHECKERRORNOX
#endif
#endif

		kernalSize = 47;
		InitializePascalsTri(91);
		vector<float> weights;
		weights.reserve(kernalSize);
		float weightSum = 0;
		for (int i = 0; i < kernalSize; i++)
		{
			weightSum += pascalsTriangle[kernalSize - 1][i];
			if (i >50)
				weights.push_back(0);
			else
				weights.push_back(pascalsTriangle[kernalSize - 1][i]);
		}
		for (int i = 0; i < weights.size(); i++)
		{
			weights[i] = weights[i] ;
		}


		aoFbo.CreateFBO(592, 592);

		aoProgram = new ShaderProgram();
		aoProgram->AddShader("ambientOcclusion.vs", GL_VERTEX_SHADER);
		aoProgram->AddShader("ambientOcclusion.fs", GL_FRAGMENT_SHADER);
		aoProgram->LinkProgram();

		bilateralBlur = new ShaderProgram();
		CHECKERRORNOX
			bilateralBlur->AddShader("bilateralBlur.comp", GL_COMPUTE_SHADER);
		bilateralBlur->LinkProgram();
		bilateralBlur->Use();
		int programId = bilateralBlur->programId;
		GLuint blockID = 0;
		glGenBuffers(1, &blockID); // Generates block for weights array
		int bindpoint = 0; // Start at zero, increment for other blocks
		int loc = glGetUniformBlockIndex(programId, "blurKernel");
		glUniformBlockBinding(programId, loc, bindpoint);
		glBindBufferBase(GL_UNIFORM_BUFFER, bindpoint, blockID);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(weights[0]) * weights.size(), &weights[0], GL_STATIC_DRAW);
		CHECKERRORNOX
			glUseProgram(0);

		bilateralBlurV = new ShaderProgram();
		CHECKERRORNOX
			bilateralBlurV->AddShader("bilateralBlurV.comp", GL_COMPUTE_SHADER);
		bilateralBlurV->LinkProgram();
		bilateralBlurV->Use();
		programId = bilateralBlurV->programId;
		//blockID = 0;
		glGenBuffers(1, &blockID); // Generates block for weights array
		bindpoint++; // Start at zero, increment for other blocks
		loc = glGetUniformBlockIndex(programId, "blurKernel");
		glUniformBlockBinding(programId, loc, bindpoint);
		glBindBufferBase(GL_UNIFORM_BUFFER, bindpoint, blockID);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(weights[0]) * weights.size(), &weights[0], GL_STATIC_DRAW);
#ifdef SHADOWS



		// shadow fbo will create the shadowmap framebuffer with the texture unit at 1 as
			// specified above the spotlight shader
			shadowFbo.CreateFBO(592, 592);
		shadowShader = new ShadowProgram();
		shadowShader->Initialize();
		blurShader = new ShaderProgram();
		CHECKERRORNOX
			blurShader->AddShader("blurPass.comp", GL_COMPUTE_SHADER);
		blurShader->LinkProgram();
		blurShader->Use();


		CHECKERRORNOX
			int programId = blurShader->programId;
		GLuint blockID = 0;
		glGenBuffers(1, &blockID); // Generates block for weights array
		int bindpoint = 0; // Start at zero, increment for other blocks
		int loc = glGetUniformBlockIndex(programId, "blurKernel");
		CHECKERRORNOX
			glUniformBlockBinding(programId, loc, bindpoint);
		CHECKERRORNOX


		glBindBufferBase(GL_UNIFORM_BUFFER, bindpoint, blockID);
		CHECKERRORNOX
			glBufferData(GL_UNIFORM_BUFFER, sizeof(weights[0]) * weights.size(), &weights[0], GL_STATIC_DRAW);
		CHECKERRORNOX
			glUseProgram(0);

			blurShaderV = new ShaderProgram();
		CHECKERRORNOX
			blurShaderV->AddShader("blurPassV.comp", GL_COMPUTE_SHADER);
		blurShaderV->LinkProgram();
		blurShaderV->Use();
		programId = blurShaderV->programId;
		//blockID ++;
		glGenBuffers(1, &blockID); // Generates block for weights array
		bindpoint++; // Start at zero, increment for other blocks
		 loc = glGetUniformBlockIndex(programId, "blurKernel");
		CHECKERRORNOX
			glUniformBlockBinding(programId, loc, bindpoint);

		glBindBufferBase(GL_UNIFORM_BUFFER, bindpoint, blockID);
		CHECKERRORNOX
			glBufferData(GL_UNIFORM_BUFFER, sizeof(weights[0]) * weights.size(), &weights[0], GL_STATIC_DRAW);
		CHECKERRORNOX



			spLightShader = new SpotLightProgram();
		spLightShader->Initialize();
		spLightShader->Use();
		spLightShader->SetSpotLight(spotLight);
		CHECKERRORNOX
			// gSampler, this is the texture unit which belongs to the mesh.
			spLightShader->SetTextUnit(0);
		// gShadowMap- texture unit that belongs to shadow map is the next one after the gbuffer textures
		spLightShader->SetShadowMapTextUnit(1);
#endif

	CHECKERRORNOX

		lineShader = new ShaderProgram();
	lineShader->AddShader("line.vs", GL_VERTEX_SHADER);
	lineShader->AddShader("line.fs", GL_FRAGMENT_SHADER);
	lineShader->LinkProgram();

#ifdef PROJECT1

	triangleShader = new ShaderProgram();
	triangleShader->AddShader("triangles.vs", GL_VERTEX_SHADER);
	triangleShader->AddShader("triangles.fs", GL_FRAGMENT_SHADER);
	triangleShader->LinkProgram();
#endif
	// Create the lighting shader program from source code files.
	lightingProgram = new ShaderProgram();

	lightingProgram->AddShader("geometry_pass.vs", GL_VERTEX_SHADER);
	lightingProgram->AddShader("geometry_pass.fs", GL_FRAGMENT_SHADER);
	CHECKERRORNOX
		lightingProgram->LinkProgram();




	iblShader = new IBLProgram();
	iblShader->Initialize();
	iblShader->Use();
	//iblShader->SetPosTU(GBuffer::GBUFFER_TYPE_POS);
	iblShader->SetNormalTU(GBuffer::GBUFFER_TYPE_NORMAL);

	CHECKERRORNOX
		//glUniform1i(loc , 0);
		CHECKERRORNOX

	sphere = new BasicMesh();
	CHECKERRORNOX
		if (!sphere->LoadMesh("content/msphere3.obj")) {
			printf("Problem loading mesh");
		}
	testMesh = new BasicMesh();
	CHECKERRORNOX
		if (!testMesh->LoadMesh("content/Dragon.obj")) {
			printf("Problem loading mesh");
		}

	spider = new BasicMesh();
	if (!spider->LoadMesh("content/sphere2.obj")) { printf("meshload error");
	}
	quad = new BasicMesh();
	if (!quad->LoadMesh("content/quad.obj")) {
		printf("Problem loading mesh");
	}

	CHECKERROR;
#ifndef PROJECT1

	box = new BasicMesh();
	CHECKERRORNOX
		if (!box->LoadMesh("content/box.obj")) {
			printf("Problem loading mesh");
		}
	smQuad = new BasicMesh();
	if (!smQuad->LoadMesh("content/quad.obj")) {
		printf("Problem loading mesh");
	}
	CHECKERROR;

	envMap = new Texture("content/Alexs_Apt_2k.hdr");

	if (!envMap->Load()) {
		CHECKERRORNOX
		printf("Error loading texture env");
		delete envMap;
		envMap = NULL;
	}
	else {
		CHECKERRORNOX
		printf("Loaded texture global env \n" );
	}

	irradMap = new Texture("content/Alexs_Apt_2k.irr.hdr");
	if (!irradMap->Load()) {
		CHECKERRORNOX
			printf("Error loading irrad env");
		delete irradMap;
		irradMap = NULL;
	}
	else {
		CHECKERRORNOX
			printf("Loaded texture global irrad \n");
	}
#endif
	randomness = 1;
#ifdef PROJECT1

	skinningShader = new SkinProgram();
	skinningShader->Initialize();
	skinningShader->Use();
	skinningShader->SetColorTextureUnit(COLOR_TEXTURE_UNIT_INDEX);

	skeleton = new AnimatedModel();
	skeleton->LoadMesh("content/gh_sample_animation.fbx");

	walkPath.InitializeControlPoints(-3.93f, .04f, vec2(-30,-30));

#endif // PROJECT1

	// determine spherical harmonic coefficients from the environment map

	

	//SphericalHarmonics sh;
	//sh.Initialize();

	//SphericalHarmonics sh;
	//sh.InitializeSH();








	physics.InitializeSim(1.0f, PhysicsSim::BODYTYPE_BLOCK, 2.f, .5f, .5f, vec3(-13.5f, 0, 0.f));

	// Schedule first timed animation call
	glutTimerFunc(30, animate, 1);
}

void Scene::MoveAnchor(bool isLeft, int mouseX, int mouseY)
{
	
}
bool slowMo = false;
bool firstKeyDown = false;
bool drawSkin = false;
int nbFrames = 0;
float lastTimeSincePrint;
bool fastCam = true;
////////////////////////////////////////////////////////////////////////
// Procedure DrawScene is called whenever the scene needs to be drawn.
void Scene::DrawScene()
{
#ifdef SOLUTION
	float now = glutGet((GLenum)GLUT_ELAPSED_TIME);

	float timeSinceUpdate = (now - last_time);
	float dist = timeSinceUpdate*speed;
	nbFrames++;
	if (now - lastTimeSincePrint >= 1000) { // If last prinf() was more than 1 sec ago
											// printf and reset timer
											//	time_t tnow = time(NULL);
											//	printf("at %s:", ctime(&tnow));
		//printf("%f dist\n",);
		nbFrames = 0;
		lastTimeSincePrint += now - lastTimeSincePrint;
	}


	float camDist = (fastCam) ? dist * 4.f : dist / 4.f;
	
	if (key == 'w')
	{
		physics.anchorRight.z += dist*4.8f;
	}
	if (key == 's')
		physics.anchorRight.z -= dist*4.8f;
	if (key == 'd')
		physics.anchorRight.x -= dist*4.8f;
	if (key == 'a')
		physics.anchorRight.x += dist*4.8f;
	if (key == 'p')
		fastCam = !fastCam;
	if (key == 'g')
		physics.anchorLeft.z -= dist*4.8f;

	if (key == 't')
		physics.anchorLeft.z += dist*4.8f;

	if (key == 'f')
		physics.anchorLeft.x += dist*4.8f;

	if (key == 'h')
		physics.anchorLeft.x -= dist*4.8f;
	if (key == 'y')
		physics.anchorLeft.y += dist*4.8f;;
	if (key == 'r')
		physics.anchorLeft.y -= dist*4.8f;;
#ifdef PROJECT1
	if (key == 'z' && firstKeyDown)
		slowMo = !slowMo;
	if (key == 'm' && firstKeyDown)
		drawSkin = !drawSkin;


	if (key == 'c' && firstKeyDown)
		skeleton->SwitchAnimation();
#endif

		
	if (key != 0)//when we update next frame key is reported to already be pressed
		firstKeyDown = false;
	else// key was just pressed
		firstKeyDown = true;


	CHECKERROR;
#endif

	// Calculate the light's position.
	float lPos[4] = {
		basePoint.x + lightDist*cos(lightSpin*rad)*sin(lightTilt*rad),
		basePoint.y + lightDist*sin(lightSpin*rad)*sin(lightTilt*rad),
		basePoint.z + lightDist*cos(lightTilt*rad),
		1.0 };

	// Set the viewport, and clear the screen
	glViewport(0, 0, width, height);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Compute Viewing and Perspective transformations.
	MAT4 WorldProj, WorldView, WorldInverse;

	MAT4 lightProj = Perspective2(front, back, 60.f, 592.f, 592.f);
	WorldProj = Perspective((ry*width) / height, ry, front, back);
	if (nav)
		WorldView = Rotate(0, tilt - 90)*Rotate(2, spin) *Translate(-eye[0], -eye[1], -eye[2]);
	else
		WorldView = Translate(tr[0], tr[1], -tr[2]) *Rotate(0, tilt - 90)*Rotate(2, spin);


	int programId;

	// Setup the perspective and viewing matrices for normal viewing.


	MAT4 VPmatrix = WorldProj*WorldView;

	CHECKERROR;
	MAT4 LightView = Rotate(1, 180) *Translate(-spotLight.pos.x, -spotLight.pos.y, -spotLight.pos.z);
	LightView = Rotate(0, 0)*Translate(-spotLight.pos[0], -spotLight.pos[1], -spotLight.pos[2]);
	//GetView(spotLight.pos, spotLight.direction, vec3(0, 0,1.f));
	MAT4 lightVP = WorldProj * LightView;
	MAT4 WorldVP = WorldProj * WorldView;
	MAT4 s = Scale(.05f, .05f, .05f);
	MAT4 r = Quaternion(-.707, vec3(.707, 0, 0)).ToMat4();
	MAT4 t = Translate(0.f, 16.8f, 0.0);
#ifdef SHADOWS


	//////////////////////
	// shadow map pass

	//glCullFace(GL_FRONT);
	shadowFbo.BindForWrite();	// from now on all depth values goto shadowFBO.sm texture
								// color writes will goto shadowFBO.textureNormalized
								// which is the normalized version of shadow depth

	shadowShader->Use();
	programId = shadowShader->programId;
	//shadowmaps fragment shader uses stored depths to normalize and exponentialize depths
	shadowShader->SetTextureUnit(shadowFbo.sm_Texture);
	//glClear(GL_DEPTH_BUFFER_BIT);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// limit depth test to the geometry pass
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND); // the light passes use blending, dont need it here

	s = Scale(.05f, .05f, .05f);


	glBindFramebuffer(GL_FRAMEBUFFER, 0); //switch back to default framebuffer by binding FBO 0
										// now fragment shader of shadowmap can be used in addition
										// to spotlight's
	
	////////////////
	// blur the shadow/depth map with compute shader horizontal pass
	blurShader->Use();
	CHECKERRORNOX
		programId = blurShader->programId;
	// set all uniform and image variables

		int imageUnit = 0; // Increment for other images
	int smTextureID = shadowFbo.textureNormalized;
	int loc = glGetUniformLocation(programId, "kernalSize");
	glUniform1i(loc, kernalSize);
	loc = glGetUniformLocation(programId, "src");
	CHECKERRORNOX
		glBindImageTexture(imageUnit, smTextureID, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);
	CHECKERRORNOX
		glUniform1i(loc, imageUnit);
	CHECKERRORNOX
		imageUnit++;
	loc = glGetUniformLocation(programId, "dst");
	int blurredTexture = shadowFbo.blurredTexture;
	glBindImageTexture(imageUnit, blurredTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);
	glUniform1i(loc, imageUnit);
	CHECKERRORNOX
		glDispatchCompute(width / 148, height, 1); // Tiles WxH image with groups sized 128x1
	glMemoryBarrier(GL_ALL_BARRIER_BITS);
	////////////////
	// vertical pass
	blurShaderV->Use();
	CHECKERRORNOX
		programId = blurShader->programId;
	// set all uniform and image variables	imageUnit++; // Increment for other images
	smTextureID = shadowFbo.blurredTexture;	// input texture is horizontally blurrded texture
	loc = glGetUniformLocation(programId, "kernalSize");
	glUniform1i(loc, kernalSize);
	loc = glGetUniformLocation(programId, "src");
	CHECKERRORNOX
		glBindImageTexture(imageUnit, smTextureID, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);
	CHECKERRORNOX
		glUniform1i(loc, imageUnit);
	CHECKERRORNOX
		imageUnit++;
	loc = glGetUniformLocation(programId, "dst");
	blurredTexture = shadowFbo.textureNormalized;	// we use the texture that was originally the normalized
														// texture as the output this time.
	glBindImageTexture(imageUnit, blurredTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);
	glUniform1i(loc, imageUnit);
	CHECKERRORNOX
		glMemoryBarrier(GL_ALL_BARRIER_BITS);
		glDispatchCompute(width , height/148, 1); // Tiles WxH image with groups sized 1x148
		
		
	
	glUseProgram(0);
#endif
#ifdef DEFERREDSHADING
		
#endif
#ifdef DEFERREDSHADING
		// from DS geometry pass
		gbuffer.DrawBind();		// all depth and color writes now go to gbuffer's textures
	#ifndef BUFFERDEBUG
		//prevent anything but this pass from writing into the depth buffer
		// as the light pass doesnt have anythign to write into it
		glDepthMask(GL_TRUE);
	#endif
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	#ifndef BUFFERDEBUG
		// limit depth test to the geometry pass
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_BLEND); // the light passes use blending, dont need it here
	#endif
#endif
	
#ifdef SHADOWS

	
	/////////////////////
	// shadow map specific pieces render pass
		//glCullFace(GL_BACK);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	spLightShader->Use();
	spLightShader->SetSpotLight(spotLight);
 	programId = spLightShader->programId;
	spLightShader->SetEyeWorldPos(eye.x, eye.y, eye.z);



	// tell lighting shader what texture to use as shadowmap
	spLightShader->SetShadowMapTextUnit(shadowFbo.textureNormalized);
	// use e^cd depth texture on the given texture unit
	shadowFbo.BindForRead(GL_TEXTURE0 + shadowFbo.textureNormalized,2);
										
	CHECKERROR
#endif
#ifndef PROJECT1
// Use the lighting shader
	lightingProgram->Use();
	lightingProgram->SetUniformi("depthMap", gbuffer.depthTexture);
	programId = lightingProgram->programId;
	int loc;
	loc = glGetUniformLocation(programId, "gWVP"); // todog actually gVP
	glUniformMatrix4fv(loc, 1, GL_TRUE, VPmatrix.Pntr());

	s = Scale(108.05f, 108.05f, 108.05f);
	// draw big sphere
	t = Translate(0, 0, -1.5f)* Rotate(2, -180.f) * s;// ;
	loc = glGetUniformLocation(lightingProgram->programId, "gWorld");
	glUniformMatrix4fv(loc, 1, GL_TRUE, t.Pntr());
	sphere->Render();
	
	s = Scale(0.30f, 0.3f, 0.3f);



	/*
	float spacing = 1.5f;	// draw spheres
	for (int i = 0; i < 1; i++)
	{
		for (int j = 0; j < 0; j++)
		{
			t = Translate(-10.f+ spacing*i, -10.f+ spacing*j, 0)* s;
			loc = glGetUniformLocation(lightingProgram->programId, "gWorld");
			glUniformMatrix4fv(loc, 1, GL_TRUE, t.Pntr());
			sphere->Render();
		}
	}
*/
	// draw dragon
	t = Translate(0, 0, 2.f)*Rotate(0,90.f)* s;
	loc = glGetUniformLocation(lightingProgram->programId, "gWorld");
	glUniformMatrix4fv(loc, 1, GL_TRUE, t.Pntr());
	//testMesh->Render();
	vector<vec3> linepoints;
	linepoints.push_back(physics.anchorLeft);
	lightingProgram->SetUniform4v("gWorld", 
		Translate(linepoints[0].x, linepoints[0].y, linepoints[0].z)*Rotate(0, 90.f));
	testMesh->Render();
	physics.UpdateSim(now, timeSinceUpdate/1000.f);
	for (int i = 0; i < NBODIES; i++)
	{
		linepoints.push_back(physics.Bodies[i].lEndPt);
		linepoints.push_back(physics.Bodies[i].rEndPt);
		t = Translate(physics.Bodies[i].x) * glTOMAT4( glm::toMat4(physics.ReflectedQuaternion(i)) ) * Scale(2.f, .5f, .5f);// *physics.Bodies[i].q;
		lightingProgram->SetUniform4v("gWorld", t);
		if (i == 1)
		{
			lightingProgram->SetUniformi("test", 5);
			box->Render();
			lightingProgram->SetUniformi("test", 0);
		}
		else
			box->Render();
	}
	linepoints.push_back(physics.anchorRight);
	lightingProgram->SetUniform4v("gWorld", Translate(physics.anchorRight.x, physics.anchorRight.y, physics.anchorRight.z));
	spider->Render();

	lineShader->Use();
	lineShader->SetUniform4v("PV", VPmatrix);
	for (int i = 0; i < linepoints.size(); i += 2)
	{
		gl::glBegin(gl::GL_LINES);
		gl::glVertex3f(linepoints[i].x, linepoints[i].y, linepoints[i].z);
		gl::glVertex3f(linepoints[i+1].x, linepoints[i+1].y, linepoints[i+1].z);
		gl::glEnd();

	}
	lineShader->Unuse();

	//aoFbo.BindForRead(GL_TEXTURE0 + aoFbo.aoTextureF);

#endif

#ifdef DEFERREDSHADING
	#ifndef BUFFERDEBUG
		glDepthMask(GL_FALSE);
		glDisable(GL_DEPTH_TEST);
	#endif
	#ifndef BUFFERDEBU
		// begin light passes by setting blending so we can add the output of 
		// FS (source color) to framebuffer (destination color)
		//since each light source is handled by its own FS invocation
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_ONE, GL_ONE);
		gbuffer.ReadBind();
		glClear(GL_COLOR_BUFFER_BIT);
	#ifndef PROJECT1
	/*	// point lights pass
		pointlightShader->Use();
		CHECKERRORNOX
			pointlightShader->SetEyeWorldPos(eye[0], eye[1], eye[2]);
		CHECKERRORNOX
			int lightsPerCirc = 100;	// make new circle of lights after every 100 lights
										// every circle will have 50 lights more than the last
		int indexInc = floor((float)maxPointLights / numPointLights);
		int circleIndex = 0;
		for (int i = 0; i < numPointLights; i++)
		{

			int inCircleIndex = (numPointLights - i) % lightsPerCirc;
			//(numPointLights + (int)floor(((float)lightsPerCirc*circleIndex*.5f))) % lightsPerCirc;
			if (inCircleIndex == 0)
				circleIndex++;
			pointlightShader->SetCurrLight(pointLights[i]);
			MAT4 translate = Translate(pointLights[i].pos[0],
				pointLights[i].pos[1], pointLights[i].pos[2]);
			float bSphereScale = CalcPointLightBSphere(pointLights[i]);
			MAT4 scale = Scale(bSphereScale, bSphereScale, bSphereScale);
			MAT4 transform = (translate*scale);
			pointlightShader->SetWVP(VPmatrix * transform);
			sphere->Render();
			CHECKERRORNOX

				// rotate light around carousel
				int degreeArrindex = (i*indexInc * circleIndex) % maxPointLights; //(int)floor(((float)inCircleIndex / lightsPerCirc));
			float circSpeed = carouselSpeed / circleIndex;
			if (degreeIncCounter[degreeArrindex] > 2 * PI)
				degreeIncCounter[degreeArrindex] = 0;
			else
				degreeIncCounter[degreeArrindex] += circSpeed * (now - last_time);

			float r = carouselRadius * circleIndex;
			pointLights[i].pos = vec3(r * cos(degreeIncCounter[degreeArrindex]),
				r * sin(degreeIncCounter[degreeArrindex]), -3.8f);
		}*/
	#endif
	/*	pointlightShader->Use();
		pointlightShader->SetEyeWorldPos(eye[0], eye[1], eye[2]);
		for (int i = 0; i < NBODIES; i++)
		{
			pointLights[i].pos = physics.Bodies[i].lEndPt;
			MAT4 translate = Translate(pointLights[i].pos[0],
				pointLights[i].pos[1], pointLights[i].pos[2]);
			float bSphereScale = CalcPointLightBSphere(pointLights[i]);
			MAT4 scale = Scale(bSphereScale, bSphereScale, bSphereScale);
			MAT4 transform = (translate*scale);
			pointlightShader->SetWVP(VPmatrix * transform);
			sphere->Render();
			CHECKERRORNOX
		}*/

		aoFbo.BindForWrite();	// from now on all color writes will goto aoFBO.aoTexture?
		glClear(GL_COLOR_BUFFER_BIT);	// if we dont clear texture keeps accumulating the writes
		aoProgram->Use();
		aoProgram->SetUniformi("positionMap", GBuffer::GBUFFER_TYPE_POS);
		aoProgram->SetUniformi("normalMap", GBuffer::GBUFFER_TYPE_NORMAL);
		//loc = glGetUniformLocation(programId, "gWorld");
		//aoProgram->SetUniformi("depthMap", gbuffer.depthTexture);
		aoProgram->SetUniformi("depthMap", GBuffer::GBUFFER_TYPE_SPEC);	// specular is not spec, but it has 
														//depth in .z component of the vec3
		quad->Render();
		aoFbo.UnbindWrite();	// switch back to default framebuffer
		/*
		//////////////////////
		// bi lateral blur pass, aoTexture -> aoTextureF
		bilateralBlur->Use();
		bilateralBlur->SetUniformi("kernalSize", kernalSize);
		bilateralBlur->SetUniformi("normalMap", GBuffer::GBUFFER_TYPE_NORMAL);
		bilateralBlur->SetUniformi("depthMap", GBuffer::GBUFFER_TYPE_SPEC);
		glBindImageTexture(0, aoFbo.aoTexture, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);
		bilateralBlur->SetUniformi("src", 0);
		glBindImageTexture(1, aoFbo.aoTextureF, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);
		bilateralBlur->SetUniformi("dst", 1);
		glDispatchCompute(width / 148, height, 1); // Tiles WxH image with groups sized 128x1
		glMemoryBarrier(GL_ALL_BARRIER_BITS);
		////////////////////// vertical pass, aoTextureF -> aoTexture, so aoTexture is the final blurred texture
		bilateralBlurV->Use();
		bilateralBlurV->SetUniformi("kernalSize", kernalSize);
		bilateralBlurV->SetUniformi("normalMap", GBuffer::GBUFFER_TYPE_NORMAL);
		bilateralBlurV->SetUniformi("depthMap", GBuffer::GBUFFER_TYPE_SPEC);
		glBindImageTexture(0, aoFbo.aoTextureF, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);
		bilateralBlurV->SetUniformi("src", 0);
		glBindImageTexture(1, aoFbo.aoTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);
		bilateralBlurV->SetUniformi("dst", 1);
		glDispatchCompute(width , height/ 148, 1); // Tiles WxH image with groups sized 128x1
		//glMemoryBarrier(GL_ALL_BARRIER_BITS);
		*/
		


		////////////////////////// IBL
		
		envMap->Bind(GL_TEXTURE0 + 8);
		irradMap->Bind(GL_TEXTURE0 + 9);
		iblShader->Use();
		programId = iblShader->programId;
		iblShader->SetEnvMapTU(8);
		iblShader->SetIrradTU(9);
		iblShader->SetVP(WorldVP);
		iblShader->SetWorldMatrix(Identity);
		iblShader->SetAOMap(aoFbo.aoTexture);
		aoFbo.BindForRead(GL_TEXTURE0 + aoFbo.aoTexture);
		iblShader->SetNormalTU(GBuffer::GBUFFER_TYPE_NORMAL);
		iblShader->SetPosTU(GBuffer::GBUFFER_TYPE_POS);
		loc = glGetUniformLocation(programId, "colorMap");
		gl::glUniform1i(loc, GBuffer::GBUFFER_TYPE_DIFFUSE);
		iblShader->SetEyePos(eye);
		iblShader->SetScreenDim();
		//gbuffer.BindDepthForRead(GL_TEXTURE0 + gbuffer.depthTexture);
		//iblShader->SetDepthMap(gbuffer.depthTexture);
		



		quad->Render();
	
		// directinal light pass
		dirLightShader->Use();
		CHECKERRORNOX
			dirLightShader->SetEyeWorldPos(eye[0], eye[1], eye[2]);
		CHECKERRORNOX
			dirLightShader->SetWVP(Identity);
		CHECKERRORNOX
			quad->Render();
		CHECKERRORNOX
		

			// draw full screen quad to reach this light to all pixels
	#endif

	#ifdef BUFFERDEBUG
			// DS lightpass
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // restore default fbo(screen) and clear it
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		gbuffer.ReadBind();		// bind fbo and gbuffer
		GLsizei halfWidth = (GLsizei)(width / 2.0f);
		GLsizei halfHeight = (GLsizei)(height / 2.0f);
		// copy gbuffer textures into screen
		gbuffer.SetReadBuffer(GBuffer::GBUFFER_TYPE_NORMAL); //btm left
		glBlitFramebuffer(0, 0, width, height,
			0, 0, halfWidth, halfHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);
		gbuffer.SetReadBuffer(GBuffer::GBUFFER_TYPE_SPEC);	// top left
		glBlitFramebuffer(0, 0, width, height,
			0, halfHeight, halfWidth, height, GL_COLOR_BUFFER_BIT, GL_LINEAR);
		gbuffer.SetReadBuffer(GBuffer::GBUFFER_TYPE_DIFFUSE); //top R
		glBlitFramebuffer(0, 0, width, height,
			halfWidth, halfHeight, width, height, GL_COLOR_BUFFER_BIT, GL_LINEAR);
		gbuffer.SetReadBuffer(GBuffer::GBUFFER_TYPE_POS);
		// BlitFramebuffer requires source FBO to be bound to GL_READ_FRAMEBUFFER
		// destination FBO to GL_DRAW_FRAMEBUFFER
		glBlitFramebuffer(0, 0, width, height,
			halfWidth, 0, width, halfHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);
	#endif
#endif

	lightingProgram->Unuse();	// will unuse all programs
	CHECKERRORNOX;
	last_time = now;
}


float Scene::CalcPointLightBSphere(const PointLight& Light)
{
	float MaxChannel = fmax(fmax(Light.color.x, Light.color.y), Light.color.z);

	float ret = (-Light.atten.linear + sqrtf(Light.atten.linear * Light.atten.linear - 4 *
		Light.atten.exp * (Light.atten.exp - 256 * MaxChannel * Light.diffuseIntensity)))
		/
		(2 * Light.atten.exp);

	return ret;
}


void Scene::UpdateWalker(float elapsedTime)
{
	walkPath.UpdateWalker(elapsedTime);
	skeleton->currWalkSpeed = walkPath.walkerSpeed;
	skeleton->globalInvTransform = walkPath.walkerTransform;
}
void Scene::ShadowMapPass()
{
	
}

void Scene::InitializePascalsTri(int numberOfRows)
{
	// Initialize pascals triangle
	pascalsTriangle.resize(numberOfRows);
	for (int i = 0; i < numberOfRows; i++)
		pascalsTriangle[i].resize(numberOfRows);
	pascalsTriangle[0][0] = 1;

	// complete rows of triangle
	for (int r = 2; r <= numberOfRows; r++)
	{
		// pad row with ones
		pascalsTriangle[r - 1][0] = 1;
		pascalsTriangle[r - 1][r - 1] = 1;

		// fill in colums in between the ones
		for (int j = 0; j <= r - 3; j++)
		{
			pascalsTriangle[r - 1][j + 1] =
				pascalsTriangle[r - 2][j] + pascalsTriangle[r - 2][j + 1];
		}
	}
}