#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>
struct Indoor {
    bool shading=true, directional=true, dirShadow=true, normal=true, bloom=true, ao=true, point=true, pointShadow=true, ssr=true, npr=false, fxaa=true, area=true, areaPanel=true, volume=true;
    int debug=0;
    float exposure=1.0f, pointPower=1.0f, areaPower=1.0f, sunPower=1.0f, fog=0.926f;
    glm::vec3 eye{4,1,-1.5f}, light{1.87659f,.4625f,.103928f}, areaCenter{1,.5f,-.5f}, sunEye{-2.845f,2.028f,-1.293f}, sunTarget{.542f,-.141f,-.422f}, volumePos{-14.225f,5.07f,-6.465f};
    float yaw=180.0f, pitch=0.0f, lookDistance=1.f, areaYaw=90.f, areaPitch=0.f;
    struct Mesh {GLuint vao=0,vbo=0, tex=0,bump=0; GLsizei count=0; glm::vec3 color{1}, ambient{1}, specular{.5f}; float shine=32,reflectivity=0,emissive=0; int kind=0;};
    std::vector<Mesh> meshes;
    std::vector<GLuint> textures;
    GLuint geom=0,shadow=0,lighting=0,reflection=0,blur=0,post=0,quad=0;
    GLuint gF=0,gPos=0,gNorm=0,gAlb=0,gMat=0,gAmbient=0,gDepth=0,dirF=0,dirDepth=0,cubeF=0,cubeDepth=0,hdrF=0,hdr=0,refF=0,ref=0,blurF[2]{},blurT[2]{};
    GLuint areaF=0,areaDepth=0,panelVAO=0,panelVBO=0,sphereVAO=0,sphereVBO=0; GLsizei sphereCount=0;
    bool dirReady=false,pointReady=false,areaReady=false; glm::vec3 lastPoint{},lastArea{},lastSun{}; float lastYaw=999, lastPitch=999;
    int w=0,h=0; static constexpr int shadowSize=1024;
    glm::vec3 forward() const;
    void init(); void resize(int,int); void draw(int,int); void release();
    void load(const std::string&,bool); void geometry(GLuint,bool);
};
