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

vec4 shadowCoord;
vec2 shadowIndex;

float lightDepth;
float pixelDepth;
float pixelDepthSquared;

uniform vec2 WindowSize;

uniform mat4 shadowMatrix;

uniform vec2 randXY;

void main()
{
    vec2 xy = gl_FragCoord.xy/WindowSize.xy;
    vec3 worldPos = texture2D(g0, xy).xyz;
    vec3 normalVec = texture2D(g1, xy).xyz; 
    vec3 diffuse =  texture2D(g2, xy).xyz;
    vec3 specular = texture2D(g3, xy).xyz;
    float shininess = texture2D(g3, xy).w;

    vec3 eyePos = (WorldInverse*vec4(0,0,0,1)).xyz;
    vec3 lightVec = lightPos - worldPos;
    vec3 eyeVec = eyePos - worldPos;

    shadowCoord = shadowMatrix * vec4(worldPos,1);


    vec3 ONE = vec3(1.0, 1.0, 1.0);

    vec3 I = vec3(2.0, 2.0, 2.0);
    vec3 Ia = 0.1*ONE;
    vec3 Kd = diffuse; 
   
    vec4 Bvalues;
    float alpha = 0.000001;//0.001 in teacher's version

    vec3 fragColor;
    if (shadowCoord.w > 0)
    {
        shadowIndex = shadowCoord.xy/shadowCoord.w;
    }


    float probabilityInShadow = 0;
    if (shadowCoord.w > 0 && shadowIndex.x >= 0 && shadowIndex.x <= 1 && shadowIndex.y >= 0 && shadowIndex.y <= 1)
    {
        //lightDepth = texture2D(shadowMap, shadowIndex).x;
        pixelDepth = ((shadowCoord.w - 20)/(150-20));
        pixelDepthSquared = pixelDepth * pixelDepth;
        Bvalues = texture2D(shadowMap, shadowIndex); //debugging the moment shadow map


        Bvalues = (1-alpha)*Bvalues + (alpha * 0.5);

        float a = 1;//In our case a is always 1. max(sqrt(1),0.0001)
        float b = Bvalues.x;//In our case a is always 1. (Bvalues.x)/a;
        float c = Bvalues.y;//In our case a is always 1. (Bvalues.y)/a;
        float d = max(sqrt((Bvalues.y)-(b*b)),0.0001);
        float e = ((Bvalues.z) - (b *c))/d;
        float f = max(sqrt((Bvalues.w) - (c*c) - (e*e)),0.0001);

        float c1hat = 1;//In our case z1 and a are always 1. z1/a
        float c2hat = (pixelDepth - b * c1hat)/d;
        float c3hat = (pixelDepthSquared - c * c1hat - e * c2hat)/f;

        float c3 = c3hat/f;
        float c2 = (c2hat - e * c3)/d;
        float c1 = (c1hat - b * c2 - c * c3)/a;

        //quadratic: (-b +-sqrt(b^2 -4ac)/2a)
        float z2 = (-c2 + sqrt((c2*c2) - (4 *c3 * c1)))/(2 * c3);
        float z3 = (-c2 - sqrt((c2*c2) - (4 *c3 * c1)))/(2 * c3);
        if(z2>z3)
        {
            float temp = z3;
            z3 = z2;
            z2 = temp;
        }
        if(pixelDepth <= z2)
        {
            probabilityInShadow = 0.0;
        }
        else if(pixelDepth <= z3)
        {
            probabilityInShadow = (pixelDepth * z3 - Bvalues.x * (pixelDepth + z3) + Bvalues.y)/((z3 - z2) * (pixelDepth - z2));
            //probabilityInShadow = 0.8;
//            if(isnan(c2))
//        {
//            probabilityInShadow = 0.5;
//        }
        }
        else
        {
            probabilityInShadow = 1 - (((z2 * z3) - (Bvalues.x * (z2 + z3)) + Bvalues.y)/((pixelDepth - z2) * (pixelDepth - z3)));
            //probabilityInShadow = 0;
//        if(isnan(c2))
//        {
//            probabilityInShadow = 0.5;
//        }
        }

        if(probabilityInShadow < 0) //debug
        {
            probabilityInShadow = 0;
        }
        if(probabilityInShadow > 1)
        {
            probabilityInShadow = 1;
        }
//        if(isnan(probabilityInShadow))
//        {
//            probabilityInShadow = 0.5;
//        }
//        probabilityInShadow = 0.9;
//        if(((z3 - z2) * (pixelDepth - z2)) == 0)
//        {
//        probabilityInShadow = 0.1;
//
//
//        }
//
//        if(isnan(z2) ||isnan(z3) || isnan(c1) || isnan(c2) || isnan(c3) || isnan(c1hat) || isnan(c2hat) || isnan(c3hat))
//        {
//            probabilityInShadow = 1;
//        }

        //probabilityInShadow = probabilityInShadow * probabilityInShadow;
        

        vec3 N = normalize(normalVec);
        vec3 L = normalize(lightVec);
        vec3 V = normalize(eyeVec);

        vec3 H = normalize(L+V);
        float LN = max(dot(L,N),0.0);
        float HN = max(dot(H,N),0.0);
        float LH = max(dot(L,H),0.0);


        vec3 FLH = specular + ((1,1,1) - specular) * pow((1-LH),5);
        float GLVH = pow(LH,2);
        float DH = (shininess + 2)/(2 * 3.14159) * pow(HN,shininess);
        vec3 BRDF = Kd/3.14159 + (FLH * DH)/(4 * GLVH);
        FragColor.xyz = Ia * Kd + (1.0-probabilityInShadow) *(I*LN * BRDF);

        //debug
        //FragColor = vec4(Bvalues.x);


    }
    else
    {
        FragColor.xyz = Ia * Kd;
    }



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
//    shadowCoord = shadowMatrix * vec4(worldPos,1);
//
//
//    vec3 ONE = vec3(1.0, 1.0, 1.0);
//
//    vec3 I = vec3(2.0, 2.0, 2.0);
//    vec3 Ia = 0.1*ONE;
//    vec3 Kd = diffuse; 
//
//    vec3 fragColor;
//    if (shadowCoord.w > 0)
//    {
//        shadowIndex = shadowCoord.xy/shadowCoord.w;
//    }
//    if (shadowCoord.w > 0 && shadowIndex.x >= 0 && shadowIndex.x <= 1 && shadowIndex.y >= 0 && shadowIndex.y <= 1)
//    {
//        lightDepth = texture2D(shadowMap, shadowIndex).x;
//        pixelDepth = ((shadowCoord.w - 30)/(150-30));
//        FragColor = (texture2D(shadowMap, shadowIndex)); //debugging the moment shadow map
//    }
//    else
//    {
//        FragColor = vec4(0,0,0,0); //debugging the moment shadow map
//    }
//    if(pixelDepth > lightDepth + 0.0001)
//    {
//        //FragColor.xyz = Ia * Kd;
//    }
//    else
//    {
//        //float blurredDepth = 1;
//
//        vec3 N = normalize(normalVec);
//        vec3 L = normalize(lightVec);
//        vec3 V = normalize(eyeVec);
//
//        vec3 H = normalize(L+V);
//        float LN = max(dot(L,N),0.0);
//        float HN = max(dot(H,N),0.0);
//        float LH = max(dot(L,H),0.0);
// 
//
//        vec3 FLH = specular + ((1,1,1) - specular) * pow((1-LH),5);
//        float GLVH = pow(LH,2);
//        float DH = (shininess + 2)/(2 * 3.14159) * pow(HN,shininess);
//        vec3 BRDF = Kd/3.14159 + (FLH * DH)/(4 * GLVH);
//        //FragColor.xyz = Ia * Kd + I*LN * BRDF;
//
//
//    }

}