#version 410 core
out vec2 uv;
void main(){vec2 q=vec2((gl_VertexID<<1)&2,gl_VertexID&2);uv=q;gl_Position=vec4(q*2-1,0,1);}
