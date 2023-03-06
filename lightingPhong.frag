/////////////////////////////////////////////////////////////////////////
// Pixel shader for lighting
////////////////////////////////////////////////////////////////////////
#version 430

//out vec4 FragColor;

// These definitions agree with the ObjectIds enum in scene.h
//const int     nullId	= 0;
//const int     skyId	= 1;
//const int     seaId	= 2;
//const int     groundId	= 3;
//const int     roomId	= 4;
//const int     boxId	= 5;
//const int     frameId	= 6;
//const int     lPicId	= 7;
//const int     rPicId	= 8;
//const int     teapotId	= 9;
//const int     spheresId	= 10;
//const int     floorId	= 11;
//
//float pi = 3.14159;
//float pi2 = 2*pi;

in vec2 texCoord;
uniform int objectId;


in vec3 normalVec, worldPos;
uniform vec3 diffuse;
uniform vec3 specular;
uniform float shininess;

out vec4 FragData[];

void main()
{   
    FragData[0].xyz = worldPos;
    FragData[1].xyz = normalVec;
    FragData[2].xyz = diffuse;
    FragData[3] = vec4(specular, shininess);

    //FragColor.xyz = N;

}
