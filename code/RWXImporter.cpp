/*
---------------------------------------------------------------------------
Open Asset Import Library (assimp)
---------------------------------------------------------------------------

Copyright (c) 2006-2017, assimp team


All rights reserved.

Redistribution and use of this software in source and binary forms,
with or without modification, are permitted provided that the following
conditions are met:

* Redistributions of source code must retain the above
  copyright notice, this list of conditions and the
  following disclaimer.

* Redistributions in binary form must reproduce the above
  copyright notice, this list of conditions and the
  following disclaimer in the documentation and/or other
  materials provided with the distribution.

* Neither the name of the assimp team, nor the names of its
  contributors may be used to endorse or promote products
  derived from this software without specific prior
  written permission of the assimp team.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
---------------------------------------------------------------------------
*/

/** @file  RWXImporter.cpp
 *  @brief Implementation of the RWX importer class
 */


#ifndef ASSIMP_BUILD_NO_RWX_IMPORTER

// internal headers
#include "RWXImporter.h"
#include "ParsingUtils.h"
#include "fast_atof.h"
#include <memory>
#include <assimp/IOSystem.hpp>
#include <assimp/scene.h>
#include <assimp/DefaultLogger.hpp>
#include <assimp/importerdesc.h>

using namespace Assimp;

static const aiImporterDesc desc = {
    "RWX Importer",
    "",
    "",
    "",
    aiImporterFlags_SupportBinaryFlavour,
    0,
    0,
    0,
    0,
    "rwx"
};

// ------------------------------------------------------------------------------------------------
// Constructor to be privately used by Importer
RWXImporter::RWXImporter()
{}

// ------------------------------------------------------------------------------------------------
// Destructor, private as well
RWXImporter::~RWXImporter()
{}

// ------------------------------------------------------------------------------------------------
// Returns whether the class can handle the format of the given file.
bool RWXImporter::CanRead( const std::string& pFile, IOSystem* pIOHandler, bool checkSig) const
{
    const std::string extension = GetExtension(pFile);

    if (extension == "rwx")
        return true;
    else if (!extension.length() || checkSig)
    {
        if (!pIOHandler)return true;
        const char* tokens[] = {"clumpbegin", "modelbegin"};
        return SearchFileHeaderForToken(pIOHandler,pFile,tokens,1);
    }
    return false;
}

// ------------------------------------------------------------------------------------------------
const aiImporterDesc* RWXImporter::GetInfo () const
{
    return &desc;
}

// ------------------------------------------------------------------------------------------------
// Imports the given file into the given scene structure.
void RWXImporter::InternReadFile( const std::string& pFile,
    aiScene* pScene, IOSystem* pIOHandler)
{
    std::unique_ptr<IOStream> file( pIOHandler->Open( pFile, "rb"));

    // Check whether we can read from the file
    if( file.get() == NULL) {
        throw DeadlyImportError( "Failed to open RWX file " + pFile + ".");
    }

    // allocate storage and copy the contents of the file to a memory buffer
    std::vector<char> mBuffer2;
    TextFileToBuffer(file.get(),mBuffer2);
    const char* buffer = &mBuffer2[0];

    std::vector<aiVector3D> vertices;
    std::vector<aiVector3D> uvs;
    std::vector<unsigned int> indices;
    float ambient = 1, diffuse = 1, specular = 1;
    aiColor4D color(1,1,1,1);
    aiString textureDiffuse;
    aiString textureBump;

    char line[4096];
    std::string token;
    while(GetNextLine(buffer,line)) {
      const char* sz = line;
      // If line is empty or we hit '#' skip line
      if (!SkipSpacesAndLineEnd(&sz) || *sz == '#')
        continue;
      if (TokenMatchI(sz, "modelbegin", 10) || TokenMatchI(sz, "modelend", 8)) {
        // ignore, they have no function and are optional
      } else if (TokenMatchI(sz, "clumpbegin", 10)) {
        // no arguments
      } else if (TokenMatchI(sz, "clumpend", 8)) {
        // no arguments
      } else if (TokenMatchI(sz, "surface", 7)) {
        // ambient diffuse specular
	ai_real a, d, s;
        sz = fast_atoreal_move<ai_real>(sz,a); SkipSpaces(&sz);
	sz = fast_atoreal_move<ai_real>(sz,d); SkipSpaces(&sz);
	sz = fast_atoreal_move<ai_real>(sz,s); SkipSpaces(&sz);
        ambient = a; diffuse = d; specular = s;
        printf("surface %f %f %f\n", a, d, s);
      } else if (TokenMatchI(sz, "ambient", 7)) {
        ai_real c;
        sz = fast_atoreal_move<ai_real>(sz,c); SkipSpaces(&sz);
        ambient = c;
      } else if (TokenMatchI(sz, "diffuse", 7)) {
        ai_real c;
        sz = fast_atoreal_move<ai_real>(sz,c); SkipSpaces(&sz);
        diffuse = c;
      } else if (TokenMatchI(sz, "specular", 8)) {
        ai_real c;
        sz = fast_atoreal_move<ai_real>(sz,c); SkipSpaces(&sz);
        specular = c;
      } else if (TokenMatchI(sz, "color", 5)) {
	ai_real r, g, b;
        sz = fast_atoreal_move<ai_real>(sz,r); SkipSpaces(&sz);
	sz = fast_atoreal_move<ai_real>(sz,g); SkipSpaces(&sz);
	sz = fast_atoreal_move<ai_real>(sz,b); SkipSpaces(&sz);
        color = aiColor4D(r,g,b, 1);
        printf("color %f %f %f\n", r, g, b);
      } else if (TokenMatchI(sz, "vertex", 6)) {
	ai_real x, y, z;
        sz = fast_atoreal_move<ai_real>(sz,x); SkipSpaces(&sz);
	sz = fast_atoreal_move<ai_real>(sz,y); SkipSpaces(&sz);
	sz = fast_atoreal_move<ai_real>(sz,z); SkipSpaces(&sz);
	vertices.push_back(aiVector3D(x,y,z));
        printf("vertex %f %f %f", x, y, z);
        if (TokenMatchI(sz, "uv", 2)) {
          ai_real u, v;
	  sz = fast_atoreal_move<ai_real>(sz,u); SkipSpaces(&sz);
	  sz = fast_atoreal_move<ai_real>(sz,v); SkipSpaces(&sz);
	  uvs.push_back(aiVector3D(u,v,0));
          printf("uv %f %f", u, v);
	}
        printf("\n");
        // prelight extension?
      } else if (TokenMatchI(sz, "geometrysampling", 16)) {
        std::string mode = GetNextToken(sz);
        // solid, wireframe, pointcloud
	// XXX handle wireframe?
      } else if (TokenMatchI(sz, "triangle", 7)) {
	indices.push_back( strtoul10(sz,&sz) ); SkipSpaces(&sz);
	indices.push_back( strtoul10(sz,&sz) ); SkipSpaces(&sz);
	indices.push_back( strtoul10(sz,&sz) ); SkipSpaces(&sz);
        if (TokenMatchI(sz, "tag", 3)) {
          /*unsigned int tag = */strtoul10(sz,&sz); SkipSpaces(&sz);
	  // XXX store tag as material?
	}
      } else if (TokenMatchI(sz, "block", 5)) {
        ai_real width, height, depth;
        sz = fast_atoreal_move<ai_real>(sz,width);  SkipSpaces(&sz);
        sz = fast_atoreal_move<ai_real>(sz,height); SkipSpaces(&sz);
        sz = fast_atoreal_move<ai_real>(sz,depth);  SkipSpaces(&sz);
	// TODO block
      } else if (TokenMatchI(sz, "hemisphere", 10)) {
        ai_real radius, density;
        sz = fast_atoreal_move<ai_real>(sz,radius);  SkipSpaces(&sz);
        sz = fast_atoreal_move<ai_real>(sz,density); SkipSpaces(&sz);
        // TODO hemisphere
      } else if (TokenMatchI(sz, "quad", 4)) {
        // Quad v1 v2 v3 v4 [UV u v] [Tag value]
	unsigned long v1, v2, v3, v4;
	v1 = strtoul10(sz,&sz); SkipSpaces(&sz);
	v2 = strtoul10(sz,&sz); SkipSpaces(&sz);
	v3 = strtoul10(sz,&sz); SkipSpaces(&sz);
	v4 = strtoul10(sz,&sz); SkipSpaces(&sz);
        printf("quad %lu %lu %lu %lu", v1, v2, v3, v4);
        // generate two triangles...
	indices.push_back(v1); indices.push_back(v2); indices.push_back(v3);
	indices.push_back(v3); indices.push_back(v4); indices.push_back(v1);
        if (TokenMatchI(sz, "uv", 2)) {
          ai_real u, v;
	  sz = fast_atoreal_move<ai_real>(sz,u); SkipSpaces(&sz);
	  sz = fast_atoreal_move<ai_real>(sz,v); SkipSpaces(&sz);
          printf(" uv %f %f", u, v);
          // TODO uv for quads
	}
        if (IsNumeric(*sz)) {
          unsigned long tag;
	  tag = strtoul10(sz,&sz); SkipSpaces(&sz);
          printf("tag %lu", tag);
	  // XXX store tag as material?
	}
        printf("\n");
      } else if (TokenMatchI(sz, "texturemode", 11) || TokenMatchI(sz, "texturemodes", 12)) {
        // lit (default), foreshorten, or filter (or NULL)
	printf("texturemode[s] %s\n", GetNextToken(sz).c_str());
      } else if (TokenMatchI(sz, "texture", 7)) {
        textureDiffuse = (GetNextToken(sz) + ".jpg").c_str();
        printf("texture %s", textureDiffuse.C_Str());
        if (TokenMatchI(sz, "mask", 4)) {
          printf(" mask %s", GetNextToken(sz).c_str());
	} else if (TokenMatchI(sz, "bump", 4)) {
          textureBump = GetNextToken(sz).c_str();
          printf(" bump %s", textureBump.C_Str());
	}
        printf("\n");
      } else if (TokenMatchI(sz, "lightsampling", 13)) {
        std::string mode = GetNextToken(sz);
        // facet (default) or vertex
      } else if (TokenMatchI(sz, "hints", 5)) {
        // Not supported by ActiveWorlds browsers....
      } else if (TokenMatchI(sz, "axisalignment", 13)) {
        // zorientx, zorienty or none
        std::string mode = GetNextToken(sz);
      } else {
        printf("Unknown token '%s'!\n", GetNextToken(sz).c_str());
      }
    }

    pScene->mRootNode = new aiNode();
    pScene->mRootNode->mName.Set("<RWXRoot>");
    pScene->mRootNode->mNumMeshes = 1;
    pScene->mRootNode->mMeshes = new unsigned int [pScene->mRootNode->mNumMeshes];
    pScene->mRootNode->mMeshes[0] = 0;

    // generate mesh
    pScene->mNumMeshes = 1;
    pScene->mMeshes = new aiMesh*[pScene->mNumMeshes];
    aiMesh* mesh = new aiMesh();
    pScene->mMeshes[0] = mesh;
    mesh->mNumVertices = indices.size();
    mesh->mVertices = new aiVector3D[mesh->mNumVertices];
    mesh->mNumFaces = indices.size() / 3;
    aiFace* faces = new aiFace [mesh->mNumFaces];
    mesh->mFaces = faces;
    if (uvs.size()) {
      mesh->mNumUVComponents[0] = 2;
      mesh->mTextureCoords[0] = new aiVector3D[indices.size()];
    }
    unsigned int vIdx = 0;
    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
      unsigned int a = indices[i*3+0] -1,
		   b = indices[i*3+1] -1,
		   c = indices[i*3+2] -1;

      faces->mNumIndices = 3;
      faces->mIndices = new unsigned int[3];
      faces->mIndices[0] = vIdx +0;
      faces->mIndices[1] = vIdx +1;
      faces->mIndices[2] = vIdx +2;
      ++faces;
      mesh->mVertices[vIdx+0] = vertices[a];
      mesh->mVertices[vIdx+1] = vertices[b];
      mesh->mVertices[vIdx+2] = vertices[c];
      if (uvs.size()) {
        mesh->mTextureCoords[0][vIdx+0] = uvs[a];
        mesh->mTextureCoords[0][vIdx+1] = uvs[b];
        mesh->mTextureCoords[0][vIdx+2] = uvs[c];
      }
      vIdx += 3;
    }

    // generate material
    pScene->mNumMaterials = 1;
    pScene->mMaterials = new aiMaterial*[pScene->mNumMaterials];
    aiMaterial* pcMat = new aiMaterial();
    pScene->mMaterials[0] = pcMat;
    // set material textures
    if (textureDiffuse.length) pcMat->AddProperty( &textureDiffuse, AI_MATKEY_TEXTURE_DIFFUSE(0));
    if (textureBump.length) pcMat->AddProperty( &textureBump, AI_MATKEY_TEXTURE_NORMALS(0));
    // set material colors
    aiColor4D diff(color.r * diffuse, color.g * diffuse, color.b * diffuse, 1);
    pcMat->AddProperty(&diff, 1, AI_MATKEY_COLOR_DIFFUSE);
    aiColor4D amb(color.r * ambient, color.g * ambient, color.b * ambient, 1);
    pcMat->AddProperty(&amb, 1, AI_MATKEY_COLOR_AMBIENT);
    aiColor4D spec(color.r * specular, color.g * specular, color.b * specular, 1);
    pcMat->AddProperty(&amb, 1, AI_MATKEY_COLOR_SPECULAR);

}

#endif // !! ASSIMP_BUILD_NO_RWX_IMPORTER

