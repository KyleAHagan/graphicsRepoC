////////////////////////////////////////////////////////////////////////
// The scene class contains all the parameters needed to define and
// draw a simple scene, including:
//   * Geometry
//   * Light parameters
//   * Material properties
//   * Viewport size parameters
//   * Viewing transformation values
//   * others ...
//
// Some of these parameters are set when the scene is built, and
// others are set by the framework in response to user mouse/keyboard
// interactions.  All of them can be used to draw the scene.

#include "shapes.h"
#include "object.h"
#include "texture.h"
#include "fbo.h"

enum ObjectIds {
    nullId	= 0,
    skyId	= 1,
    seaId	= 2,
    groundId	= 3,
    roomId	= 4,
    boxId	= 5,
    frameId	= 6,
    lPicId	= 7,
    rPicId	= 8,
    teapotId	= 9,
    spheresId	= 10,
    floorId     = 11
};

class Shader;


class Scene
{
public:
    GLFWwindow* window;

    // @@ Declare interactive viewing variables here. (spin, tilt, ry, front back, ...)

    // Light parameters
    float lightSpin, lightTilt, lightDist;
    glm::vec3 lightPos;
    // @@ Perhaps declare additional scene lighting values here. (lightVal, lightAmb)
    


    bool drawReflective;
    bool nav;
    int key;
    float spin, tilt, speed, ry, front, back;
    glm::vec3 eye, tr;
    float last_time;
    int smode; // Shadow on/off/debug mode
    int rmode; // Extra reflection indicator hooked up some keys and sent to shader
    int lmode; // BRDF mode
    int tmode; // Texture mode
    int imode; // Image Based Lighting mode
    bool flatshade;
    int mode; // Extra mode indicator hooked up to number keys and sent to shader
    
    // Viewport
    int width, height;

    // Transformations
    glm::mat4 WorldProj, WorldView, WorldInverse, shadowProj, shadowView, ShadowInverse, shadowMatrix;

    // All objects in the scene are children of this single root object.
    Object* objectRoot;
    Object *central, *anim, *room, *floor, *teapot, *podium, *sky, *ground, *sea, *spheres, *lights, *leftFrame, *rightFrame;

    std::vector<Object*> animated;
    ProceduralGround* proceduralground;

    // Shader programs
    ShaderProgram* lightingProgram;
    // @@ Declare additional shaders if necessary
    ShaderProgram* shadowMapProgram;
    ShaderProgram* shadowBlurProgram;
    ShaderProgram* lightingProgramDeferred;
    ShaderProgram* localLightingProgram;

    float* blurWeights;

    FBO FrameBufferObject;
    FBO shadowMap;


    // Options menu stuff
    bool show_demo_window;

    void InitializeScene();
    void BuildTransforms();
    void BuildGaussianBlur(int width);
    void DrawMenu();
    void DrawScene();

};
