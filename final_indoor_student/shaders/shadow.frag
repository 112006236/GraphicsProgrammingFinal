#version 410 core
in vec3 p;in vec2 t;
uniform sampler2D diffuseMap;
uniform bool hasTex,radial;
uniform int kind;
uniform vec3 lightPos;
uniform float farPlane;
void main(){if(kind==1&&hasTex&&texture(diffuseMap,t).a<.5)discard;gl_FragDepth=radial?length(p-lightPos)/farPlane:gl_FragCoord.z;}
