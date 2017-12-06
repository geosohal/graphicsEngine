#include <glbinding/Binding.h>
#include "AnimatedModel.h"
#include <glu.h>                // For gluErrorString
#include "utility.h"

using namespace std;
using namespace gl;
using namespace glm;
#define ZERO_MEM(a) memset(a, 0, sizeof(a))
#define SAFE_DELETE(p) if (p) { delete p; p = NULL; }
#define ARRAY_SIZE_IN_ELEMENTS(a) (sizeof(a)/sizeof(a[0]))

#define POSITION_LOCATION    0
#define NORMAL_LOCATION      1
#define TEX_COORD_LOCATION   2
#define BONE_ID_LOCATION     3
#define BONE_WEIGHT_LOCATION 4

#define CHECKERRORNOX {GLenum err = glGetError(); if (err != GL_NO_ERROR) { fprintf(stderr, "OpenGL cerror (at line %d): %s\n", __LINE__, gluErrorString(err)); } }


AnimatedModel::AnimatedModel()
{
	ZERO_MEM(buffers);
	drawBones = true;
	currWalkSpeed = 1;
}

AnimatedModel::~AnimatedModel()
{
	Clear();
}
void AnimatedModel::Clear()
{
	if (buffers[0] != 0) {
		glDeleteBuffers(NUM_VBs, buffers);
	}

	if (mVAO != 0) {
		glDeleteVertexArrays(1, &mVAO);
		mVAO = 0;
	}
}

void AnimatedModel::LoadMesh(const string& filename)
{
	numBones = 0;
	mVAO = 0;
	elapsedAnimTime = 0;
	Assimp::Importer importer;

	// Release the previously loaded mesh (if it exists)
	Clear();

	// Create the VAO
	glGenVertexArrays(1, &mVAO);
	glBindVertexArray(mVAO);
	// From now any change to state of vertex processor will affect this VAO

	// Create the buffers for the vertices attributes
	glGenBuffers(NUM_VBs, buffers);

	// set max 4 bones per vertex
	importer.SetPropertyInteger(AI_CONFIG_PP_LBW_MAX_WEIGHTS, 4);

	const aiScene* pScene = importer.ReadFile(filename.c_str(),
		aiProcess_Triangulate | aiProcess_GenNormals);

	if (pScene)
	{
		// get global transformation of model and store its inverse
		MAT4 globaltransform = aiToMAT4(pScene->mRootNode->mTransformation);
		invert(&globaltransform, &globalInvTransform);
		InitModelInfo(pScene, filename);
		aiNode* rootBoneNode = GetRootBoneNode(pScene->mRootNode);
		MakeBoneHierarchy(rootBoneNode, 0);
		LoadBoneAnimations(pScene);

	}

	CHECKERRORNOX

}

// the root bone node is sometimes not at the top of the hierarchy so we go down
// until we find it and MakeBoneHierarchy will use it to make the hierarchy
aiNode* AnimatedModel::GetRootBoneNode(aiNode* node)
{
	if (node->mName.data == skeleton[0].name)
		return node;
	// go two levels deep to find the root bone node
	for (unsigned int i = 0; i < node->mNumChildren; ++i)
	{
		if (node->mChildren[i]->mName.data == skeleton[0].name)
			return node->mChildren[i];
		for (unsigned int j = 0; j < node->mChildren[i]->mNumChildren; j++)
		{
			if (node->mChildren[i]->mChildren[j]->mName.data == skeleton[0].name)
				return node->mChildren[i]->mChildren[j];
		}
	}
}

// Initialize model information that will be sent to the shader
// through buffers such as: pos, normals, bones, texcoord, vertex skin data
bool AnimatedModel::InitModelInfo(const aiScene* pScene, const string& filename)
{
	meshEntries.resize(pScene->mNumMeshes);
	textures.resize(pScene->mNumMaterials);

	vector<vec3> Positions;
	vector<vec3> Normals;
	vector<vec2> TexCoords;
	vector<SkinnedVertData> VertexSkins;
	vector<unsigned> Indices;

	unsigned NumVertices = 0;
	unsigned NumIndices = 0;

	// Count the number of vertices and indices
	for (unsigned i = 0; i < meshEntries.size(); i++) {
		meshEntries[i].MaterialIndex = pScene->mMeshes[i]->mMaterialIndex;
		meshEntries[i].NumIndices = pScene->mMeshes[i]->mNumFaces * 3;
		meshEntries[i].BaseVertex = NumVertices;
		meshEntries[i].BaseIndex = NumIndices;

		NumVertices += pScene->mMeshes[i]->mNumVertices;
		NumIndices += meshEntries[i].NumIndices;
	}

	// Reserve space in the vectors for the vertex attributes and indices
	Positions.reserve(NumVertices);
	Normals.reserve(NumVertices);
	TexCoords.reserve(NumVertices);
	VertexSkins.resize(NumVertices);
	Indices.reserve(NumIndices);

	// Initialize the meshes in the scene one by one
	for (unsigned i = 0; i < meshEntries.size(); i++) {
		const aiMesh* paiMesh = pScene->mMeshes[i];
		InitMesh(i, paiMesh, Positions, Normals, TexCoords, VertexSkins, Indices);
	}

	InitMaterials(pScene, filename);
	CHECKERRORNOX
		// Generate and populate the buffers with vertex attributes and indices
		glBindBuffer(GL_ARRAY_BUFFER, buffers[POS_VB]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Positions[0]) * Positions.size(), &Positions[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(POSITION_LOCATION);
	glVertexAttribPointer(POSITION_LOCATION, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, buffers[TEXCOORD_VB]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(TexCoords[0]) * TexCoords.size(), &TexCoords[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(TEX_COORD_LOCATION);
	glVertexAttribPointer(TEX_COORD_LOCATION, 2, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, buffers[NORMAL_VB]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Normals[0]) * Normals.size(), &Normals[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(NORMAL_LOCATION);
	glVertexAttribPointer(NORMAL_LOCATION, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, buffers[BONE_VB]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(VertexSkins[0]) * VertexSkins.size(), &VertexSkins[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(BONE_ID_LOCATION);
	glVertexAttribIPointer(BONE_ID_LOCATION, 4, GL_INT, sizeof(SkinnedVertData), (const GLvoid*)0);

	glEnableVertexAttribArray(BONE_WEIGHT_LOCATION);
	glVertexAttribPointer(BONE_WEIGHT_LOCATION, 4, GL_FLOAT, GL_FALSE, sizeof(SkinnedVertData), (const GLvoid*)16);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[INDEX_BUFFER]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices[0]) * Indices.size(), &Indices[0], GL_STATIC_DRAW);
	CHECKERRORNOX
		return true;
}

// called by init model info. populates the vertices with information about each mesh in the model
void AnimatedModel::InitMesh(unsigned int meshIndex, const aiMesh* paiMesh,
	vector<vec3>& positions, vector<vec3>& normals,
	vector<vec2>& texCoords, vector<SkinnedVertData>& bones,
	vector<unsigned int>& indices)
{
	const aiVector3D zerovec(0.0f, 0.0f, 0.0f);

	// Populate the vertex attribute vectors
	for (unsigned i = 0; i < paiMesh->mNumVertices; i++) {
		const aiVector3D* pPos = &(paiMesh->mVertices[i]);
		const aiVector3D* pNormal = &(paiMesh->mNormals[i]);
		const aiVector3D* pTexCoord = paiMesh->HasTextureCoords(0) ? &(paiMesh->mTextureCoords[0][i]) : &zerovec;

		positions.push_back(vec3(pPos->x, pPos->y, pPos->z));
		normals.push_back(vec3(pNormal->x, pNormal->y, pNormal->z));
		texCoords.push_back(vec2(pTexCoord->x, pTexCoord->y));
	}

	LoadBones(meshIndex, bones, paiMesh);

	// Populate the index buffer
	for (unsigned i = 0; i < paiMesh->mNumFaces; i++) {
		const aiFace& Face = paiMesh->mFaces[i];
		assert(Face.mNumIndices == 3);
		indices.push_back(Face.mIndices[0]);
		indices.push_back(Face.mIndices[1]);
		indices.push_back(Face.mIndices[2]);
	}
}

void AnimatedModel::LoadBoneAnimations(const aiScene* scene)
{
	// iterate animations
	for (int i = 0; i < scene->mNumAnimations; i++)
	{
		aiAnimation* anim = scene->mAnimations[i];
		string animName = scene->mAnimations[i]->mName.C_Str();
		animationNames.push_back(animName);
		currAnimation = animName;//the last animation inserted will end up being the default
		// iterate channels/bones that animations includes
		for (int j = 0; j < anim->mNumChannels; j++)
		{
			aiNodeAnim* chan = anim->mChannels[j];

			// look up bone for this animation channel
			mapIt = boneNameToIdMap.find(chan->mNodeName.C_Str());
			if (mapIt == boneNameToIdMap.end())
			{
				printf("bone '%s' for anim not found", anim->mName.C_Str());
				continue;
			}
			int boneIndex = mapIt->second;
			Animation boneAnim;
			boneAnim.Duration = anim->mDuration;
			boneAnim.Name = animName;
			boneAnim.TicksPerSec = anim->mTicksPerSecond;
			int numKeys = chan->mNumPositionKeys;
			// store all VQS keyframes for this bones animations for this animation
			for (int k = 0; k < chan->mNumPositionKeys; k++)
			{
				KeyFrame key;
				aiVectorKey posKey = chan->mPositionKeys[k];
				aiQuatKey qKey = chan->mRotationKeys[k];
				aiVectorKey sKey = chan->mScalingKeys[k];
				key.Time = ((float)k / (numKeys - 1)); // time since start of animation
				key.ToParentFromBone = VQS(
					vec3(posKey.mValue[0], posKey.mValue[1], posKey.mValue[2]),
					Quaternion(qKey.mValue.w, qKey.mValue.x, qKey.mValue.y, qKey.mValue.z),
					sKey.mValue[0]);

				boneAnim.Track.KeyFrames.push_back(key);
			}
			// remember. animation map has "start", "stop", "walk"
			skeleton[boneIndex].animationMap[animName] = boneAnim;
		}
	}
}

bool AnimatedModel::InitMaterials(const aiScene* pScene, const string& Filename)
{
	// Extract the directory part from the file name
	string::size_type SlashIndex = Filename.find_last_of("/");
	string Dir;

	if (SlashIndex == string::npos) {
		Dir = ".";
	}
	else if (SlashIndex == 0) {
		Dir = "/";
	}
	else {
		Dir = Filename.substr(0, SlashIndex);
	}

	bool Ret = true;

	// Initialize the materials
	for (unsigned int i = 0; i < pScene->mNumMaterials; i++) {
		const aiMaterial* pMaterial = pScene->mMaterials[i];

		textures[i] = NULL;

		aiString Path;

		// test code for loading other texture types
		if (pMaterial->GetTextureCount(aiTextureType_SPECULAR) > 0) {
			textures[i]->Load();
		}
		if (pMaterial->GetTextureCount(aiTextureType_AMBIENT) > 0) {
			textures[i]->Load();
		}
		if (pMaterial->GetTextureCount(aiTextureType_NONE) > 0) {
			textures[i]->Load();
		}
		if (pMaterial->GetTextureCount(aiTextureType_NORMALS) > 0) {
			textures[i]->Load();
		}if (pMaterial->GetTextureCount(aiTextureType_SHININESS) > 0) {
			textures[i]->Load();
		}if (pMaterial->GetTextureCount(aiTextureType_REFLECTION) > 0) {
			textures[i]->Load();
		}if (pMaterial->GetTextureCount(aiTextureType_UNKNOWN) > 0) {
			textures[i]->Load();
		}if (pMaterial->GetTextureCount(aiTextureType_LIGHTMAP) > 0) {
			textures[i]->Load();
		}

		if (pMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
			aiString Path;

			if (pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
				string p(Path.data);

				if (p.substr(0, 2) == ".\\") {
					p = p.substr(2, p.size() - 2);
				}

				string FullPath = Dir + "/" + p;

				//TODOG
				textures[i] = new Texture(FullPath.c_str());

				if (!textures[i]->Load()) {
					printf("Error loading texture '%s'\n", FullPath.c_str());
					delete textures[i];
					textures[i] = NULL;
					Ret = false;
				}
				else {
					printf("Loaded texture '%s'\n", FullPath.c_str());
				}

			}
		}
	}
	return Ret;
}

// called from InitMesh(). it loads vertex skin information and bone information
// for a single aiMesh. another mesh may have already loaded the bone info in which case we skip
// adding the bone but for skinning we will record the vertex that uses this bone and record it's
// respective bone weight.
void AnimatedModel::LoadBones(unsigned int meshIndex, vector<SkinnedVertData>& bones, const aiMesh* pMesh)
{
	for (int i = 0; i < pMesh->mNumBones; i++)
	{
		string boneName = pMesh->mBones[i]->mName.data;
		unsigned int boneIndex;
		// if a bone is not yet added to skeleton
		if (boneNameToIdMap.find(boneName) == boneNameToIdMap.end())
		{
			// bone is not found so add it's information
			boneIndex = numBones++;
			Bone newBone;
			MAT4 boneTransform = aiToMAT4(pMesh->mBones[i]->mOffsetMatrix);
			newBone.ToModelFromBone = boneTransform;
			invert(&boneTransform, &newBone.ToBoneFromModel);
			newBone.name = boneName;
			newBone.boneIndex = boneIndex;
			newBone.isIK = false;
			newBone.isEnd = false;
			if (skeleton.size() == 0)	// if this is the first bone added its the root
				newBone.parentIndex = 0;	// root node is parent to itself
			skeleton.push_back(newBone);
			boneNameToIdMap[boneName] = boneIndex;
		}
		else
			boneIndex = boneNameToIdMap[boneName];

		// whether bone was found or not we record the vertex weight information next
		for (int k = 0; k < pMesh->mBones[i]->mNumWeights; k++)
		{
			int vertId = meshEntries[meshIndex].BaseVertex + pMesh->mBones[i]->mWeights[k].mVertexId;
			bones[vertId].AddBoneVertexInfo(boneIndex, pMesh->mBones[i]->mWeights[k].mWeight);
		}
	}

	// load bones can be called from multiple meshes within the model, also the same bone can
	// be listed in a meshes bones list (pMesh->mBones) multiple times. when a bone is read
	// multiple times it is affecting a different vertex, so we find the vertex and append
	// a new boneweight and it's corresponding bone id.
	// the absolute vertex id for this mesh instance's bone data is needed for adding bone
	// weight data. it is calculated by starting at the mesh entry's
	// base vertex and adding the mesh's bone weight vertex id since vertex IDs are relevant
	// to a single mesh.

}

void AnimatedModel::DrawBones(int gModelLoc, MAT4 modelTrans, int shaderId)
{
	glBegin(GL_LINES);
	for (int i = 0; i < 21; i++)//debug
	{
		if (skeleton[i].isIK)	// dont draw IK bones
			continue;
		// traverse children and draw from bone to each child
		for (int ch = 0; ch < skeleton[i].childrenIndices.size(); ch++)
		{
			int chIndex = skeleton[i].childrenIndices[ch];
			if (skeleton[chIndex].isIK)
				continue; // dont draw IK pieces
			
			
			vec3 startPos = skeleton[chIndex].pos;
			vec3 endPos = skeleton[i].pos;
			int loc = glGetUniformLocation(shaderId, "red");

			if (chIndex == currBoneToHighlight)// if bone is selected by highlighter
				glUniform1fARB(loc, 1);
			else 
				glUniform1fARB(loc, .5);

			DrawLine(startPos, endPos);

		/* deprecated code for drawing triangles instead of lines
		vec3 difference = skeleton[chIndex].pos - skeleton[i].pos; // bone from parent to child
			float len = length(difference);
			// get the quaternion to rotate the updirection to the vector of the bone
			Quaternion ztoaQuat = (Quaternion::FromZtoA(vec3(0, 0, 1), difference / len)).Normalize();
			// final transformation of the bone after its run through heierachy and is being animated
			
			MAT4 finalBoneTrans =  Translate(skeleton[i].pos[0], skeleton[i].pos[1],
			skeleton[i].pos[2]) * modelTrans * ztoaQuat.ToMat4() * Scale(1, len, 1);
			glUniformMatrix4fv(gModelLoc, 1, GL_TRUE, finalBoneTrans.Pntr());
			glBindVertexArray(mVAO);
			glDrawElements(GL_TRIANGLES, 13, GL_UNSIGNED_INT, (void*)0);*/
		}
	}

	glEnd();
}

// updates elapsed time, and then calls recursive function that updates animation transformations,
// by working through the hierarchy and using the parents as the origin
void AnimatedModel::UpdateAnimations(float timeSinceUpdate)
{
	float animTime = skeleton[0].animationMap[currAnimation].Duration ;
	// elapsed anim time is time elapsed in the animation loop
	elapsedAnimTime += timeSinceUpdate * currWalkSpeed;
	if (elapsedAnimTime > animTime)	// if we have finished the loop, we start elapsedtime over again at 0
		elapsedAnimTime = 0;

	UpdateAnimations(0, VQS(), vec3(0, 0, 0));
}

// update transformations for each bone, Bone.AnimatedTranform for each bone becomes the final
// bone transformation that accounts for animation and hierarchy transformations
void AnimatedModel::UpdateAnimations(int currBone, VQS& currVQS, vec3 lastBonePos)
{
	// time to traverse all keyframes is ticks/second * 1/ticks = animTime
	// current place in keyframe list is given by: percentComplete=  elapsedSeconds / animTime
	// elapsedSeconds is reset to 0 upon reaching animTime
	// prev and next keyframes gotten from percentComplete ratio so we then can interpolate between them

	float animTime = skeleton[0].animationMap[currAnimation].Duration;
	float percentComplete = elapsedAnimTime / animTime;
	if (percentComplete > 1.f)
	{
		percentComplete = 0;	// if animation completes, loop back to beginning 
		elapsedAnimTime = 0;
	}

	// store interpolated animation for this animation frame
	VQS interpolatedTrans = skeleton[currBone].Transformation;
	int numKeys = skeleton[currBone].animationMap[currAnimation].Track.KeyFrames.size();
	if (numKeys > 0)
	{
		int lastKey = numKeys - 1;
		int prevKeyIndex = (int)floorf(percentComplete * lastKey);
		//in the case that we are at the last key, then the next key will start over again at the first key
		int nextKeyIndex = prevKeyIndex + 1 > lastKey ? 0 : prevKeyIndex + 1;
		// get the percentages of total duration for each keyframe
		float prevKeyTime = skeleton[currBone].animationMap[currAnimation].Track.KeyFrames[prevKeyIndex].Time;
		float nextKeyTime = skeleton[currBone].animationMap[currAnimation].Track.KeyFrames[nextKeyIndex].Time;
		// percent between is percentage of the way we are between previous and next keyframe
		float percentBetween = (percentComplete - prevKeyTime) / (nextKeyTime - prevKeyTime);
		assert(percentBetween >= 0 && percentBetween <= 1);
		//
		if (skeleton[currBone].animationMap[currAnimation].Track.KeyFrames.size() > 0)
			interpolatedTrans = VQS::Interpolate(
				skeleton[currBone].animationMap[currAnimation].Track.KeyFrames[prevKeyIndex].ToParentFromBone,
				skeleton[currBone].animationMap[currAnimation].Track.KeyFrames[nextKeyIndex].ToParentFromBone,
				percentBetween);
	}
	//	else if there are no keyframes such as for some IK bones use the node transformation provided

	VQS nextVQS = currVQS * interpolatedTrans;	// update origin space for children
	skeleton[currBone].AnimatedTranform =  globalInvTransform* nextVQS.ToMat4() *skeleton[currBone].ToModelFromBone;
	MAT4 boneMatrix = globalInvTransform* nextVQS.ToMat4();
	// bone positions for this point in animation come from the translation component of the bone matrix
	skeleton[currBone].pos = vec3(boneMatrix[0][3], boneMatrix[1][3], boneMatrix[2][3]);
	for (int i = 0; i < skeleton[currBone].childrenIndices.size(); i++)
		UpdateAnimations(skeleton[currBone].childrenIndices[i], nextVQS, lastBonePos);

}
// father is the root bone of skeleton. recursive function that populates
// the children of the root following the hierarchy given by assimp
// also adds all IK bones which were not added by load bones since IK nodes
// are not related to meshes and load bones used aiMesh
void AnimatedModel::MakeBoneHierarchy(aiNode* pNode, int fatherIndex)
{
	// loop through all children and connect to father
	for (int i = 0; i < pNode->mNumChildren; i++)
	{
		Bone* childBone;
		mapIt = boneNameToIdMap.find(pNode->mChildren[i]->mName.data);
		if (mapIt == boneNameToIdMap.end())
		{
			if (pNode->mNumChildren == 0) // bone end piece, no further recursions necessary
				continue;
			else
			{	// the node has children that are not in our skeleton already
				// these children are IK bones and so we need to put these in the skeleton array
				Bone newBone;
				newBone.name = pNode->mChildren[i]->mName.C_Str();
				newBone.parentIndex = fatherIndex;
				int childIndex = numBones++;
				newBone.boneIndex = childIndex;
				newBone.Transformation = VQS(aiToMAT4(pNode->mChildren[i]->mTransformation));
				newBone.isIK = false; newBone.isEnd = false;
				boneNameToIdMap[newBone.name] = newBone.boneIndex;
				if (newBone.name.find("end") != std::string::npos)
					newBone.isEnd = true;
				if (newBone.name.find("IK") != std::string::npos)
					newBone.isIK = true;
				skeleton[fatherIndex].childrenIndices.push_back(childIndex);
				skeleton.push_back(newBone);
				//childBone = &skeleton[boneIndex];
				MakeBoneHierarchy(pNode->mChildren[i], childIndex);
				continue;
			}
		}
		// this is a normal (non-IK) bone so we just add indices into the existing bone in skeleton
		int childIndex = mapIt->second;
		skeleton[childIndex].parentIndex = fatherIndex;
		skeleton[childIndex].Transformation = VQS(aiToMAT4(pNode->mChildren[i]->mTransformation));
		skeleton[fatherIndex].childrenIndices.push_back(childIndex);
		MakeBoneHierarchy(pNode->mChildren[i], childIndex);
	}
}

void AnimatedModel::DrawLine(vec3 start, vec3 end)
{
	
	glVertex3f(start[0], start[1], start[2]);
	glVertex3f(end[0], end[1], end[2]);

}

void AnimatedModel::RenderModel()
{
	glBindVertexArray(mVAO);

	for (uint i = 0; i < meshEntries.size(); i++) {
		const uint MaterialIndex = meshEntries[i].MaterialIndex;

		assert(MaterialIndex < textures.size());

		if (textures[MaterialIndex]) {
			textures[MaterialIndex]->Bind(GL_TEXTURE0);
		}

		glDrawElementsBaseVertex(GL_TRIANGLES,
			meshEntries[i].NumIndices,
			GL_UNSIGNED_INT,
			(void*)(sizeof(uint) * meshEntries[i].BaseIndex),
			meshEntries[i].BaseVertex);
	}


	glBindVertexArray(0);
}

void AnimatedModel::GetTransforms(vector<MAT4>& transforms)
{
	for (int i = 0; i < skeleton.size(); i++)
	{
		if (!skeleton[i].isIK && !skeleton[i].isEnd)
			transforms.push_back(skeleton[i].AnimatedTranform);
	}
}


void AnimatedModel::SwitchAnimation()
{
	currAnimIndex = (currAnimIndex+1) % animationNames.size();
	currAnimation = animationNames[currAnimIndex];
}