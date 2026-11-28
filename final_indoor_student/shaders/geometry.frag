#version 410 core
in vec3 p,n;in vec2 t;
layout(location=0) out vec4 pos;
layout(location=1) out vec4 norm;
layout(location=2) out vec4 alb;
layout(location=3) out vec4 mat;
layout(location=4) out vec4 ambient;
uniform sampler2D diffuseMap,normalMap;
uniform bool hasTex,hasBump;
uniform vec3 baseColor,ambientColor,specularColor;
uniform float shine,reflectivity,emissive;
uniform int kind;
void main(){vec4 tex=hasTex?texture(diffuseMap,t):vec4(1);if(kind==1&&tex.a<.5)discard;
vec3 N=normalize(n);if(!gl_FrontFacing)N=-N;
if(hasBump){vec3 dpdx=dFdx(p),dpdy=dFdy(p);vec2 dtdx=dFdx(t),dtdy=dFdy(t);float det=dtdx.x*dtdy.y-dtdx.y*dtdy.x;if(abs(det)>1e-9){vec3 T=(dpdx*dtdy.y-dpdy*dtdx.y)/det;T-=N*dot(N,T);if(length(T)>1e-6){T=normalize(T);vec3 B=normalize(cross(N,T))*sign(det)* (gl_FrontFacing?1.0:-1.0);N=normalize(mat3(T,B,N)*(texture(normalMap,t).xyz*2-1));}}}
pos=vec4(p,kind==2?2:1);norm=vec4(N,emissive);alb=vec4(hasTex?tex.rgb:baseColor,1);mat=vec4(specularColor,shine);ambient=vec4(ambientColor,reflectivity);}
