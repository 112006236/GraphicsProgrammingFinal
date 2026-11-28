#version 460 core
layout(location=0) in vec3 vertex;
layout(location=1) in vec3 normal;
layout(location=2) in vec2 texcoord;
layout(location=3) in float meshType;
layout(location=0) uniform mat4 modelMat;
layout(location=1) uniform int vertexProcessIdx;
layout(location=5) uniform sampler2D elevationMap;
layout(location=6) uniform sampler2D normalMap;
layout(location=7) uniform mat4 viewMat;
layout(location=8) uniform mat4 projMat;
layout(location=9) uniform mat4 terrainVToUVMat;
struct Instance {mat4 model; vec4 sphere; uvec4 info;};
layout(std430,binding=0) readonly buffer Instances{Instance instances[];};
layout(std430,binding=1) readonly buffer Visible{uint ids[];};
out vec3 world, N;out vec2 uv;flat out int type;
void main(){mat4 m=modelMat;type=int(meshType);if(vertexProcessIdx==2)m=instances[ids[gl_BaseInstance+gl_InstanceID]].model;vec4 p=m*vec4(vertex,1);uv=texcoord;N=mat3(m)*normal;if(vertexProcessIdx==3){type=7;vec4 t=terrainVToUVMat*p;uv=t.xz;p.y=texture(elevationMap,uv).r;N=texture(normalMap,uv).rgb*2-1;}world=p.xyz;gl_Position=projMat*viewMat*p;}
