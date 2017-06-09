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
    if (count == 0xffffffff) {
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

    for (long c = 0; c < drawChunkCount; c++) {
      for (long p = 0; p < 4; p++) {
          aiMesh* mesh = pScene->mMeshes[p];
          long numQuads = stream.GetI4();
          for (long q = 0; q < numQuads; q++) {
              stream.GetF4(); // PX
              stream.GetF4(); // PY
              stream.GetF4(); // PZ
              stream.GetF4(); // U
              stream.GetF4(); // V
              if (hasColors) {
                  stream.GetI4(); // color
	      }
              stream.GetI4(); // A1
              stream.GetI4(); // B1
              stream.GetI4(); // C1
              stream.GetI4(); // A2
              stream.GetI4(); // B2
              stream.GetI4(); // C2
	  }
      }
    }

    aiMesh *mesh = new aiMesh;
    mesh->mNumVertices = verticeCount;
    mesh->mVertices = new aiVector3D[verticeCount];
    mesh->mNormals = new aiVector3D[verticeCount];
    mesh->mNumUVComponents[0] = 2;
    mesh->mTextureCoords[0] = new aiVector3D[verticeCount];
    if (hasColors)
      mesh->mColors[0] = new aiColor4D[verticeCount];

    for (long v = 0; v < verticeCount; v++) {
        mesh->mVertices[v].x = stream.GetF4();
        mesh->mVertices[v].y = stream.GetF4();
        mesh->mVertices[v].z = stream.GetF4();

        mesh->mNormals[v].x = stream.GetF4();
        mesh->mNormals[v].y = stream.GetF4();
        mesh->mNormals[v].z = stream.GetF4();

        mesh->mTextureCoords[0][v].x = stream.GetF4();
        mesh->mTextureCoords[0][v].y = stream.GetF4();

        if (hasColors) {
          mesh->mColors[0][v].r = stream.GetF4();
          mesh->mColors[0][v].g = stream.GetF4();
          mesh->mColors[0][v].b = stream.GetF4();
          mesh->mColors[0][v].a = stream.GetF4();
	}

        stream.GetI1(); // flags?
    }

    mesh->mNumFaces = faceCount;
    mesh->mFaces = new aiFace[faceCount];

    for (long t = 0; t < faceCount; t++) {
        mesh->mFaces[t].mNumIndices = 3;
        mesh->mFaces[t].mIndices = new unsigned int[3];

        mesh->mFaces[t].mIndices[0] = stream.GetI2();
        mesh->mFaces[t].mIndices[1] = stream.GetI2();
        mesh->mFaces[t].mIndices[2] = stream.GetI2();

        stream.GetI2(); // D?
        stream.GetI1(); // flags?
    }
}

#endif // ASSIMP_BUILD_NO_RMP_IMPORTER
