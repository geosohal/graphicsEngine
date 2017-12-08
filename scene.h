////////////////////////////////////////////////////////////////////////
// The scene class contains all the parameters needed to define and
// draw the (really) simple scene, including:
//   * Geometry (in a display list)
//   * Light parameters
//   * Material properties
//   * Viewport size parameters
//   * Viewing transformation values
//   * others ...
//
// Some of these parameters are set when the scene is built, and
// others are set by the framework in response to user mouse/keyboard
// interactions.  All of them should be used to draw the scene.

#include "shapes.h"
#include "object.h"
#include "texture.h"
#include "gbuffer.h"
//#include "mesh.h"
#include "AnimatedModel.h"
#include "ShadowFbo.h"
#include "walkpath.h"
#include "aoBuffer.h"
#define SOLUTION
enum ObjectIds {
    nullId	= 0,
    skyId	= 1,
    seaId	= 2,
    groundId	= 3,
    wallId	= 4,
    boxId	= 5,
    frameId	= 6,
    lPicId	= 7,
    rPicId	= 8,
    teapotId	= 9,
    spheresId	= 10,
};
const float PI = 3.14159f;

class Shader;

class Scene
{
public:
    // Viewing transformation parameters (suggested) FIXME: This is a
    // good place for the transformation values which are set by the
    // user mouse/keyboard actions and used in DrawScene to create the
    // transformation matrices.

    ProceduralGround* ground;

    // Light position parameters
    float lightSpin, lightTilt, lightDist;

#ifdef SOLUTION
    bool nav;
    char key;
    float spin, tilt, speed, ry, front, back;
    vec3 eye, tr;
    float last_time;
#endif
    vec3 basePoint;  // Records where the scene building is centered.
    int mode; // Extra mode indicator hooked up to number keys and sent to shader
    
    // Viewport
    int width, height;

    // All objects in the scene are children of this single root object.
    Object* objectRoot;
    std::vector<Object*> animated;

    // Shader programs
    ShaderProgram* lightingProgram;
	PLightShaderProgram* pointlightShader;
	DirLightProgram* dirLightShader;
	ShaderProgram* lineShader;
	ShaderProgram* triangleShader;	// for drawing bones as triangle primitive
	SkinProgram* skinningShader;
	ShadowProgram* shadowShader;
	SpotLightProgram* spLightShader;
	ShaderProgram* blurShader;	// compute shader for horizontal shadows blur pass
	ShaderProgram* blurShaderV;	// vertical pass
	IBLProgram* iblShader;
	ShaderProgram* aoProgram; //ambient occl
	ShaderProgram* bilateralBlur;
	ShaderProgram* bilateralBlurV;
	GBuffer gbuffer;
//	ShadowFbo shadowFbo;
	AoFbo aoFbo;


    //void append(Object* m) { objects.push_back(m); }

	//Scene();
    void InitializeScene();
    void DrawScene();

	BasicMesh* testMesh;
	BasicMesh* box;
	BasicMesh* sphere;
	BasicMesh* quad;
	BasicMesh* smQuad;
	BasicMesh* skelly;
	BasicMesh* spider;
	BasicMesh* gridMesh;	//water mesh
	Texture* envMap;
	Texture* irradMap;
	AnimatedModel* skeleton;
	WalkPath walkPath;

	static const int maxPointLights = 200;
	int numPointLights =0;
//	float pointLightSpacing = 30.f;
	float carouselRadius = 8.f;
	PointLight pointLights[maxPointLights];
	float degreeIncCounter[maxPointLights];
	float carouselSpeed = PI/6000.f; // degree per second
	DirectionalLight dirLight;
	SpotLight spotLight;
	int randomness = 1;
	std::vector<float> shCoeffs;	// spherical harmonic coefficients
	




private:
	float CalcPointLightBSphere(const PointLight& Light);
	void ShadowMapPass();

	void RenderModels();
	void InitializePascalsTri(int numRows);
	void UpdateWalker(float elapsedTime);
	//file loadinng helpers:

	int kernalSize;
	float lastctime;
	vector<vector<unsigned int>> pascalsTriangle;
};
