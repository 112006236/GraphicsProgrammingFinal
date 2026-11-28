#version 410 core
layout(location=0) in vec3 position;
layout(location=1) in vec3 normal;
layout(location=2) in vec2 uv;
uniform mat4 vp;uniform vec3 offset;uniform mat4 rotation;
out vec3 p,n;
out vec2 t;
void main(){p=(rotation*vec4(position,1)).xyz+offset;n=mat3(rotation)*normal;t=uv;gl_Position=vp*vec4(p,1);}
