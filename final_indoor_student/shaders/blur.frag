#version 410 core
in vec2 uv;out vec4 frag;
uniform sampler2D source;
uniform vec2 direction;
uniform bool extractBright;
vec3 sampleColor(vec2 t){vec3 c=texture(source,t).rgb;if(extractBright)c*=max(max(c.r,max(c.g,c.b))-.65,0)/max(max(c.r,max(c.g,c.b)),.001);return c;}
void main(){vec2 stepUV=direction/vec2(textureSize(source,0));vec3 c=sampleColor(uv)*.227027;c+=(sampleColor(uv+stepUV*1.384615)+sampleColor(uv-stepUV*1.384615))*.316216;c+=(sampleColor(uv+stepUV*3.230769)+sampleColor(uv-stepUV*3.230769))*.070270;frag=vec4(c,1);}
