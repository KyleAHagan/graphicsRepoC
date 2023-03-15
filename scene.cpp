////////////////////////////////////////////////////////////////////////
// The scene class contains all the parameters needed to define and
// draw a simple scene, including:
//   * Geometry
//   * Light parameters
//   * Material properties
//   * viewport size parameters
//   * Viewing transformation values
//   * others ...
//
// Some of these parameters are set when the scene is built, and
// others are set by the framework in response to user mouse/keyboard
// interactions.  All of them can be used to draw the scene.

const bool fullPolyCount = true; // Use false when emulating the graphics pipeline in software

#include "math.h"
#include <cmath>//new
#include <iostream>
#include <stdlib.h>
#include <time.h>

#include <glbinding/gl/gl.h>
#include <glbinding/Binding.h>
using namespace gl;

#include <glu.h>                // For gluErrorString

#define GLM_FORCE_RADIANS
#define GLM_SWIZZLE
#include <glm/glm.hpp>
#include <glm/ext.hpp>          // For printing GLM objects with to_string

#include "framework.h"
#include "shapes.h"
#include "object.h"
#include "texture.h"
#include "transform.h"
int randX, randY;

struct localLight
{
    float radius;

    float positionX;
    float positionY;
    float positionZ;

    float colorR;
    float colorG;
    float colorB;
};

const int numlights = 400;
localLight localLightList[numlights];

unsigned int quadVAO, quadVBO;

const float PI = 3.14159f;
const float rad = PI/180.0f;    // Convert degrees to radians

glm::mat4 Identity;

const float grndSize = 1000.0;    // Island radius;  Minimum about 20;  Maximum 1000 or so
const float grndOctaves = 4.0;  // Number of levels of detail to compute
const float grndFreq = 0.03;    // Number of hills per (approx) 50m
const float grndPersistence = 0.03; // Terrain roughness: Slight:0.01  rough:0.05
const float grndLow = -3.0;         // Lowest extent below sea level
const float grndHigh = 5.0;        // Highest extent above sea level

////////////////////////////////////////////////////////////////////////
// This macro makes it easy to sprinkle checks for OpenGL errors
// throughout your code.  Most OpenGL calls can record errors, and a
// careful programmer will check the error status *often*, perhaps as
// often as after every OpenGL call.  At the very least, once per
// refresh will tell you if something is going wrong.
#define CHECKERROR {GLenum err = glGetError(); if (err != GL_NO_ERROR) { fprintf(stderr, "OpenGL error (at line scene.cpp:%d): %s\n", __LINE__, gluErrorString(err)); exit(-1);} }

// Create an RGB color from human friendly parameters: hue, saturation, value
glm::vec3 HSV2RGB(const float h, const float s, const float v)
{
    if (s == 0.0)
        return glm::vec3(v,v,v);

    int i = (int)(h*6.0) % 6;
    float f = (h*6.0f) - i;
    float p = v*(1.0f - s);
    float q = v*(1.0f - s*f);
    float t = v*(1.0f - s*(1.0f-f));
    if      (i == 0)     return glm::vec3(v,t,p);
    else if (i == 1)  return glm::vec3(q,v,p);
    else if (i == 2)  return glm::vec3(p,v,t);
    else if (i == 3)  return glm::vec3(p,q,v);
    else if (i == 4)  return glm::vec3(t,p,v);
    else   /*i == 5*/ return glm::vec3(v,p,q);
}

////////////////////////////////////////////////////////////////////////
// Constructs a hemisphere of spheres of varying hues
//Object* SphereOfSpheres(Shape* SpherePolygons)
//{
//    Object* ob = new Object(NULL, nullId);
//    
//    for (float angle = 0.0; angle < 360.0; angle += 9.0)
//    {
//        for (float row = 0.05; row < 10; row += 0.5)
//        {
//            glm::vec3 hue = HSV2RGB(angle / 360.0, 2.0f, 1.0f);
//
//            Object* sp = new Object(SpherePolygons, spheresId, hue, glm::vec3(1.0, 1.0, 1.0), 120.0);
//            float s = sin(row);
//            float c = row + 1;
//            ob->add(sp, Rotate(2, angle) * Translate(c, 0, 0.25) * Scale(0.1, 0.1, 0.1));
//        }
//    }
//    return ob;
//}
Object* SphereOfSpheres(Shape* SpherePolygons)
{
    Object* ob = new Object(NULL, nullId);

    for (float angle = 0.0; angle < 360.0; angle += 18.0)
        for (float row = 0.075; row < PI / 2.0; row += PI / 2.0 / 6.0) {
            glm::vec3 hue = HSV2RGB(angle / 360.0, 1.0f - 2.0f * row / PI, 1.0f);

            Object* sp = new Object(SpherePolygons, spheresId,
                hue, glm::vec3(1.0, 1.0, 1.0), 120.0);
            float s = sin(row);
            float c = cos(row);
            ob->add(sp, Rotate(2, angle) * Translate(c, 0, s) * Scale(0.075 * c, 0.075 * c, 0.075 * c));
        }
    return ob;
}

Object* SphereOfLights(Shape* SpherePolygons)
{
    Object* ob = new Object(NULL, nullId);

    for (float angle = 0.0; angle < 360.0; angle += 9.0)
    {
        for (float row = 0.05; row < 10; row += 0.5)
        {
            glm::vec3 hue = HSV2RGB(angle / 360.0, 2.0f, 1.0f);

            Object* sp = new Object(SpherePolygons, spheresId, hue, hue, 0.0);
            float s = sin(row);
            float c = row + 1;
            ob->add(sp, Rotate(2, angle) * Translate(c, 0, 0.25) * Scale(0.3, 0.3, 0.3));
        }
    }
    return ob;
}


////////////////////////////////////////////////////////////////////////
// Constructs a -1...+1  quad (canvas) framed by four (elongated) boxes
Object* FramedPicture(const glm::mat4& modelTr, const int objectId, 
                      Shape* BoxPolygons, Shape* QuadPolygons)
{
    // This draws the frame as four (elongated) boxes of size +-1.0
    float w = 0.05;             // Width of frame boards.
    
    Object* frame = new Object(NULL, nullId);
    Object* ob;
    
    glm::vec3 woodColor(87.0/255.0,51.0/255.0,35.0/255.0);
    ob = new Object(BoxPolygons, frameId,
                    woodColor, glm::vec3(0.2, 0.2, 0.2), 10.0);
    frame->add(ob, Translate(0.0, 0.0, 1.0+w)*Scale(1.0, w, w));
    frame->add(ob, Translate(0.0, 0.0, -1.0-w)*Scale(1.0, w, w));
    frame->add(ob, Translate(1.0+w, 0.0, 0.0)*Scale(w, w, 1.0+2*w));
    frame->add(ob, Translate(-1.0-w, 0.0, 0.0)*Scale(w, w, 1.0+2*w));

    ob = new Object(QuadPolygons, objectId,
                    woodColor, glm::vec3(0.0, 0.0, 0.0), 10.0);
    frame->add(ob, Rotate(0,90));

    return frame;
}

////////////////////////////////////////////////////////////////////////
// InitializeScene is called once during setup to create all the
// textures, shape VAOs, and shader programs as well as setting a
// number of other parameters.
void Scene::InitializeScene()
{
    srand(time(NULL));
    glEnable(GL_DEPTH_TEST);
    CHECKERROR;

    // @@ Initialize interactive viewing variables here. (spin, tilt, ry, front back, ...)
    
    // Set initial light parameters
    lightSpin = 150.0;
    lightTilt = -45.0;
    lightDist = 100.0;
    // @@ Perhaps initialize additional scene lighting values here. (lightVal, lightAmb)
    
    shadowMap.CreateFBO(1024, 1024);

    key = 0;
    nav = false;
    spin = 0.0;
    tilt = 30.0;
    eye = glm::vec3(0.0, -20.0, 0.0);
    speed = 300.0/30.0;
    last_time = glfwGetTime();
    tr = glm::vec3(0.0, 0.0, 25.0);

    ry = 0.4;
    front = 0.5;
    back = 5000.0;

    CHECKERROR;
    objectRoot = new Object(NULL, nullId);

    
    // Enable OpenGL depth-testing
    glEnable(GL_DEPTH_TEST);

    // Create the lighting shader program from source code files.
    // @@ Initialize additional shaders if necessary
    lightingProgram = new ShaderProgram();
    lightingProgram->AddShader("lightingPhong.vert", GL_VERTEX_SHADER);
    lightingProgram->AddShader("lightingPhong.frag", GL_FRAGMENT_SHADER);

    glBindAttribLocation(lightingProgram->programId, 0, "vertex");
    glBindAttribLocation(lightingProgram->programId, 1, "vertexNormal");
    glBindAttribLocation(lightingProgram->programId, 2, "vertexTexture");
    glBindAttribLocation(lightingProgram->programId, 3, "vertexTangent");
    lightingProgram->LinkProgram();

    shadowMapProgram = new ShaderProgram();
    shadowMapProgram->AddShader("shadowmap.vert", GL_VERTEX_SHADER);
    shadowMapProgram->AddShader("shadowmap.frag", GL_FRAGMENT_SHADER);

    glBindAttribLocation(shadowMapProgram->programId, 0, "vertex");
    glBindAttribLocation(shadowMapProgram->programId, 1, "vertexNormal");
    glBindAttribLocation(shadowMapProgram->programId, 2, "vertexTexture");
    glBindAttribLocation(shadowMapProgram->programId, 3, "vertexTangent");
    shadowMapProgram->LinkProgram();



    shadowBlurProgram = new ShaderProgram();
    shadowBlurProgram->AddShader("shadowBlur.comp", GL_COMPUTE_SHADER);
    shadowBlurProgram->LinkProgram();


    // Create the lighting shader program from source code files.
    // @@ Initialize additional shaders if necessary
    lightingProgramDeferred = new ShaderProgram();
    lightingProgramDeferred->AddShader("lightingPhongDeferred.vert", GL_VERTEX_SHADER);
    lightingProgramDeferred->AddShader("lightingPhongDeferred.frag", GL_FRAGMENT_SHADER);

    glBindAttribLocation(lightingProgram->programId, 0, "vertex");
    glBindAttribLocation(lightingProgram->programId, 1, "vertexNormal");
    glBindAttribLocation(lightingProgram->programId, 2, "vertexTexture");
    glBindAttribLocation(lightingProgram->programId, 3, "vertexTangent");
    lightingProgramDeferred->LinkProgram();
     

    localLightingProgram = new ShaderProgram();
    localLightingProgram->AddShader("localLights.vert", GL_VERTEX_SHADER);
    localLightingProgram->AddShader("localLights.frag", GL_FRAGMENT_SHADER);

    glBindAttribLocation(localLightingProgram->programId, 0, "vertex");
    glBindAttribLocation(localLightingProgram->programId, 1, "vertexNormal");
    glBindAttribLocation(localLightingProgram->programId, 2, "vertexTexture");
    glBindAttribLocation(localLightingProgram->programId, 3, "vertexTangent");
    localLightingProgram->LinkProgram();

    glfwGetFramebufferSize(window, &width, &height);
    FrameBufferObject.CreateFBO(width, height);

    float quadVertices[] =
    {
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };

    float radius = 10;

    float positionX = 10;
    float positionY = 10;
    float positionZ = 10;

    float colorR = 128;
    float colorG = 128;
    float colorB = 128;

    for (int i = 0; i < numlights; i++)
    {
        localLight newLight;
        newLight.radius = 10;
        newLight.positionX = 10 * i;
        newLight.positionY = 10;
        newLight.positionZ = 10;
        newLight.colorR = 256;
        newLight.colorG = 256;
        newLight.colorB = 256;
        localLightList[i] = newLight;
    }


    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    // Create all the Polygon shapes
    proceduralground = new ProceduralGround(grndSize, 400,grndOctaves, grndFreq, grndPersistence,grndLow, grndHigh);
    
    Shape* TeapotPolygons =  new Teapot(fullPolyCount?12:2);
    Shape* BoxPolygons = new Box();
    Shape* SpherePolygons = new Sphere(32);
    Shape* RoomPolygons = new Ply("room.ply");
    Shape* FloorPolygons = new Plane(10.0, 10);
    Shape* QuadPolygons = new Quad();
    Shape* SeaPolygons = new Plane(2000.0, 50);
    Shape* GroundPolygons = proceduralground;

    // Various colors used in the subsequent models
    glm::vec3 woodColor(87.0/255.0, 51.0/255.0, 35.0/255.0);
    glm::vec3 brickColor(134.0/255.0, 60.0/255.0, 56.0/255.0);
    glm::vec3 floorColor(6*16/255.0, 5.5*16/255.0, 3*16/255.0);
    glm::vec3 brassColor(0.5, 0.5, 0.1);
    glm::vec3 grassColor(62.0/255.0, 102.0/255.0, 38.0/255.0);
    glm::vec3 waterColor(0.3, 0.3, 1.0);

    glm::vec3 black(0.0, 0.0, 0.0);
    glm::vec3 brightSpec(0.5, 0.5, 0.5);
    glm::vec3 polishedSpec(0.3, 0.3, 0.3);
 
    // Creates all the models from which the scene is composed.  Each
    // is created with a polygon shape (possibly NULL), a
    // transformation, and the surface lighting parameters Kd, Ks, and
    // alpha.

    // @@ This is where you could read in all the textures and
    // associate them with the various objects being created in the
    // next dozen lines of code.

    // @@ To change an object's surface parameters (Kd, Ks, or alpha),
    // modify the following lines.
    
    central    = new Object(NULL, nullId);
    anim       = new Object(NULL, nullId);
    room       = new Object(RoomPolygons, roomId, brickColor, black, 1);
    floor      = new Object(FloorPolygons, floorId, floorColor, black, 1);
    teapot     = new Object(TeapotPolygons, teapotId, brassColor, brightSpec, 120);
    podium     = new Object(BoxPolygons, boxId, glm::vec3(woodColor), polishedSpec, 10); 
    sky        = new Object(SpherePolygons, skyId, black, black, 0);
    ground     = new Object(GroundPolygons, groundId, grassColor, black, 1);
    sea        = new Object(SeaPolygons, seaId, waterColor, brightSpec, 120);
    leftFrame  = FramedPicture(Identity, lPicId, BoxPolygons, QuadPolygons);
    rightFrame = FramedPicture(Identity, rPicId, BoxPolygons, QuadPolygons); 
    spheres    = SphereOfSpheres(SpherePolygons);
    lights = SphereOfLights(SpherePolygons);
#ifdef REFL
    spheres->drawMe = true;
#else
    spheres->drawMe = true; //CHANGED was false
#endif
    lights->drawMe = false;

    // @@ To change the scene hierarchy, examine the hierarchy created
    // by the following object->add() calls and adjust as you wish.
    // The objects being manipulated and their polygon shapes are
    // created above here.

    // Scene is composed of sky, ground, sea, room and some central models
    if (fullPolyCount) {
        objectRoot->add(sky, Scale(2000.0, 2000.0, 2000.0));
        objectRoot->add(sea); 
        objectRoot->add(ground); }
    objectRoot->add(central);
#ifndef REFL
    objectRoot->add(room,  Translate(0.0, 0.0, 0.02));
#endif
    objectRoot->add(floor, Translate(0.0, 0.0, 0.02));

    // Central model has a rudimentary animation (constant rotation on Z)
    animated.push_back(anim);

    // Central contains a teapot on a podium and an external sphere of spheres
    central->add(podium, Translate(0.0, 0,0));
    central->add(anim, Translate(0.0, 0,0));
    anim->add(teapot, Translate(0.1, 0.0, 1.5)*TeapotPolygons->modelTr);
    if (fullPolyCount)
    {
        anim->add(spheres, Translate(0.0, 0.0, 0.0) * Scale(16, 16, 16));
        anim->add(lights, Translate(0.0, 0.0, 0.0) * Scale(16, 16, 16));
    }
    // Room contains two framed pictures
    if (fullPolyCount)
    {
        room->add(leftFrame, Translate(-1.5, 9.85, 1.)*Scale(0.8, 0.8, 0.8));
        room->add(rightFrame, Translate( 1.5, 9.85, 1.)*Scale(0.8, 0.8, 0.8));
    }

    CHECKERROR;

    // Options menu stuff
    show_demo_window = false;
}

void Scene::DrawMenu()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    if (ImGui::BeginMainMenuBar()) {
        // This menu demonstrates how to provide the user a list of toggleable settings.
        if (ImGui::BeginMenu("Objects")) {
            if (ImGui::MenuItem("Draw spheres", "", spheres->drawMe))  {spheres->drawMe ^= true; }
            if (ImGui::MenuItem("Draw walls", "", room->drawMe))       {room->drawMe ^= true; }
            if (ImGui::MenuItem("Draw ground/sea", "", ground->drawMe)){ground->drawMe ^= true;
                							sea->drawMe = ground->drawMe;}
            ImGui::EndMenu(); }
                	
        // This menu demonstrates how to provide the user a choice
        // among a set of choices.  The current choice is stored in a
        // variable named "mode" in the application, and sent to the
        // shader to be used as you wish.
        if (ImGui::BeginMenu("Menu ")) {
            if (ImGui::MenuItem("<sample menu of choices>", "",	false, false)) {}
            if (ImGui::MenuItem("Do nothing 0", "",		mode==0)) { mode=0; }
            if (ImGui::MenuItem("Do nothing 1", "",		mode==1)) { mode=1; }
            if (ImGui::MenuItem("Do nothing 2", "",		mode==2)) { mode=2; }
            ImGui::EndMenu(); }
        
        ImGui::EndMainMenuBar(); }
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Scene::BuildTransforms()
{
    
    // Work out the eye position as the user move it with the WASD keys.
    float now = glfwGetTime();
    float dist = (now-last_time)*speed;
    last_time = now;
    if (key == GLFW_KEY_W)
        eye += dist*glm::vec3(sin(spin*rad), cos(spin*rad), 0.0);
    if (key == GLFW_KEY_S)
        eye -= dist*glm::vec3(sin(spin*rad), cos(spin*rad), 0.0);
    if (key == GLFW_KEY_D)
        eye += dist*glm::vec3(cos(spin*rad), -sin(spin*rad), 0.0);
    if (key == GLFW_KEY_A)
        eye -= dist*glm::vec3(cos(spin*rad), -sin(spin*rad), 0.0);

    eye[2] = proceduralground->HeightAt(eye[0], eye[1]) + 2.0;

    CHECKERROR;

    if (nav)
        WorldView = Rotate(0, tilt-90)*Rotate(2, spin) *Translate(-eye[0], -eye[1], -eye[2]);
    else
        WorldView = Translate(tr[0], tr[1], -tr[2]) *Rotate(0, tilt-90)*Rotate(2, spin);
    WorldProj = Perspective((ry*width)/height, ry, front, (mode==0) ? 1000 : back);


    shadowProj = Perspective(40.0f / lightDist, 40.0f / lightDist, front, back);
    shadowView = LookAt(lightPos, glm::vec3(0, 0, 0), glm::vec3(0, 0, 1));

    //shadowProj = WorldProj;//DEBUG REMOVE
    //shadowView = WorldView;//DEBUG REMOVE

    // @@ Print the two matrices (in column-major order) for
    // comparison with the project document.
    //std::cout << "WorldView: " << glm::to_string(WorldView) << std::endl;
    //std::cout << "WorldProj: " << glm::to_string(WorldProj) << std::endl;
}

void Scene::BuildGaussianBlur(int blurWidth)
{
    delete blurWeights;
    blurWeights = new float[2 * blurWidth + 1];

    for (int iterator = 0; iterator < (2 * blurWidth + 1); iterator++)
    {
        float blurWeight;
        int blurIndex = iterator - blurWidth;
        float s = blurWidth / 2.0;//Focuses on the center of the curve.
        float coefficient = (-1.0 / 2.0);//debug
        float blurIndexOverS = blurIndex / s;//debug
        float blurIndexOverSSquared = pow((blurIndex / s), 2);//debug
        blurWeight = exp((-1.0/ 2.0)*pow((blurIndex/s),2));
        blurWeights[iterator] = blurWeight;
    }
    float blurTotal = 0;
    for (int iterator = 0; iterator < (2 * blurWidth + 1); iterator++)
    {
        blurTotal += blurWeights[iterator];
    }
    for (int iterator = 0; iterator < (2 * blurWidth + 1); iterator++)
    {
        blurWeights[iterator] = blurWeights[iterator] / blurTotal;
    }
}




////////////////////////////////////////////////////////////////////////
// Procedure DrawScene is called whenever the scene needs to be
// drawn. (Which is often: 30 to 60 times per second are the common
// goals.)
void Scene::DrawScene()
{
    randX = rand() % 1024;
    randY = rand() % 1024;
    // Set the viewport
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    CHECKERROR;
    // Calculate the light's position from lightSpin, lightTilt, lightDist
    lightPos = glm::vec3(lightDist*cos(lightSpin*rad)*sin(lightTilt*rad),
                         lightDist*sin(lightSpin*rad)*sin(lightTilt*rad), 
                         lightDist*cos(lightTilt*rad));

    // Update position of any continuously animating objects
    double atime = 360.0*glfwGetTime()/36;
    for (std::vector<Object*>::iterator m = animated.begin(); m < animated.end(); m++)
    {
        (*m)->animTr = Rotate(2, atime);
    }

    BuildTransforms();
    int blursize = 50;
    BuildGaussianBlur(blursize);
    // The lighting algorithm needs the inverse of the WorldView matrix
    WorldInverse = glm::inverse(WorldView);
    


    ////////////////////////////////////////////////////////////////////////////////
    // Anatomy of a pass:
    //   Choose a shader  (create the shader in InitializeScene above)
    //   Choose and FBO/Render-Target (if needed; create the FBO in InitializeScene above)
    //   Set the viewport (to the pixel size of the screen or FBO)
    //   Clear the screen.
    //   Set the uniform variables required by the shader
    //   Draw the geometry
    //   Unset the FBO (if one was used)
    //   Unset the shader
    ////////////////////////////////////////////////////////////////////////////////

    CHECKERROR;
    int loc, programId;

    ////////////////////////////////////////////////////////////////////////////////
    // Deferred Lighting storing values pass
    ////////////////////////////////////////////////////////////////////////////////
    
    //
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    //

    // Choose the lighting shader
    lightingProgram->UseShader();
    programId = lightingProgram->programId;

    // Set the viewport, and clear the screen
    //glViewport(0, 0, width, height);
    FrameBufferObject.BindFBO();
    GLenum bufs[4] = { GL_COLOR_ATTACHMENT0_EXT , GL_COLOR_ATTACHMENT1_EXT ,GL_COLOR_ATTACHMENT2_EXT , GL_COLOR_ATTACHMENT3_EXT };
    glDrawBuffers(4, bufs);


    CHECKERROR;
    glClearColor(0.5, 0.5, 0.5, 1.0);
    CHECKERROR;
    glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT);
    CHECKERROR;

    // @@ The scene specific parameters (uniform variables) used by
    // the shader are set here.  Object specific parameters are set in
    // the Draw procedure in object.cpp
    CHECKERROR;
    loc = glGetUniformLocation(programId, "WorldProj");
    glUniformMatrix4fv(loc, 1, GL_FALSE, Pntr(WorldProj));
    loc = glGetUniformLocation(programId, "WorldView");
    glUniformMatrix4fv(loc, 1, GL_FALSE, Pntr(WorldView));
    loc = glGetUniformLocation(programId, "WorldInverse");
    glUniformMatrix4fv(loc, 1, GL_FALSE, Pntr(WorldInverse));
    loc = glGetUniformLocation(programId, "lightPos");
    glUniform3fv(loc, 1, &(lightPos[0]));   
    loc = glGetUniformLocation(programId, "mode");
    glUniform1i(loc, mode);
    CHECKERROR;

    // Draw all objects (This recursively traverses the object hierarchy.)
    CHECKERROR;
    objectRoot->Draw(lightingProgram, Identity);
    CHECKERROR; 

    
    // Turn off the shader
    lightingProgram->UnuseShader();
    CHECKERROR;
    FrameBufferObject.UnbindFBO();

    ////////////////////////////////////////////////////////////////////////////////
    // Build Shadow Map pass
    ////////////////////////////////////////////////////////////////////////////////
    

    shadowMapProgram->UseShader();
    shadowMap.BindFBO();
    GLenum buffers[4] = { GL_COLOR_ATTACHMENT0_EXT , GL_COLOR_ATTACHMENT1_EXT ,GL_COLOR_ATTACHMENT2_EXT , GL_COLOR_ATTACHMENT3_EXT }; //TODO DEBUG research gl_color_attachment
    glDrawBuffers(4, buffers);
    programId = shadowMapProgram->programId;

    // Set the viewport, and clear the screen
    glViewport(0, 0, 1024, 1024);//NOTES: This is the shadowmap's dimensions
    glClearColor(0.5, 0.5, 0.5, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    // @@ The scene specific parameters (uniform variables) used by
    // the shader are set here.  Object specific parameters are set in
    // the Draw procedure in object.cpp

    CHECKERROR;
    loc = glGetUniformLocation(programId, "WorldProj");
    glUniformMatrix4fv(loc, 1, GL_FALSE, Pntr(WorldProj));
    loc = glGetUniformLocation(programId, "WorldView");
    glUniformMatrix4fv(loc, 1, GL_FALSE, Pntr(WorldView));
    loc = glGetUniformLocation(programId, "WorldInverse");
    glUniformMatrix4fv(loc, 1, GL_FALSE, Pntr(WorldInverse));
    loc = glGetUniformLocation(programId, "mode");
    glUniform1i(loc, mode);
    CHECKERROR;
    loc = glGetUniformLocation(programId, "shadowProj");
    CHECKERROR;
    glUniformMatrix4fv(loc, 1, GL_FALSE, Pntr(shadowProj));
    CHECKERROR;
    loc = glGetUniformLocation(programId, "shadowView");
    CHECKERROR;
    glUniformMatrix4fv(loc, 1, GL_FALSE, Pntr(shadowView));
    CHECKERROR;
    loc = glGetUniformLocation(programId, "lightPos");
    CHECKERROR;
    glUniform3fv(loc, 1, &(lightPos[0]));
    CHECKERROR;

    // Draw all objects (This recursively traverses the object hierarchy.)
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    objectRoot->Draw(shadowMapProgram, Identity);
    glDisable(GL_CULL_FACE);
    CHECKERROR;


    shadowMap.UnbindFBO();
    // Turn off the shader
    shadowMapProgram->UnuseShader();



    ////////////////////////////////////////////////////////////////////////////////
    // Blur Shadow Map pass
    ////////////////////////////////////////////////////////////////////////////////
    CHECKERROR;
    shadowBlurProgram->UseShader();
    programId = shadowBlurProgram->programId;
    gl::GLuint blockID; //?
    gl::GLuint bindpoint; //?
    glGenBuffers(1, &blockID); //Generates block
    bindpoint = 0;//"bindpoint = ?; //Start at 0 increment for other blocks"
    //Send block of weights to the shader as a uniform block
    loc = glGetUniformBlockIndex(programId, "blurKernel");
    glUniformBlockBinding(programId, loc, bindpoint);
    glBindBuffer(GL_UNIFORM_BUFFER, blockID);
    glBindBufferBase(GL_UNIFORM_BUFFER, bindpoint, blockID);
    CHECKERROR;
    glBufferData(GL_UNIFORM_BUFFER, 4* (blursize * 2 + 1), blurWeights, GL_STATIC_DRAW);//debug: assuming floats are 4 bytes
    CHECKERROR;
    loc = glGetUniformLocation(programId, "blurSize");
    glUniform1i(loc, blursize);
    //loc = glGetUniformLocation(programId, "src");
    //glBindImageTexture(0, textureID, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
    //glUniform1i(loc, imageUnit);

    //loc = glGetUniformLocation(programId, "dst");
    //glBindImageTexture(1, textureID, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    //glUniform1i(loc, imageUnit);

    shadowMap.BindImageTexture(0, 0, gl::GLenum::GL_READ_ONLY, programId, "src");
    shadowMap.BindImageTexture(1, 1, gl::GLenum::GL_WRITE_ONLY, programId, "dst");
    CHECKERROR;
    glDispatchCompute(width / 128, height, 1); // Tiles WxH image groups sized 128x1
    CHECKERROR;
    shadowBlurProgram->UnuseShader();
    ////////////////////////////////////////////////////////////////////////////////
    // Deferred Lighting using stored values pass
    ////////////////////////////////////////////////////////////////////////////////
     
    CHECKERROR;
    // Choose the lighting shader
    lightingProgramDeferred->UseShader();
    CHECKERROR;
    programId = lightingProgramDeferred->programId;
    CHECKERROR;
    // Set the viewport, and clear the screen
    glViewport(0, 0, width, height);
    glClearColor(0.5, 0.5, 0.5, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    FrameBufferObject.BindTexture(0, 0, programId, "g0");
    FrameBufferObject.BindTexture(1, 1, programId, "g1");
    FrameBufferObject.BindTexture(2, 2, programId, "g2");
    FrameBufferObject.BindTexture(3, 3, programId, "g3");

    shadowMap.BindTexture(4, 1, programId, "shadowMap");//? should go from 0 to 1 once the blur is putting the completed blur in 1.

    CHECKERROR;

    shadowMatrix = Translate(0.5, 0.5, 0.5) * Scale(0.5, 0.5, 0.5) * shadowProj * shadowView;


    // @@ The scene specific parameters (uniform variables) used by
    // the shader are set here.  Object specific parameters are set in
    // the Draw procedure in object.cpp

    loc = glGetUniformLocation(programId, "randXY");
    glUniform2f(loc, randX, randY);
    CHECKERROR;

    loc = glGetUniformLocation(programId, "shadowMatrix");
    glUniformMatrix4fv(loc, 1, GL_FALSE, Pntr(shadowMatrix));

    loc = glGetUniformLocation(programId, "WindowSize");
    glUniform2f(loc, width, height);
    CHECKERROR;
    loc = glGetUniformLocation(programId, "WorldProj");
    glUniformMatrix4fv(loc, 1, GL_FALSE, Pntr(WorldProj));
    CHECKERROR;
    loc = glGetUniformLocation(programId, "WorldView");
    glUniformMatrix4fv(loc, 1, GL_FALSE, Pntr(WorldView));
    CHECKERROR;
    loc = glGetUniformLocation(programId, "WorldInverse");
    glUniformMatrix4fv(loc, 1, GL_FALSE, Pntr(WorldInverse));
    CHECKERROR;
    loc = glGetUniformLocation(programId, "lightPos");
    glUniform3fv(loc, 1, &(lightPos[0]));
    CHECKERROR;
    loc = glGetUniformLocation(programId, "mode");
    glUniform1i(loc, mode);

    // Draw all objects (This recursively traverses the object hierarchy.)
    CHECKERROR;
    //objectRoot->Draw(lightingProgramDeferred, Identity);
    //TODO: instead draw a quad that covers the screen
    glDisable(GL_DEPTH_TEST);
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    CHECKERROR;


    // Turn off the shader
    lightingProgramDeferred->UnuseShader();
    glEnable(GL_DEPTH_TEST);

    ////////////////////////////////////////////////////////////////////////////////////
    ////// //Ambient Light pass
    ////////////////////////////////////////////////////////////////////////////////////
    //////spheres->drawMe = false;//debug
    //////lights->drawMe = false;//debug
    ////////////////////////////////////////////////////////////////////////////////////
    ////// //Local Lights pass
    ////////////////////////////////////////////////////////////////////////////////////
    //central->drawMe = true;
    //room->drawMe = false;
    //floor->drawMe = false;
    //teapot->drawMe = false;
    //podium->drawMe = false;
    //sky->drawMe = false;
    //ground->drawMe = false;
    //sea->drawMe = false;
    //spheres->drawMe = false;
    //leftFrame->drawMe = false;
    //rightFrame->drawMe = false;
    //lights->drawMe = true;

    //localLightingProgram->UseShader();
    //CHECKERROR;
    //programId = localLightingProgram->programId;

    //CHECKERROR;
    //glDisable(GL_DEPTH_TEST);
    //glEnable(GL_BLEND);
    //glBlendFunc(GL_ONE, GL_ONE);
    //glEnable(GL_CULL_FACE);
    //glCullFace(GL_BACK);
    //CHECKERROR;

    //FrameBufferObject.BindTexture(0, 0, programId, "g0");
    //CHECKERROR;

    //loc = glGetUniformLocation(programId, "WindowSize");
    //glUniform2f(loc, width, height);
    //loc = glGetUniformLocation(programId, "WorldProj");
    //glUniformMatrix4fv(loc, 1, GL_FALSE, Pntr(WorldProj));
    //loc = glGetUniformLocation(programId, "WorldView");
    //glUniformMatrix4fv(loc, 1, GL_FALSE, Pntr(WorldView));
    //loc = glGetUniformLocation(programId, "WorldInverse");
    //glUniformMatrix4fv(loc, 1, GL_FALSE, Pntr(WorldInverse));
    //loc = glGetUniformLocation(programId, "mode");
    //glUniform1i(loc, mode);
    //CHECKERROR;

    //objectRoot->Draw(localLightingProgram, Identity);
    //CHECKERROR;

    //glDisable(GL_CULL_FACE);
    //glEnable(GL_DEPTH_TEST);
    //glDisable(GL_BLEND);
    //central->drawMe = true;
    //room->drawMe = true;
    //floor->drawMe = true;
    //teapot->drawMe = true;
    //podium->drawMe = true;
    //sky->drawMe = true;
    //ground->drawMe = true;
    //sea->drawMe = true;
    //spheres->drawMe = true;
    //leftFrame->drawMe = true;
    //rightFrame->drawMe = true;
    //lights->drawMe = false;
    //CHECKERROR;

}
