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

/** @file  RWXParser.h
 *  @brief Declaration of RWX parser class
 */

#include <vector>
#include <string>
#include <map>

#include <assimp/types.h>
#include <assimp/mesh.h>


class RWXParser {
public:
    RWXParser(const char* buffer);
    ~RWXParser();

    void Parse();
private:
    const char *buffer;

    struct Material {
        aiString textureBump, textureDiffuse;
        ai_real ambient, diffuse, specular;
        aiColor4D color;

	void clear() {
            textureBump = textureDiffuse = "";
	    ambient = diffuse = specular = 1;
	    color = aiColor4D(1.0);
	}
    };
    struct Mesh {
        std::vector<aiFace> _faces;
        std::vector<aiVector3D> _vertices;
        std::vector<aiVector3D> _uvs;
	Material mat;

	void clear() {
            _faces.clear();
	    _vertices.clear();
	    _uvs.clear();
	    mat.clear();
	}
    } m;

    struct State {
        // state for matrix operations
        std::vector<aiMatrix4x4> matrixstack;
        aiMatrix4x4 currentMatrix;

        // vertex command storage
        std::vector<aiVector3D> vertices;
        std::vector<aiVector3D> uvs;
	std::vector<aiFace> faces;

        // Material state
        std::vector<Material> matstack;
        Material mat;

        void clear() {
            matrixstack.clear();
	    currentMatrix = aiMatrix4x4();

	    vertices.clear();
	    uvs.clear();

	    matstack.clear();
            mat.clear();
        }
    } s;
    std::vector<State> statestack;
    std::vector<Mesh> object;
    std::vector<Mesh> output;

    // proto state
    std::string protoBeingCreated;
    std::map<std::string,std::vector<Mesh>> protos;

    void applyProto(const std::vector<Mesh>& proto);
    void addFace(int count, unsigned int indices[]);
    void flushFaces();
};
