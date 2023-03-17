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

    shadowPosition = gl_Position;

    worldPos = (ModelTr*vertex).xyz;

    normalVec = vertexNormal*mat3(NormalTr); 

    texCoord = vertexTexture;
}