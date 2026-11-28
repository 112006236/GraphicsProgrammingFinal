#version 410 core
in vec2 uv;out vec4 frag;
uniform sampler2D positions,normals,albedo,materials,sunDepth,ambientMap;
uniform samplerCube pointDepth;
uniform sampler2DArray areaDepth;
uniform mat4 areaVP[6];
uniform mat4 view,projection,lightVP,areaRotation;
uniform vec3 eye,sunDir,lightPos,areaCenter,volumePos;
uniform float pointPower,areaPower,sunPower,fog;
uniform bool shadeOn,dirOn,dirShadowOn,aoOn,pointOn,shadowOn,areaOn,volumeOn;
uniform int debugMode;
float sunVolumeVisibility(vec3 p){vec4 q=lightVP*vec4(p,1);vec3 s=q.xyz/q.w*.5+.5;if(any(lessThan(s,vec3(0)))||any(greaterThan(s,vec3(1))))return 1.0;return s.z-.001<=texture(sunDepth,s.xy).r?1.0:0.0;}
float sunVisibility(vec3 p,vec3 N){vec4 q=lightVP*vec4(p,1);vec3 s=q.xyz/q.w*.5+.5;if(any(lessThan(s,vec3(0)))||any(greaterThan(s,vec3(1))))return 1.0;float v=0;vec2 texel=1.0/vec2(textureSize(sunDepth,0));float bias=max(.0025,.006*(1-dot(N,sunDir)));for(int x=-1;x<=1;x++)for(int y=-1;y<=1;y++)v+=s.z-bias<=texture(sunDepth,s.xy+vec2(x,y)*texel).r?1:0;return v/9;}
float pointVisibility(vec3 p,float bias){vec3 delta=p-lightPos;float d=length(delta);if(d<.035)return 1.0;return d-bias<=texture(pointDepth,delta).r*10?1.0:0.0;}
float softPoint(vec3 p,vec3 N){vec3 delta=p-lightPos;float dist=length(delta);vec3 D=normalize(delta);vec3 T=normalize(cross(D,abs(D.y)>.95?vec3(1,0,0):vec3(0,1,0)));vec3 B=cross(D,T);float v=0;float bias=.012+.040*(1-max(dot(N,-D),0));vec3 offsetP=p+N*.012;float depth=length(offsetP-lightPos);for(int i=0;i<7;++i){float angle=float(i)*2.399963;vec3 o=i==0?vec3(0):.008*dist*(cos(angle)*T+sin(angle)*B);v+=depth-bias<=texture(pointDepth,offsetP-lightPos+o).r*10?1.0:0.0;}return v/7;}
float areaVisibility(vec3 p,vec3 N,int i){vec4 q=areaVP[i]*vec4(p+N*.012,1);if(q.w<=0)return 1.0;vec3 s=q.xyz/q.w*.5+.5;if(any(lessThan(s,vec3(0)))||any(greaterThan(s,vec3(1))))return 1.0;float v=0;for(int x=-1;x<=1;x++)for(int y=-1;y<=1;y++)v+=s.z-.0016<=texture(areaDepth,vec3(s.xy+vec2(x,y)/512.0,float(i))).r?1.0:0.0;return v/9;}
vec2 projectUV(vec3 p){vec4 q=projection*view*vec4(p,1);return q.xy/q.w*.5+.5;}
float ambientOcclusion(vec3 p,vec3 N){vec3 T=normalize(cross(N,abs(N.y)>.95?vec3(1,0,0):vec3(0,1,0)));vec3 B=cross(N,T);float noise=fract(sin(dot(gl_FragCoord.xy,vec2(12.9898,78.233)))*43758.5453);float occ=0;for(int i=0;i<64;i++){float f=(float(i)+.5)/64;float z=.18+.8*f;float a=float(i)*2.399963+noise*6.283185;vec3 dir=T*cos(a)*sqrt(1-z*z)+B*sin(a)*sqrt(1-z*z)+N*z;vec3 sampleP=p+dir*(.5*f);vec2 st=projectUV(sampleP);if(any(lessThan(st,vec2(0)))||any(greaterThan(st,vec2(1))))continue;vec4 actual=texture(positions,st);if(actual.a<.5)continue;float sampleZ=(view*vec4(sampleP,1)).z;float actualZ=(view*vec4(actual.xyz,1)).z;float range=1-smoothstep(.14,.55,length(actual.xyz-p));occ+=(actualZ>sampleZ+.025?1:0)*range;}return clamp(1-occ/64*1.65,.28,1);}
vec3 blinn(vec3 N,vec3 V,vec3 L,vec3 base,vec3 spec,float shine,float kd,float ks){float d=max(dot(N,L),0);vec3 H=normalize(L+V);return kd*base*d+ks*spec*pow(max(dot(N,H),0),max(shine,1))*step(.001,d);}
float mask(vec2 st,vec2 source){if(any(lessThan(st,vec2(0)))||any(greaterThan(st,vec2(1))))return 0.0;vec4 hit=texture(positions,st);float open=(hit.a<.5||hit.a>1.5)?1:0;float spot=exp(-dot(st-source,st-source)*1800);return open*(.19+spot);}
void main(){vec4 pp=texture(positions,uv);if(pp.a<.5){frag=vec4(.19,.19,.19,1);return;}vec3 p=pp.xyz,N=normalize(texture(normals,uv).xyz),base=texture(albedo,uv).rgb;vec4 m=texture(materials,uv),am=texture(ambientMap,uv);vec3 V=normalize(eye-p);float a=aoOn&&shadeOn?ambientOcclusion(p,N):1;float sv=dirOn&&dirShadowOn?sunVisibility(p,N):1;float pv=pointOn&&shadowOn?softPoint(p,N):1;
if(debugMode==1){frag=vec4(normalize(p)*.5+.5,1);return;}if(debugMode==2){frag=vec4(N*.5+.5,1);return;}if(debugMode==3){frag=vec4(am.rgb,1);return;}if(debugMode==4){frag=vec4(base,1);return;}if(debugMode==5){frag=vec4(m.rgb,1);return;}if(debugMode==6){frag=vec4(vec3(a),1);return;}if(debugMode==7){frag=vec4(vec3(sv),1);return;}if(debugMode==8){frag=vec4(vec3(pv),1);return;}if(!shadeOn){frag=vec4(base,1);return;}
float emit=texture(normals,uv).a;vec3 c=.1*am.rgb*base*a+base*emit;
if(dirOn)c+=blinn(N,V,sunDir,base,m.rgb,m.a,.7,.2)*sunPower*sv;
if(pointOn){vec3 d=lightPos-p;float r=length(d);c+=blinn(N,V,d/max(r,.001),base,m.rgb,m.a,.7,.2)*pointPower/(1+.7*r+.14*r*r)*pv;}
if(areaOn){vec3 axis=mat3(areaRotation)*vec3(1,0,0);for(int i=0;i<6;i++){vec3 q=areaCenter+mat3(areaRotation)*vec3(0,(float(i/2)-1)*.47,(float(i%2)-.5)*.48);vec3 d=q-p;float r=length(d);vec3 L=d/max(r,.001);float emitter=max(dot(-L,axis),0);c+=blinn(N,V,L,base,m.rgb,m.a,1,.2)*vec3(.8,.6,0)*areaPower*emitter*areaVisibility(p,N,i)/(6*(.7+2.1*r*r));}}
if(volumeOn){vec4 clip=projection*view*vec4(volumePos,1);if(clip.w>0){vec2 source=clip.xy/clip.w*.5+.5,delta=(uv-source)*fog/100.,st=uv;float decay=1,scatter=0;for(int i=0;i<100;i++){st-=delta;scatter+=mask(st,source)*decay*.58767*.4;decay*=.96815;}c+=vec3(scatter*.2*.25);}}
frag=vec4(max(c,0),1);}
