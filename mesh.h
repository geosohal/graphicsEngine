/*

Copyright 2011 Etay Meiri

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef MESH_H
#define	MESH_H

//#include <map>
#include <vector>
#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>       // Output data structure
#include <assimp/postprocess.h> // Post processing flags
#include <glbinding/gl/gl.h>
//#include <glbinding/Binding.h>
#include <glm/glm.hpp>
#include "transform.h"
#include "texture.h"

#include <assimp/types.h>

using namespace glm;
using namespace gl;
struct Vertex
{
	vec3 m_pos;
	vec2 m_tex;
	vec3 m_normal;

	Vertex() {}

	Vertex(const vec3& pos, const vec2& tex, const vec3& normal)
	{
		m_pos = pos;
		m_tex = tex;
		m_normal = normal;
	}
};


class BasicMesh
{
public:
	BasicMesh();

	~BasicMesh();

	bool LoadMesh(const std::string& Filename);

	void Render();

	void Render(unsigned int NumInstances, const MAT4* WVPMats, const MAT4* WorldMats);
	void SetTextureUnit(GLenum tu);
	std::vector<Texture*> m_Textures;

private:
	bool InitFromScene(const aiScene* pScene, const std::string& Filename);
	void InitMesh(const aiMesh* paiMesh,
		std::vector<vec3>& Positions,
		std::vector<vec3>& Normals,
		std::vector<vec2>& TexCoords,
		std::vector<unsigned int>& Indices);

	bool InitMaterials(const aiScene* pScene, const std::string& Filename);
	void Clear();

#define INVALID_MATERIAL 0xFFFFFFFF

#define INDEX_BUFFER 0    
#define POS_VB       1
#define NORMAL_VB    2
#define TEXCOORD_VB  3    
#define WVP_MAT_VB   4
#define WORLD_MAT_VB 5

	GLuint m_VAO;
	GLuint m_Buffers[6];

	struct BasicMeshEntry {
		BasicMeshEntry()
		{
			NumIndices = 0;
			BaseVertex = 0;
			BaseIndex = 0;
			MaterialIndex = INVALID_MATERIAL;
		}

		unsigned int NumIndices;
		unsigned int BaseVertex;
		unsigned int BaseIndex;
		unsigned int MaterialIndex;
	};

	std::vector<BasicMeshEntry> m_Entries;

	GLenum textureUnit;

};


#endif	/* MESH_H */

