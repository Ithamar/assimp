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
#include "RWXParser.h"
#include <memory>
#include <map>
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

    RWXParser parser(buffer);
    parser.Parse();

    pScene->mRootNode = new aiNode();
    pScene->mRootNode->mName.Set("<RWXRoot>");
    pScene->mRootNode->mNumMeshes = 1;
    pScene->mRootNode->mMeshes = new unsigned int [pScene->mRootNode->mNumMeshes];
    pScene->mRootNode->mMeshes[0] = 0;
/*
    // generate mesh
    pScene->mNumMeshes = 1;
    pScene->mMeshes = new aiMesh*[pScene->mNumMeshes];
    aiMesh* mesh = new aiMesh();
    pScene->mMeshes[0] = mesh;
    mesh->mNumFaces = o.geo.indices.size() / 3;
    aiFace* faces = new aiFace [mesh->mNumFaces];
    mesh->mFaces = faces;
    mesh->mNumVertices = o.outVertices;
    mesh->mVertices = new aiVector3D[mesh->mNumVertices];
    if (o.geo.uvs.size()) {
        mesh->mNumUVComponents[0] = 2;
        mesh->mTextureCoords[0] = new aiVector3D[mesh->mNumVertices];
    }
    unsigned int vIdx = 0;
    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        unsigned int a = o.geo.indices[i*3+0] -1,
                     b = o.geo.indices[i*3+1] -1,
                     c = o.geo.indices[i*3+2] -1;

        faces->mNumIndices = 3;
        faces->mIndices = new unsigned int[3];
        faces->mIndices[0] = vIdx +0;
        faces->mIndices[1] = vIdx +1;
        faces->mIndices[2] = vIdx +2;
        ++faces;
        mesh->mVertices[vIdx+0] = o.geo.vertices[a];
        mesh->mVertices[vIdx+1] = o.geo.vertices[b];
        mesh->mVertices[vIdx+2] = o.geo.vertices[c];
        if (o.geo.uvs.size()) {
            mesh->mTextureCoords[0][vIdx+0] = o.geo.uvs[a];
            mesh->mTextureCoords[0][vIdx+1] = o.geo.uvs[b];
            mesh->mTextureCoords[0][vIdx+2] = o.geo.uvs[c];
        }
        vIdx += 3;
    }

    // generate material
    pScene->mNumMaterials = 1;
    pScene->mMaterials = new aiMaterial*[pScene->mNumMaterials];
    aiMaterial* pcMat = new aiMaterial();
    pScene->mMaterials[0] = pcMat;
    // set material textures
    if (mat.textureDiffuse.length) pcMat->AddProperty( &mat.textureDiffuse, AI_MATKEY_TEXTURE_DIFFUSE(0));
    if (mat.textureBump.length) pcMat->AddProperty( &mat.textureBump, AI_MATKEY_TEXTURE_NORMALS(0));
    // set material colors
    aiColor4D diff(mat.color.r * mat.diffuse, mat.color.g * mat.diffuse, mat.color.b * mat.diffuse, 1);
    pcMat->AddProperty(&diff, 1, AI_MATKEY_COLOR_DIFFUSE);
    aiColor4D amb(mat.color.r * mat.ambient, mat.color.g * mat.ambient, mat.color.b * mat.ambient, 1);
    pcMat->AddProperty(&amb, 1, AI_MATKEY_COLOR_AMBIENT);
    aiColor4D spec(mat.color.r * mat.specular, mat.color.g * mat.specular, mat.color.b * mat.specular, 1);
    pcMat->AddProperty(&amb, 1, AI_MATKEY_COLOR_SPECULAR);
*/
}

#endif // !! ASSIMP_BUILD_NO_RWX_IMPORTER

