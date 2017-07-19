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

/** @file  RWXParser.cpp
 *  @brief Implementation of RWX parser class
 */


#include "RWXParser.h"

#include "ParsingUtils.h"
#include "fast_atof.h"

using namespace Assimp;

RWXParser::RWXParser(const char *buffer) {
}

RWXParser::~RWXParser() {
}


void
RWXParser::flushFaces() {
    if (m._faces.size())
        object.push_back( m );
}

void
RWXParser::addFace(int count, unsigned int *indices) {
    aiFace face;
    face.mNumIndices = count;
    face.mIndices = new unsigned int[count];

    for (int c = 0; c < count; c++) {
        int index = indices[c];
        aiVector3D pt = s.vertices[index];
        aiVector3D uv = s.vertices[index];
        pt *= s.currentMatrix;
        face.mIndices[c] = m._vertices.size();
        m._vertices.push_back( pt );
        m._uvs.push_back( uv );
    }
    m._faces.push_back( face );
}

void
RWXParser::applyProto(const std::vector<Mesh>& proto)
{
}

void
RWXParser::Parse() {
    char line[4096];
    std::string token;
    unsigned long lineno = 0;
    while(GetNextLine(buffer,line)) {
        const char* sz = line;
        lineno++;
        // If line is empty or we hit '#' skip line
        if (!SkipSpacesAndLineEnd(&sz) || *sz == '#')
            continue;
        printf("%lu: %s\n", lineno, line);
        if (TokenMatchI(sz, "addmaterialmode", 15)) {
            std::string value = GetNextToken(sz);
            // XXX assert( value == "double" );
        } else if (TokenMatchI(sz, "addtexturemode", 14)) {
            // mode can be lit (=default), foreshorten, or filter
	    // AW v3.0+ only supports lit
        } else if (TokenMatchI(sz, "ambient", 7)) {
            ai_real c;
            sz = fast_atoreal_move<ai_real>(sz,c); SkipSpaces(&sz);
            s.mat.ambient = c;
        } else if (TokenMatchI(sz, "axisalignment", 13)) {
            // zorientx, zorienty or none
            std::string mode = GetNextToken(sz);
        } else if (TokenMatchI(sz, "block", 5)) {
            ai_real width, height, depth;
            sz = fast_atoreal_move<ai_real>(sz,width);  SkipSpaces(&sz);
            sz = fast_atoreal_move<ai_real>(sz,height); SkipSpaces(&sz);
            sz = fast_atoreal_move<ai_real>(sz,depth);  SkipSpaces(&sz);
	    // TODO create block
        } else if (TokenMatchI(sz, "clumpbegin", 10)) {
            // XXX: create new state, push current on stack
        } else if (TokenMatchI(sz, "clumpend", 8)) {
            // XXX: store current state, pop previous from stack
        } else if (TokenMatchI(sz, "collision", 9)) {
            std::string value = GetNextToken(sz);
            // XXX assert( value == "on" || value == "off" );
        } else if (TokenMatchI(sz, "color", 5)) {
            ai_real r, g, b;
            sz = fast_atoreal_move<ai_real>(sz,r); SkipSpaces(&sz);
            sz = fast_atoreal_move<ai_real>(sz,g); SkipSpaces(&sz);
            sz = fast_atoreal_move<ai_real>(sz,b); SkipSpaces(&sz);
            s.mat.color = aiColor4D(r,g,b, 1);
        } else if (TokenMatchI(sz, "cone", 4)) {
            // height radius numsides
        } else if (TokenMatchI(sz, "cylinder", 8)) {
            // height bottomradius topradius numsides
        } else if (TokenMatchI(sz, "diffuse", 7)) {
            ai_real c;
            sz = fast_atoreal_move<ai_real>(sz,c); SkipSpaces(&sz);
            s.mat.diffuse = c;
        } else if (TokenMatchI(sz, "disc", 4)) {
            // vertical displacement, radius, numsides
        } else if (TokenMatchI(sz, "geometrysampling", 16)) {
          std::string mode = GetNextToken(sz);
          // solid, wireframe, pointcloud
	  // XXX handle wireframe?
        } else if (TokenMatchI(sz, "hemisphere", 10)) {
            ai_real radius, density;
            sz = fast_atoreal_move<ai_real>(sz,radius);  SkipSpaces(&sz);
            sz = fast_atoreal_move<ai_real>(sz,density); SkipSpaces(&sz);
            // TODO hemisphere
        } else if (TokenMatchI(sz, "identity", 8)) {
            s.currentMatrix = aiMatrix4x4();
        } else if (TokenMatchI(sz, "lightsampling", 13)) {
            std::string mode = GetNextToken(sz);
            // facet (default) or vertex
        } else if (TokenMatchI(sz, "materialbegin", 13)) {
            // push material on stack
        } else if (TokenMatchI(sz, "materialmode", 12) || TokenMatchI(sz, "materialmodes", 13)) {
            std::string value = GetNextToken(sz);
	    // assert(value == "double" || value == "NULL");
        } else if (TokenMatchI(sz, "materialend", 11)) {
            // pop material from stack
        } else if (TokenMatchI(sz, "modelbegin", 10)) {
            // no arguments
        } else if (TokenMatchI(sz, "modelend", 8)) {
            // no arguments
        } else if (TokenMatchI(sz, "opacity", 7)) {
            ai_real opacity;
            sz = fast_atoreal_move<ai_real>(sz,opacity); SkipSpaces(&sz);
	    // 0.0 .. 1.0
        } else if (TokenMatchI(sz, "polygon", 7)) {
            unsigned long n = strtoul10(sz,&sz); SkipSpaces(&sz);
            unsigned int indices[n];
            for (unsigned long i = 0; i < n; i++) {
                indices[i] = (unsigned int)strtoul10(sz,&sz); SkipSpaces(&sz);
            }
            if (TokenMatchI(sz, "tag", 3)) {
                unsigned long tag;
                tag = strtoul10(sz,&sz); SkipSpaces(&sz);
                // XXX store tag as material?
            }
            addFace( n, indices );
        } else if (TokenMatchI(sz, "protobegin", 10)) {
            flushFaces();
	    if (object.size()) {
               output.insert(output.end(), object.begin(), object.end());
	       object.clear();
	    }
	    // flush to object, store object in output
            protoBeingCreated = GetNextToken(sz);
	    statestack.push_back(s);
	    s.clear();
	} else if (TokenMatchI(sz, "protoend", 8)) {
            protos[protoBeingCreated] = object;
	    s = statestack.back();
	    statestack.pop_back();
	    object.clear();
        } else if (TokenMatchI(sz, "protoinstance", 13)) {
            std::string name = GetNextToken(sz);
	    applyProto( protos[name] );
        } else if (TokenMatchI(sz, "protoinstancegeometry", 21)) {
            std::string name = GetNextToken(sz);
        } else if (TokenMatchI(sz, "quad", 4)) {
            // Quad v1 v2 v3 v4 [UV u v] [Tag value]
            unsigned int indices[4];
            indices[0] = strtoul10(sz,&sz); SkipSpaces(&sz);
            indices[1] = strtoul10(sz,&sz); SkipSpaces(&sz);
            indices[2] = strtoul10(sz,&sz); SkipSpaces(&sz);
            indices[3] = strtoul10(sz,&sz); SkipSpaces(&sz);
            if (TokenMatchI(sz, "uv", 2)) {
                ai_real u, v;
                sz = fast_atoreal_move<ai_real>(sz,u); SkipSpaces(&sz);
                sz = fast_atoreal_move<ai_real>(sz,v); SkipSpaces(&sz);
                // TODO uv for quads
            }
            if (TokenMatchI(sz, "tag", 3)) {
                unsigned long tag;
                tag = strtoul10(sz,&sz); SkipSpaces(&sz);
                // XXX store tag as material?
            }
            addFace(4, indices);
        } else if (TokenMatchI(sz, "removematerialmode", 18)) {
            std::string mode = GetNextToken(sz);
            // assert(mode == "double");
        } else if (TokenMatchI(sz, "removetexturemode", 17)) {
            std::string mode = GetNextToken(sz);
            // lit, foreshorten, or filter
        } else if (TokenMatchI(sz, "rotate", 6)) {
            // x y z [0|1] r [float]
	    unsigned long x, y, z;
            x = strtoul10(sz,&sz); SkipSpaces(&sz);
            y = strtoul10(sz,&sz); SkipSpaces(&sz);
            z = strtoul10(sz,&sz); SkipSpaces(&sz);
            ai_real r;
            sz = fast_atoreal_move<ai_real>(sz,r); SkipSpaces(&sz);
	    aiMatrix4x4 rot;
	    ai_real rad = AI_DEG_TO_RAD(r);
            if (x) {
	        aiMatrix4x4::RotationX(rad, rot);
	        s.currentMatrix *= rot;
	    }
	    if (y) {
                aiMatrix4x4::RotationY(rad, rot);
	        s.currentMatrix *= rot;
	    }
	    if (z) {
                aiMatrix4x4::RotationZ(rad, rot);
		s.currentMatrix *= rot;
	    }
        } else if (TokenMatchI(sz, "scale", 5)) {
            aiVector3D v;
            sz = fast_atoreal_move<ai_real>(sz,v.x); SkipSpaces(&sz);
            sz = fast_atoreal_move<ai_real>(sz,v.y); SkipSpaces(&sz);
            sz = fast_atoreal_move<ai_real>(sz,v.z); SkipSpaces(&sz);
	    aiMatrix4x4 matrix;
	    aiMatrix4x4::Scaling(v, matrix);
	    s.currentMatrix *= matrix;
        } else if (TokenMatchI(sz, "specular", 8)) {
            ai_real c;
            sz = fast_atoreal_move<ai_real>(sz,c); SkipSpaces(&sz);
            s.mat.specular = c;
        } else if (TokenMatchI(sz, "sphere", 6)) {
            ai_real radius, density;
            sz = fast_atoreal_move<ai_real>(sz,radius); SkipSpaces(&sz);
            sz = fast_atoreal_move<ai_real>(sz,density); SkipSpaces(&sz);
        } else if (TokenMatchI(sz, "surface", 7)) {
            // ambient diffuse specular
            ai_real a, d, sf;
            sz = fast_atoreal_move<ai_real>(sz,a); SkipSpaces(&sz);
            sz = fast_atoreal_move<ai_real>(sz,d); SkipSpaces(&sz);
            sz = fast_atoreal_move<ai_real>(sz,sf); SkipSpaces(&sz);
            s.mat.ambient = a; s.mat.diffuse = d; s.mat.specular = sf;
        } else if (TokenMatchI(sz, "tag", 3)) {
            // tag for limb (bone)
            strtoul10(sz,&sz); SkipSpaces(&sz);
        } else if (TokenMatchI(sz, "texture", 7)) {
            s.mat.textureDiffuse = (GetNextToken(sz) + ".jpg").c_str();
            printf("texture %s", s.mat.textureDiffuse.C_Str());
            if (TokenMatchI(sz, "mask", 4)) {
                printf(" mask %s", GetNextToken(sz).c_str());
            } else if (TokenMatchI(sz, "bump", 4)) {
                s.mat.textureBump = GetNextToken(sz).c_str();
                printf(" bump %s", s.mat.textureBump.C_Str());
            }
            printf("\n");
        } else if (TokenMatchI(sz, "textureaddressmode", 18)) {
            std::string mode = GetNextToken(sz);
            // wrap, mirror, clamp
        } else if (TokenMatchI(sz, "texturemipmapstate", 18)) {
            std::string mode = GetNextToken(sz);
	    // assert(mode == "off" || mode == "on");
        } else if (TokenMatchI(sz, "texturemode", 11) || TokenMatchI(sz, "texturemodes", 12)) {
            // lit (default), foreshorten, or filter (or NULL)
            std::string mode = GetNextToken(sz).c_str();
        } else if (TokenMatchI(sz, "transform", 9)) {
            ai_real m[16];
            for (short i = 0; i < 16; i++) {
              sz = fast_atoreal_move<ai_real>(sz,m[i]); SkipSpaces(&sz);
	    }
	    aiMatrix4x4 matrix( m[0],  m[1],  m[2],  m[3],
                                m[4],  m[5],  m[6],  m[7],
				m[8],  m[9],  m[10], m[11],
				m[12], m[13], m[14], m[15] );
            s.currentMatrix = matrix;
        } else if (TokenMatchI(sz, "transformbegin", 14)) {
            s.matrixstack.push_back(s.currentMatrix);
        } else if (TokenMatchI(sz, "transformend", 12)) {
            s.currentMatrix = s.matrixstack.back();
            s.matrixstack.pop_back();
        } else if (TokenMatchI(sz, "translate", 9)) {
            aiVector3D v;
            sz = fast_atoreal_move<ai_real>(sz,v.x); SkipSpaces(&sz);
            sz = fast_atoreal_move<ai_real>(sz,v.y); SkipSpaces(&sz);
            sz = fast_atoreal_move<ai_real>(sz,v.z); SkipSpaces(&sz);
	    aiMatrix4x4 matrix;
	    aiMatrix4x4::Translation(v, matrix);
	    s.currentMatrix *= matrix;
        } else if (TokenMatchI(sz, "vertex", 6)) {
            aiVector3D v; SkipSpaces(&sz);
            sz = fast_atoreal_move<ai_real>(sz,v.x); SkipSpaces(&sz);
            sz = fast_atoreal_move<ai_real>(sz,v.y); SkipSpaces(&sz);
            sz = fast_atoreal_move<ai_real>(sz,v.z); SkipSpaces(&sz);
            s.vertices.push_back(v);
            if (TokenMatchI(sz, "uv", 2)) {
                aiVector3D uv; SkipSpaces(&sz);
                sz = fast_atoreal_move<ai_real>(sz,uv.x); SkipSpaces(&sz);
                sz = fast_atoreal_move<ai_real>(sz,uv.y); SkipSpaces(&sz);
                s.uvs.push_back(uv);
            }
            // prelight extension?
        } else if (TokenMatchI(sz, "triangle", 7)) {
            unsigned int indices[3];
            indices[0] = strtoul10(sz,&sz); SkipSpaces(&sz);
            indices[1] = strtoul10(sz,&sz); SkipSpaces(&sz);
            indices[2] = strtoul10(sz,&sz); SkipSpaces(&sz);
            if (TokenMatchI(sz, "tag", 3)) {
                /*unsigned int tag = */strtoul10(sz,&sz); SkipSpaces(&sz);
                // XXX store tag as material?
            }
            addFace(3, indices);
            // Not supported by ActiveWorlds browsers....
        } else if (TokenMatchI(sz, "hints", 5)) {
        } else if (TokenMatchI(sz, "addhint", 7)) {
        } else if (TokenMatchI(sz, "include", 7)) {
        } else if (TokenMatchI(sz, "includegeometry", 15)) {
        } else if (TokenMatchI(sz, "removehint", 10)) {
        } else if (TokenMatchI(sz, "texturedithering", 16)) {
        } else if (TokenMatchI(sz, "texturegammacorrection", 22)) {
        } else if (TokenMatchI(sz, "trace", 5)) {
        } else if (TokenMatchI(sz, "transformjoint", 14)) {
        } else {
            printf("Unknown token '%s'!\n", GetNextToken(sz).c_str());
        }
    }
}

