#version 410 core
layout(location=0) in vec3 position;
layout(location=2) in vec2 uv;
uniform mat4 vp;
out vec3 p;out vec2 t;
void main(){p=position;t=uv;gl_Position=vp*vec4(p,1);}
