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

in vec3 normalVec, lightVec, eyeVec;
in vec2 texCoord;

uniform int objectId;
uniform vec3 diffuse;
uniform vec3 specular;
uniform float shininess;

uniform sampler2D g0;

uniform vec2 WindowSize;

in vec3 center;
in float radius;


void main()
{
    vec2 xy = gl_FragCoord.xy/WindowSize.xy;
    vec3 worldPos = texture2D(g0, xy).xyz;
    float distanceToLight = distance(center,worldPos);
    if(distanceToLight > radius)
    {
        FragColor.xyz = vec3(0,0,0);
    }
    else
    {
        vec3 ONE = vec3(1.0, 1.0, 1.0);
        vec3 N = normalize(normalVec);
        vec3 L = normalize(lightVec);
        vec3 V = normalize(eyeVec);
        vec3 H = normalize(L+V);
        float NL = max(dot(N,L),0.0);
        float NV = max(dot(N,V),0.0);
        float HN = max(dot(H,N),0.0);

        vec3 I = ONE;
        vec3 Ia = 0.2*ONE;
        vec3 Kd = diffuse; 
    
       // Lighting is diffuse + ambient + specular
        vec3 fragColor = Ia*Kd;


        float adjustment = 35.0/(distanceToLight * distanceToLight) - 35.0/(radius * radius);
        //float adjustment = (radius/distanceToLight) * (radius/distanceToLight) ;
        //float adjustment = 3.0/(distanceToLight * distanceToLight);
        fragColor = vec3(fragColor.x * adjustment, fragColor.y * adjustment, fragColor.z * adjustment);

        FragColor.xyz = fragColor;
    }
}
