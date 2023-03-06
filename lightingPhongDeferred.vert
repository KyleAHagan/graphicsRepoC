/////////////////////////////////////////////////////////////////////////
// Vertex shader for lighting
//
// Copyright 2013 DigiPen Institute of Technology
////////////////////////////////////////////////////////////////////////
#version 330

uniform mat4 WorldView, WorldProj, ModelTr, shadowMatrix;

in vec4 vertex;

layout (location = 0) in vec2 aPos;

out vec4 shadowCoord;

void main()
{
    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);
    shadowCoord = shadowMatrix * ModelTr * vertex;
}
