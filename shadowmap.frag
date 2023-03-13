
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

in vec4 shadowPosition;

void main()
{   

    //FragColor.xyz = vec3(shadowPosition.w)/100;
    //FragData[0]= vec4(shadowPosition.w)/100;
    FragData[0].x = shadowPosition.w;
    FragData[0].y = shadowPosition.w * shadowPosition.w;
    FragData[0].z = shadowPosition.w * shadowPosition.w  * shadowPosition.w;
    FragData[0].w = shadowPosition.w * shadowPosition.w  * shadowPosition.w  * shadowPosition.w;

    FragData[1]= shadowPosition;
    FragData[2]= shadowPosition;
    FragData[3]= shadowPosition;
}
