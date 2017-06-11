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

/** @file  RMPLoader.cpp
 *  @brief Implementation of the Davilex 3D importer class
 */


#ifndef ASSIMP_BUILD_NO_RMP_IMPORTER

// internal headers
#include "RMPImporter.h"
#include "ConvertToLHProcess.h"
#include "StreamReader.h"
#include <memory>
#include <assimp/IOSystem.hpp>
#include <assimp/scene.h>
#include <assimp/DefaultLogger.hpp>
#include <assimp/importerdesc.h>

using namespace Assimp;

static const aiImporterDesc desc = {
    "RMP Importer",
    "",
    "",
    "",
    aiImporterFlags_SupportBinaryFlavour,
    0,
    0,
    0,
    0,
    "rmp"
};

// ------------------------------------------------------------------------------------------------
// Constructor to be privately used by Importer
RMPImporter::RMPImporter()
{}

// ------------------------------------------------------------------------------------------------
// Destructor, private as well
RMPImporter::~RMPImporter()
{}

// ------------------------------------------------------------------------------------------------
// Returns whether the class can handle the format of the given file.
bool RMPImporter::CanRead( const std::string& pFile, IOSystem* pIOHandler, bool checkSig) const
{
    const std::string extension = GetExtension(pFile);
    return extension == "rmp";
}

// ------------------------------------------------------------------------------------------------
const aiImporterDesc* RMPImporter::GetInfo () const
{
    return &desc;
}

// ------------------------------------------------------------------------------------------------
// Imports the given file into the given scene structure.
void RMPImporter::InternReadFile( const std::string& pFile,
    aiScene* pScene, IOSystem* pIOHandler)
{
    StreamReaderLE stream(pIOHandler->Open(pFile,"rb"));

    // We should have at least one chunk
    if (stream.GetRemainingSize() < 16) {
        throw DeadlyImportError("Davilex RMP file is either empty or corrupt: " + pFile);
    }

    long count = stream.GetI4();
    bool hasColors = false;
    if (count == -1) {
        count = stream.GetI4();
	hasColors = true;
    }
    long drawChunkCount = stream.GetI4();

    pScene->mNumMeshes = 4;
    pScene->mMeshes = new aiMesh*[4];
    for (long m = 0; m < 4; m++) {
        pScene->mMeshes[m] = new aiMesh();
    }
    pScene->mRootNode = new aiNode();
    pScene->mRootNode->mNumMeshes = 4;
    pScene->mRootNode->mMeshes = new unsigned int[4];
    pScene->mRootNode->mMeshes[0] = 0;
    pScene->mRootNode->mMeshes[1] = 1;
    pScene->mRootNode->mMeshes[2] = 2;
    pScene->mRootNode->mMeshes[3] = 3;

    std::vector<aiVector3D> vertices[4];
    std::vector<aiVector3D> uvs[4];
    std::vector<aiColor4D> colors[4];
    std::vector<unsigned int> indices[4];

    for (long c = 0; c < drawChunkCount; c++) {
        for (long p = 0; p < 4; p++) {
            long numQuads = stream.GetI4();
            for (long q = 0; q < numQuads; q++) {
                long vOff = vertices[p].size();
                for (long v = 0; v < 4; v++) {
                    aiVector3D vertex, uv;
		    aiColor4D color;

                    vertex.x = stream.GetF4();
                    vertex.y = stream.GetF4();
                    vertex.z = stream.GetF4();
		    vertices[p].push_back(vertex);

                    uv.x = stream.GetF4();
                    uv.y = stream.GetF4();
		    uvs[p].push_back(uv);

                    if (hasColors) {
                        unsigned long hex = stream.GetI4(); // color
		        color.b = ((hex >>  0) & 0xff) / 255.0;
		        color.r = ((hex >>  8) & 0xff) / 255.0;
		        color.g = ((hex >> 16) & 0xff) / 255.0;
			printf("%lx: %g,%g,%g\n", hex, color.r, color.g, color.b);
		        color.a = 1;
			colors[p].push_back(color);
	            }
		}
		// 2x triangle => 1 quad
                indices[p].push_back( vOff + stream.GetI4() );
                indices[p].push_back( vOff + stream.GetI4() );
                indices[p].push_back( vOff + stream.GetI4() );
                indices[p].push_back( vOff + stream.GetI4() );
                indices[p].push_back( vOff + stream.GetI4() );
                indices[p].push_back( vOff + stream.GetI4() );
	    }
        }
    }

    for (long m = 0; m < 4; m++) {
        aiMesh *mesh = pScene->mMeshes[m];
        mesh->mNumFaces = indices[m].size() / 3;
        mesh->mFaces = new aiFace[mesh->mNumFaces];
	mesh->mNumVertices = indices[m].size();
	mesh->mVertices = new aiVector3D[mesh->mNumVertices];
        mesh->mNumUVComponents[0] = 2;
        mesh->mTextureCoords[0] = new aiVector3D[mesh->mNumVertices];
        if (hasColors)
            mesh->mColors[0] = new aiColor4D[mesh->mNumVertices];

	long outV = 0;
        for (long f = 0; f < mesh->mNumFaces; f++) {
            mesh->mFaces[f].mNumIndices = 3;
            mesh->mFaces[f].mIndices = new unsigned int[3];
            mesh->mFaces[f].mIndices[0] = outV + 0;
            mesh->mFaces[f].mIndices[1] = outV + 1;
            mesh->mFaces[f].mIndices[2] = outV + 2;

            unsigned int a, b, c;
	    a = indices[m][f*3 + 0];
	    b = indices[m][f*3 + 1];
	    c = indices[m][f*3 + 2];
            mesh->mVertices[outV + 0] = vertices[m][a];
            mesh->mVertices[outV + 1] = vertices[m][b];
            mesh->mVertices[outV + 2] = vertices[m][c];
            mesh->mTextureCoords[0][outV + 0] = uvs[m][a];
            mesh->mTextureCoords[0][outV + 1] = uvs[m][b];
            mesh->mTextureCoords[0][outV + 2] = uvs[m][c];
            if (hasColors) {
                mesh->mColors[0][outV + 0] = colors[m][a];
                mesh->mColors[0][outV + 1] = colors[m][b];
                mesh->mColors[0][outV + 2] = colors[m][c];
            }
            outV += 3;
	}
    }

    // Convert everything to OpenGL space... it's the same operation as the conversion back, so we can reuse the step directly
    MakeLeftHandedProcess convertProcess;
    convertProcess.Execute(pScene);

    FlipWindingOrderProcess flipper;
    flipper.Execute(pScene);
}

#endif // ASSIMP_BUILD_NO_RMP_IMPORTER
