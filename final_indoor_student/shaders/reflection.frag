#version 410 core
in vec2 uv;out vec4 frag;
uniform sampler2D scene,positions,normals,materials;
uniform mat4 view,projection;
uniform vec3 eye;
uniform bool enabled;
void main(){vec3 c=texture(scene,uv).rgb;vec4 pp=texture(positions,uv);float strength=texture(materials,uv).a;if(enabled&&pp.a>.5&&strength>.02){vec3 p=pp.xyz,N=normalize(texture(normals,uv).xyz),R=normalize(reflect(normalize(p-eye),N));vec3 start=p+N*.035;float oldDiff=0;for(int i=0;i<52;i++){float t=.055+float(i)*.065;vec3 q=start+R*t;vec4 clip=projection*view*vec4(q,1);if(clip.w<=0)break;vec2 st=clip.xy/clip.w*.5+.5;if(any(lessThan(st,vec2(.002)))||any(greaterThan(st,vec2(.998))))break;vec4 hit=texture(positions,st);if(hit.a<.5)continue;float diff=(view*vec4(hit.xyz,1)).z-(view*vec4(q,1)).z;if(diff>0&&diff<.16&&oldDiff<=0&&length(hit.xyz-p)>.10){float edge=smoothstep(0,.12,min(min(st.x,st.y),min(1-st.x,1-st.y)));float fresnel=.35+.65*pow(1-max(dot(N,normalize(eye-p)),0),3);c=mix(c,texture(scene,st).rgb,clamp(strength*fresnel*edge*(1-t/4),0,.85));break;}oldDiff=diff;}}frag=vec4(c,1);}
