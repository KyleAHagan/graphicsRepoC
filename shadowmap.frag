///////////////////////////////////////////////////////////////////////////
//// Pixel shader for lighting
////
//// Copyright 2013 DigiPen Institute of Technology
//////////////////////////////////////////////////////////////////////////
//#version 330
//
//
//
//in vec4 position;
//
////out vec4 FragColor;
//
////DEBUGout vec4 FragData[];
//
//out vec4 fragColor;
//
//void main()
//{
//	//gl_FragData[0] = vec4(position.w);
//	//FragData[0] = position;// debug extra data version of previous statement.
//	//FragData[1] = position;
//	//FragData[2] = position;
//	//FragData[3] = position;
//	fragColor = vec4(position.w)/100;
//
//	fragColor = vec4(0.5f, 0.0f, 1.0f,0.8f);
//}
//

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

//out vec4 FragColor;

in vec4 shadowPosition;

void main()
{   

    //FragColor.xyz = vec3(shadowPosition.w)/100;
    //FragData[0]= vec4(shadowPosition.w)/100;
    FragData[0]= shadowPosition;
    FragData[1]= shadowPosition;
    FragData[2]= shadowPosition;
    FragData[3]= shadowPosition;
}
