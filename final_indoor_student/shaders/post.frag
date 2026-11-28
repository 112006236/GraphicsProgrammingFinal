#version 410 core
in vec2 uv;out vec4 frag;
uniform sampler2D scene,glow,normals,positions;
uniform bool bloomOn,nprOn,fxaaOn;
uniform float exposure;
uniform int debugMode;
vec3 tone(vec3 c){c*=exposure;return pow(clamp((c*(2.51*c+.03))/(c*(2.43*c+.59)+.14),0,1),vec3(1/2.2));}
vec3 colorAt(vec2 p){vec3 c=texture(scene,p).rgb;if(bloomOn)c+=texture(glow,p).rgb*.30;return tone(c);}
float luma(vec3 c){return dot(c,vec3(.299,.587,.114));}
void main(){if(debugMode!=0){frag=vec4(texture(scene,uv).rgb,1);return;}vec2 px=1.0/vec2(textureSize(scene,0));vec3 c=colorAt(uv);if(fxaaOn){vec3 nw=colorAt(uv+px*vec2(-1,1)),ne=colorAt(uv+px),sw=colorAt(uv-px),se=colorAt(uv+px*vec2(1,-1));float a=luma(nw),b=luma(ne),d=luma(sw),e=luma(se),m=luma(c);float lo=min(m,min(min(a,b),min(d,e))),hi=max(m,max(max(a,b),max(d,e)));if(hi-lo>max(.045,hi*.13)){vec2 dir=vec2(-((a+b)-(d+e)),(a+d)-(b+e));float reduce=max((a+b+d+e)*.03125,.0078125);dir=clamp(dir/(min(abs(dir.x),abs(dir.y))+reduce),vec2(-8),vec2(8))*px;vec3 A=.5*(colorAt(uv+dir*(-1.0/6))+colorAt(uv+dir*(1.0/6)));vec3 B=A*.5+.25*(colorAt(uv-dir*.5)+colorAt(uv+dir*.5));c=luma(B)<lo||luma(B)>hi?A:B;}}
if(nprOn){vec3 N=texture(normals,uv).xyz,p=texture(positions,uv).xyz;vec3 gx=vec3(0),gy=vec3(0);for(int x=-1;x<=1;x++)for(int y=-1;y<=1;y++){vec3 n=texture(normals,uv+vec2(x,y)*px).xyz;gx+=n*float(x*(2-abs(y)));gy+=n*float(y*(2-abs(x)));}float edge=(length(gx)+length(gy))*.25;float l=max(luma(c),.001);c*=floor(l*3+.5)/3/l;c=mix(c,vec3(.035,.027,.025),smoothstep(.22,.75,edge));}frag=vec4(clamp(c,0,1),1);}
