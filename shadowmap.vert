///////////////////////////////////////////////////////////////////////////
//// Vertex shader for lighting
////
//// Copyright 2013 DigiPen Institute of Technology
//////////////////////////////////////////////////////////////////////////
//#version 430
//
//
//
//uniform mat4 shadowProj, shadowView, modelTr;
//
//in vec4 vertex;
//
//out vec4 position;
//
//void main()
//{     
//    gl_Position = shadowProj * shadowView * modelTr * vertex;
//    position = gl_Position;
//}


/////////////////////////////////////////////////////////////////////////
// Vertex shader for lighting
//
// Copyright 2013 DigiPen Institute of Technology
////////////////////////////////////////////////////////////////////////
#version 430

uniform mat4 WorldView, WorldInverse, WorldProj, ModelTr, NormalTr,shadowProj, shadowView, modelTr;

in vec4 vertex;
in vec3 vertexNormal;
in vec2 vertexTexture;
in vec3 vertexTangent;

out vec3 normalVec, worldPos;
out vec2 texCoord;
uniform vec3 lightPos;

out vec4 shadowPosition;

void main()
{
    vec3 eye = (WorldInverse*vec4(0,0,0,1)).xyz;

    gl_Position = shadowProj * shadowView*ModelTr*vertex;
    
    //    gl_Position = shadowProj * shadowView * modelTr * vertex;
    shadowPosition = shadowProj * shadowView*ModelTr*vertex;

    worldPos = (ModelTr*vertex).xyz;

    normalVec = vertexNormal*mat3(NormalTr); 

    texCoord = vertexTexture;
}