/////////////////////////////////////////////////////////////////////////
// Pixel shader for lighting
////////////////////////////////////////////////////////////////////////
#version 430

out vec4 FragColor;

// These definitions agree with the ObjectIds enum in scene.h
const int     nullId	= 0;
const int     skyId	= 1;
const int     seaId	= 2;
const int     groundId	= 3;
const int     roomId	= 4;
const int     boxId	= 5;
const int     frameId	= 6;
const int     lPicId	= 7;
const int     rPicId	= 8;
const int     teapotId	= 9;
const int     spheresId	= 10;
const int     floorId	= 11;

float pi = 3.14159;
float pi2 = 2*pi;


in vec2 texCoord;

uniform int objectId;

uniform mat4 WorldInverse;
uniform vec3 lightPos;

uniform sampler2D g0;
uniform sampler2D g1;
uniform sampler2D g2;
uniform sampler2D g3;

uniform sampler2D shadowMap;

in vec4 shadowCoord;
vec2 shadowIndex;

float lightDepth;
float pixelDepth;

uniform vec2 WindowSize;

void main()
{
    //FragData[0].xyz = worldPos;
    //FragData[1].xyz = N;
    //FragData[2].xyz = Kd;
    //FragData[3] = vec4(specular, shininess);

//vec2 uv = gl_FragCoord.xy/WindowSize.xy;
//FragColor = texture(shadowMap, uv);

    vec2 xy = gl_FragCoord.xy/WindowSize.xy;
    vec3 worldPos = texture2D(g0, xy).xyz;
    vec3 normalVec = texture2D(g1, xy).xyz; 
    vec3 diffuse =  texture2D(g2, xy).xyz;
    vec3 specular = texture2D(g3, xy).xyz;
    float shininess = texture2D(g3, xy).w;

    vec3 eyePos = (WorldInverse*vec4(0,0,0,1)).xyz;
    vec3 lightVec = lightPos - worldPos;
    vec3 eyeVec = eyePos - worldPos;


    vec3 ONE = vec3(1.0, 1.0, 1.0);
    vec3 N = normalize(normalVec);
    vec3 L = normalize(lightVec);
    vec3 V = normalize(eyeVec);
    vec3 H = normalize(L+V);
    float NL = max(dot(N,L),0.0);
    float NV = max(dot(N,V),0.0);
    float HN = max(dot(H,N),0.0);

    vec3 I = vec3(0.3, 0.3, 0.3);
    vec3 Ia = 0.1*ONE;
    vec3 Kd = diffuse; 
   
    vec3 fragColor;
    if (shadowCoord.w > 0)
    {
        shadowIndex = shadowCoord.xy/shadowCoord.w;
    }
    if (shadowCoord.w > 0 && shadowIndex.x >= 0 && shadowIndex.x <= 1 && shadowIndex.y >= 0 && shadowIndex.y <= 1)
    {
        lightDepth = texture2D(shadowMap, shadowIndex).w;
        pixelDepth = shadowCoord.w;
    }
    if(pixelDepth > lightDepth + 0.01)
    {
        fragColor.xyz = Ia * Kd;
    }
    else
    {
         //Lighting is diffuse + ambient + specular
        fragColor = Ia*Kd;
        fragColor += I*Kd*NL;
        fragColor += I*specular*pow(HN,shininess);
        //fragColor.xyz = vec3(shadowCoord.w/100);//debug test
    }
//
//    //testing lighting
//    //fragColor = abs(N);
//    //fragColor = Kd;
//    //fragColor = specular;
//    //fragColor = worldPos/10.0;
//
       FragColor.xyz = fragColor;
//
   //FragColor.xyz = vec3(texture2D(shadowMap, shadowIndex).w/100);
//   FragColor.xyz = vec3(shadowIndex.x, 0.0, shadowIndex.y);
}



///////////////////////////////////////////////////////////////////////////
//// Pixel shader for lighting
//////////////////////////////////////////////////////////////////////////
//#version 430
//
//out vec4 FragColor;
//
//// These definitions agree with the ObjectIds enum in scene.h
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
//
//
//in vec2 texCoord;
//
//uniform int objectId;
//
//uniform mat4 WorldInverse;
//uniform vec3 lightPos;
//
//uniform sampler2D g0;
//uniform sampler2D g1;
//uniform sampler2D g2;
//uniform sampler2D g3;
//
//uniform sampler2D shadowMap;
//
//in vec4 shadowCoord;
//vec2 shadowIndex;
//
//float lightDepth;
//float pixelDepth;
//
//uniform vec2 WindowSize;
//
//void main()
//{
////    FragData[0].xyz = worldPos;
////    FragData[1].xyz = N;
////    FragData[2].xyz = Kd;
////    FragData[3] = vec4(specular, shininess);
//
//
//    vec2 xy = gl_FragCoord.xy/WindowSize.xy;
//    vec3 worldPos = texture2D(g0, xy).xyz;
//    vec3 normalVec = texture2D(g1, xy).xyz; 
//    vec3 diffuse =  texture2D(g2, xy).xyz;
//    vec3 specular = texture2D(g3, xy).xyz;
//    float shininess = texture2D(g3, xy).w;
//
//    vec3 eyePos = (WorldInverse*vec4(0,0,0,1)).xyz;
//    vec3 lightVec = lightPos - worldPos;
//    vec3 eyeVec = eyePos - worldPos;
//
//
//    vec3 ONE = vec3(1.0, 1.0, 1.0);
//    vec3 N = normalize(normalVec);
//    vec3 L = normalize(lightVec);
//    vec3 V = normalize(eyeVec);
//    vec3 H = normalize(L+V);
//    float NL = max(dot(N,L),0.0);
//    float NV = max(dot(N,V),0.0);
//    float HN = max(dot(H,N),0.0);
//    float LH = max(dot(L,H),0.0);
//    float LN = max(dot(L,N),0.0);
//
//    vec3 I = vec3(4, 4, 4);
//    vec3 Ia = 0.15*ONE;
//    vec3 Kd = diffuse; 
//   
//    if (shadowCoord.w > 0)
//    {
//        shadowIndex = shadowCoord.xy/shadowCoord.w;
//    }
//    if (shadowCoord.w > 0 && shadowIndex.x >= 0 && shadowIndex.x <= 1 && shadowIndex.y >= 0 && shadowIndex.y <= 1)
//    {
//        lightDepth = texture2D(shadowMap, shadowIndex).w;
//        pixelDepth = shadowCoord.w;
//    }
//    if(pixelDepth > lightDepth + 0.01)
//    {
//        //FragColor.xyz = Ia * diffuse;
//    }
//    else
//    {
//        vec3 N = normalize(normalVec);
//        vec3 L = normalize(lightVec);
//        vec3 V = normalize(eyeVec);
//
//        vec3 H = normalize(L+V);
//        float LN = max(dot(L,N),0.0);
//        float HN = max(dot(H,N),0.0);
//        float LH = max(dot(L,H),0.0);
//
//        vec3 Kd = diffuse;   
//    
//        //fragColor.xyz = vec3(0.5,0.5,0.5)*Kd + Kd*max(dot(L,N),0.0);
//
//        vec3 FLH = specular + ((1,1,1) - specular) * pow((1-LH),5);
//        float GLVH = pow(LH,2);
//        float DH = (shininess + 2)/(2 * 3.14159) * pow(HN,shininess);
//        vec3 BRDF = Kd/3.14159 + (FLH * DH)/(4 * GLVH);
//        //FragColor.xyz = Ia * Kd + I*LN * BRDF;
//
//
//        //debug shadow
//        //FragColor = texture2D(shadowMap, shadowIndex);
//        FragColor.rg = shadowIndex;
//    }
//
//
//
//    //testing lighting
//    //fragColor = abs(N);
//    //fragColor = Kd;
//    //fragColor = specular;
//    //fragColor = worldPos/10.0;
//}
