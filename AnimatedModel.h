// AnimatedModel.h
// Description: contains animated model and it's meshes
// Animated mesh class loads and stores animation data from
// mesh file. it stores information about bones which are contained as
// nodes in a hierarchy. Each node has VQS (vector, quaternion, scale)
// transformations which

#include <vector>
#include <map>
#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>       // Output data structure
#include <assimp/postprocess.h> // Post processing flags
#include <glm/glm.hpp>
#include <assimp/types.h>
#include <glbinding/gl/gl.h>
//#include "transform.h"
#include <assimp/types.h>
#include "texture.h"
#include "VQS.h"
#include "mesh.h"
//#include "shader.h"
using namespace gl;
using namespace std;

// buffers in buffer array that go to the shader
#define INDEX_BUFFER 0    
#define POS_VB       1
#define NORMAL_VB    2
#define TEXCOORD_VB  3    
#define BONE_VB		 4
#define NUM_VBs		 5

#define INVALID_MATERIAL 0xFFFFFFFF

#define BONES_PER_VERT 4 // maximum bones that affect a vertex
#define ARRAY_SIZE_IN_ELEMENTS(a) (sizeof(a)/sizeof(a[0]))


class AnimatedModel
{
public:
	struct MeshEntry {
		MeshEntry() {
			NumIndices = 0; BaseVertex = 0; BaseIndex = 0;
			MaterialIndex = INVALID_MATERIAL;
		}
		unsigned int NumIndices;
		unsigned int BaseVertex;
		unsigned int BaseIndex;
		unsigned int MaterialIndex;
	};

	// stores the bones and their respective weights that affect a vertex
	struct SkinnedVertData
	{
		unsigned int BoneIDs[BONES_PER_VERT]; //bones that influence vertex
		float BoneWeights[BONES_PER_VERT]; //amount of influence each bone has
										   // should add up to 1.0
		// add bone vertex data in next available slot
		void AddBoneVertexInfo(int boneId, float weight)
		{
			for (int i = 0; i < ARRAY_SIZE_IN_ELEMENTS(BoneIDs); i++)
			{
				if (BoneWeights[i] == 0) // no weight yet added
				{
					BoneIDs[i] = boneId;
					BoneWeights[i] = weight;
					return;
				}
			}
			printf("not enough bone slots");
		}
	};
	struct KeyFrame
	{
		float Time;	// range 0 to 1, at what percentage of duration does the keyframe begin
		VQS ToParentFromBone;
	};
	struct Track
	{
		vector<KeyFrame> KeyFrames;
	};
	struct Animation
	{
		string Name; // ie start, walk, stop

		float Duration;
		int TicksPerSec;
		Track Track;
	};
	struct Bone
	{
		MAT4 ToModelFromBone;	// from bonespace to local (for skin)
		MAT4 ToBoneFromModel; // model to bonespace (inverse) (for skin)
		MAT4 AnimatedTranform;	// results from keyframe interpolation
		VQS Transformation;		// pulled from hierarchy data
		unsigned int parentIndex;
		vector<int> childrenIndices;
		map<string, Animation> animationMap;	//allows indexing into animation by name
		int boneIndex;
		string name;
		vec3 pos;	// bone pos at some point in animation, see DrawBones()
		bool isIK;	// is the bone an IK node, false if not.
		bool isEnd; // is this bone and end piece
		Bone() {}
	};



	AnimatedModel();
	~AnimatedModel();
	void LoadMesh(const string& filename);
	bool InitModelInfo(const aiScene* pScene, const string& filename);
	void InitMesh(unsigned int meshIndex, const aiMesh* paiMesh,
		vector<vec3>& positions,vector<vec3>& normals,vector<vec2>& texCoords,
		vector<SkinnedVertData>& bones,vector<unsigned int>& indices);
	bool InitMaterials(const aiScene* pScene, const string& filename);
	void LoadBones(unsigned int meshIndex, vector<SkinnedVertData>& bones, const aiMesh* pMesh);
	void LoadBoneAnimations(const aiScene* pScene);

	void DrawBones(int gWorldLoc, MAT4 modelTrans, int shaderId);
	// fills skeleton array so child/parent indices are stored in each node
	void MakeBoneHierarchy(aiNode* node, int father);
	void DrawLine(vec3 start, vec3 end);
	void UpdateAnimations(float timeSinceUpdate);
	// resursive function to make animation transformations through hierarchy
	void UpdateAnimations(int currBone, VQS& currVQS, vec3 lastBonePos);
	void RenderModel();
	void GetTransforms(vector<MAT4>& transforms);
	void SwitchAnimation();
	void MoveHighLight(bool up)
	{
		if (up)
			currBoneToHighlight++;
		else if (currBoneToHighlight>0)
			currBoneToHighlight--;
		printf("\nhighlighted bone: %s", skeleton[currBoneToHighlight].name.c_str());
	}
	int currBoneToHighlight = 0;

	MAT4 globalInvTransform;	// inverse transform for root node
	float currWalkSpeed;
private:

	void Clear();
	aiNode* GetRootBoneNode(aiNode* node);
	//Bone* GetBoneFromaiNode(aiNode* pNode);
	vector<MeshEntry> meshEntries;
	vector<Texture*> textures;
	vector<Bone> skeleton;
	vector<string> animationNames;
	string currAnimation;
	int currAnimIndex = 2;
	// maps bone name to its index in skeleton array
	map<string, unsigned int> boneNameToIdMap;
	BasicMesh* bonemesh;	// for draw bones
	GLuint buffers[NUM_VBs];	// buffers that are sent to shader for drawing model
	GLuint mVAO;	// vertex array object

	unsigned int numBones;
	bool drawBones;
	aiNode* rootBoneNode;
	std::map<string, unsigned int>::iterator mapIt;
	float elapsedAnimTime;
	int MaxBonesToAdd = 5;	// for debug, how many bones are allowed to be added from model
	//float walkSpeed = .19f;	// speed of walk that matches speedmUltipier=1
	float speedMultiplier = 1.f;	// custom tweaked for the animation to match movement
};

