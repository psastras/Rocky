#version 400 core


uniform mat4 modelviewMatrix;
uniform mat4 projMatrix;
uniform sampler3D tex;
uniform sampler3D normalTex;
uniform sampler2D waterTex;
uniform sampler2D testTex;
uniform sampler2D sandTex;
uniform sampler2D reflTex;
uniform sampler1D permutation;
uniform sampler1D gradient;
uniform vec2 grid;
uniform float D;
uniform vec3 lightPos;
uniform vec3 cameraPos;
uniform float waterLevel = 0.0;
const float tile =0.015;
const float attenuation = 0.075;
uniform float LOD;

#ifdef _VERTEX_
in vec3 in_Position;
in vec3 in_Normal;
in vec3 in_TexCoord;
out vec3 vPosition;
out vec3 vTexCoord;
out int vInstance;

void main() {
    vPosition = in_Position;
    float idxX = gl_InstanceID / int(grid.x) - (grid.x / 2.f);
    float idxY = gl_InstanceID % int(grid.y) - (grid.y / 2.f);
    vInstance = gl_InstanceID;
    vTexCoord = in_TexCoord;
    vPosition.xz += vec2(D*idxX,D*idxY);
    gl_Position.xz = vPosition.xz;
}

#endif

#ifdef _TESSCONTROL_
layout(vertices = 4) out;
in vec3 vPosition[];
in vec3 vTexCoord[];
in int vInstance[];
out vec3 tcPosition[];
out int tcInstance[];
out vec3 tcTexCoord[];
uniform float TessLevelInner;
uniform float TessLevelOuter;

float lod(vec3 pos) {
    vec3 distance = cameraPos.xyz-(pos.xyz*0.5);
    distance *= 0.15; //bigger = lower tesselation
    float d = LOD-clamp(pow(length(distance), 0.57),0.0,LOD-1);
    return d;
}

bool offscreen(vec4 vertex){
    if((vertex.z < 0.0)) return true;
    float bound = 1.1;
    
    return any(lessThan(vertex.xy, vec2(-bound)) ||
           greaterThan(vertex.xy, vec2(bound)));
}

void main() {
    tcPosition[gl_InvocationID] = vPosition[gl_InvocationID];
    tcInstance[gl_InvocationID] = vInstance[gl_InvocationID];
    tcTexCoord[gl_InvocationID] = vTexCoord[gl_InvocationID];
    if (gl_InvocationID == 0) {
	
	float instances = grid.x * grid.y;
	float z = (vInstance[0]  + 0.5) / instances; //gl_InvocationID / instances
	vec3 size = textureSize(tex, 0);
	float prec = 0.5 / size.x;
	
	vec3 coord0 = vec3(clamp(vTexCoord[0].st, prec, 1.0-prec),z);
	vec3 coord1 = vec3(clamp(vTexCoord[1].st, prec, 1.0-prec),z);
	vec3 coord2 = vec3(clamp(vTexCoord[2].st, prec, 1.0-prec),z);
	vec3 coord3 = vec3(clamp(vTexCoord[3].st, prec, 1.0-prec),z);
	
	vec3 v0 = vPosition[0].xyz;
	vec3 v1 = vPosition[1].xyz;
	vec3 v2 = vPosition[2].xyz;
	vec3 v3 = vPosition[3].xyz;
	
	v0.y += max(texture(tex, coord0).x, waterLevel);
	v1.y += max(texture(tex, coord1).x, waterLevel);
	v2.y += max(texture(tex, coord2).x, waterLevel);
	v3.y += max(texture(tex, coord3).x, waterLevel);
	
	mat4 pmv = projMatrix*modelviewMatrix;
	vec4 ss0 = pmv*vec4(v0,1.0);
	vec4 ss1 = pmv*vec4(v1,1.0);
	vec4 ss2 = pmv*vec4(v2,1.0);
	vec4 ss3 = pmv*vec4(v3,1.0);
	ss0 /= ss0.w;
	ss1 /= ss1.w;
	ss2 /= ss2.w;
	ss3 /= ss3.w;
	
	if(all(bvec4(offscreen(ss0),
		     offscreen(ss1),
		     offscreen(ss2),
		     offscreen(ss3)
		 ))){
		     gl_TessLevelInner[0] = 0;
		     gl_TessLevelInner[1] = 0;
		     gl_TessLevelOuter[0] = 0;
		     gl_TessLevelOuter[1] = 0;
		     gl_TessLevelOuter[2] = 0;
		     gl_TessLevelOuter[3] = 0;
		 } 
	else {
	    float lod1 = lod(v0+v1);
	    float lod2 = lod(v1+v2);
	    float lod3 = lod(v2+v3);
	    float lod0 = lod(v3+v0);
	    
	    gl_TessLevelInner[0] = mix(lod1,lod2,0.5);
	    gl_TessLevelInner[1] = mix(lod0,lod3,0.5);
	    
	    //this order is messed up...
	    gl_TessLevelOuter[2] = lod0;
	    gl_TessLevelOuter[1] = lod1;
	    gl_TessLevelOuter[0] = lod2;
	    gl_TessLevelOuter[3] = lod3;
	}
    }
}

#endif

#ifdef _TESSEVAL_
layout(quads, fractional_odd_spacing, cw) in;
in vec3 tcPosition[];
in int tcInstance[];
in vec3 tcTexCoord[];
out vec3 gTexCoord;

void main() {
    
    float u = gl_TessCoord.x, v = gl_TessCoord.y;
    vec3 a = mix(tcPosition[1], tcPosition[0], u);
    vec3 b = mix(tcPosition[2], tcPosition[3], u);
    vec3 tePosition = mix(a, b, v);

    
    vec3 s = mix(tcTexCoord[1], tcTexCoord[0], u);
    vec3 t = mix(tcTexCoord[2], tcTexCoord[3], u);
    vec3 tc = mix(s, t, v);
     
    float instances = grid.x * grid.y;
    float z = (tcInstance[1]  + 0.5) / instances; //gl_InvocationID / instances
    vec3 size = textureSize(tex, 0);
    float prec = 0.5 / size.x;
    gTexCoord = vec3(clamp(tc.st, prec, 1.0-prec),z);
    float tHeight = texture(tex, gTexCoord).x;
    if(tHeight > waterLevel) {
	tePosition.y += tHeight;
    } else {
	vec3 displacement = texture(waterTex, tePosition.xz*tile).xyz;
	//attenuate y near shore
	float dH = abs(tHeight - waterLevel)*attenuation;
	//displacement.y *= min(dH, 1.0);
	tePosition.xyz += displacement;
	
    }
    gl_Position = vec4(tePosition, 1);  
}

#endif


#ifdef _GEOMETRY_

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;
in vec4 gPatchDistance[3];
in vec3 gTexCoord[3];
out vec3 fTriDistance;
out vec3 fTexCoord;
out vec4 fPosition;
void main() {
    precision lowp float;
    fTriDistance = vec3(1, 0, 0);
    fTexCoord = gTexCoord[0];
    fPosition =  gl_in[0].gl_Position;
    gl_Position =  projMatrix * modelviewMatrix * fPosition; EmitVertex();

    fTriDistance = vec3(0, 1, 0);
    fTexCoord = gTexCoord[1];
    fPosition =  gl_in[1].gl_Position;
    gl_Position =  projMatrix * modelviewMatrix * fPosition; EmitVertex();

    fTriDistance = vec3(0, 0, 1);
    fTexCoord = gTexCoord[2];
    fPosition =  gl_in[2].gl_Position;
    gl_Position =  projMatrix * modelviewMatrix * fPosition; EmitVertex();

    EndPrimitive();
}
#endif

#ifdef _FRAGMENT_
in vec3 fTexCoord;
in vec4 fPosition;
in vec3 fTriDistance;
out vec4 out_Color0;
out vec4 out_Color1;
const vec4 wireframeColor = vec4(0.7, 0.3, 0.3, 1);


float amplify(float d, float scale, float offset) {
    d = scale * d + offset;
    d = clamp(d, 0, 1);
    d = 1 - exp2(-2*d*d);
    return d;
}

vec4 computeFFTNormal(vec2 pos, float atten) {
    
    float delta =(1.0/256.0);
    pos *= tile;
    
    float p0 = texture(waterTex, (pos + vec2(0.0,-delta)) ).y * atten;
    float p1 = texture(waterTex, (pos + vec2(-delta,0.0) )).y * atten;
    float p2 = texture(waterTex, (pos + vec2(delta,0.0) ) ).y * atten;
    float p3 = texture(waterTex, (pos + vec2(0.0,delta) ) ).y * atten;
    
    return vec4(p1-p2,2.0*200.0*delta,p0-p3,0.0);
}

vec4 refractTerrain(vec3 disp, vec3 normal, float depth) { //fake refraction based on depth - we could do the real calculation but this looks good enough and is faster
    vec3 coord = fTexCoord;
    coord.xz += disp.xz*depth*depth*normal.y*0.002;
    float h = texture(tex, coord).x;
    float pVal = h*0.01+0.75;
    vec4 sand = texture(sandTex, coord.st*18.0);
    vec4 color = mix(sand, vec4(.7, .7, .6, 1.0)*pVal,1.0-max(pVal-0.5, -0.1));
    return color ;
}

vec4 reflectTerrain(vec3 wspos) {
    return vec4(0.0);
}


const float sharpness = 0.995;
const float gain = 0.65;
const float lacunarity = 1.53;
const float cover = 100;
const int octaves = 4;
uniform float time;
vec3 fade(vec3 t) {
	return t * t * t * (t * (t * 6 - 15) + 10); // new curve
}

float perm(float x) {
	return texture(permutation, x).x;
}

float grad(float x, vec3 p) {
	return dot(texture(gradient, x*16).xyz, p);
}

float inoise(vec3 p) {
	vec3 P = mod(floor(p), 256.0);
	p -= floor(p);
	vec3 f = fade(p);
	P /= 256.0;
	const float one = 1.0 / 256.0;
	float A = perm(P.x) + P.y;
	vec4 AA;
	AA.x = perm(A) + P.z;
	AA.y = perm(A + one) + P.z;
	float B =  perm(P.x + one) + P.y;
	AA.z = perm(B) + P.z;
	AA.w = perm(B + one) + P.z;
	return mix(mix(mix(grad(perm(AA.x),p),  
			   grad(perm(AA.z),p + vec3(-1,0,0)), f.x),
		       mix(grad(perm(AA.y),p + vec3(0,-1,0)),
			   grad(perm(AA.w),p + vec3(-1,-1,0)), f.x), f.y),
		 mix(mix(grad(perm(AA.x+one), p + vec3(0,0,-1)),
			     grad(perm(AA.z+one), p + vec3(-1,0,-1)),f.x),
		       mix(grad(perm(AA.y+one), p + vec3(0,-1,-1)),
			     grad(perm(AA.w+one), p + vec3(-1,-1,-1)),f.x), f.y), f.z);
}

float expfilter(float v) {
	float c = max(v - 255.0 + cover, 0.0);
	return 1.0 - pow(sharpness, c);
}

float turbulence(vec3 p, float lacunarity, float gain){
	float sum = 0;
	float freq = 1.0, amp = 1.0;
	for(int i=0; i<octaves; i++) {
		sum += abs(inoise(p*freq))*amp;
		freq *= lacunarity;
		amp *= gain;
	}
	return sum;
}

float clouds(vec3 pos) {
	float noise = turbulence(pos, lacunarity, gain);
	return expfilter(255.0f * noise);
}


vec4 atmosphere(vec3 pos) {
    vec4 c0 = vec4(0.172, 0.290, 0.486, 1.000)*1.25;
    vec4 c1 = vec4(0.321, 0.482, 0.607, 1.000)*1.25;
    vec4 s0 = vec4(5.0, 5.0, 5.0, 1.0) * 0.2; //sun color
    
    const float atmoHeight = 100.0;
    vec3 skypos = pos;
    skypos *= atmoHeight / skypos.y / atmoHeight;
    skypos.xz /= 10.0;
    skypos.xz += vec2(time, time);
    float d = length(pos - lightPos)*100.0;
    vec4 atmoColor;
    if(pos.y >= 0.0)
	 atmoColor = mix(mix(c1,c0,pos.y), s0, clamp(1.0/pow(d,1.1), 0.0, 1.0));
    else
	 atmoColor = mix(c1, s0, clamp(1.0/pow(d,1.1), 0.0, 1.0));
    
   // if(pos.y > .01) {
	float cloudCover = clouds(skypos)*2.0;
	atmoColor = mix(atmoColor, vec4(1.0), clamp(cloudCover, 0.0, 1.0));
	//atmoColor.w = 1.0;	
//    } else {
	
	//atmoColor.w = 0.0;
  //  }
    
    return atmoColor;
}

vec4 water(vec3 normal, vec4 pos, vec3 disp) {
    
    vec4 baseColor = vec4(0.133, 0.411, 0.498, 0.0);
    vec3 norm = normalize(normal);
    vec3 eyeDir	=  normalize(-cameraPos + pos.xyz);
    vec3 reflDir = normalize(reflect(eyeDir, norm));
    vec4 transColor = vec4(0.0, 0.278, 0.321, 0.0);
    float cos_angle = max(dot(norm, eyeDir), 0.3);
    
    vec4 waterColor = mix(baseColor, transColor*transColor, cos_angle);
    reflDir = normalize(reflDir);
    
    vec4 ss = projMatrix * modelviewMatrix * fPosition; 
    ss /= ss.w;
    ss += vec4(1.0);
    ss *= 0.5;
    //ss.xy += disp.xz*0.002*disp.y;
    vec4 refl = texture(reflTex, ss.st);
    vec4 atmoColor = atmosphere(reflDir)*1.5;
    if(refl.w > 0.01) atmoColor = mix(atmoColor, refl, 0.75);
    float distance = length(cameraPos.xyz - pos.xyz);
    distance *= .005;
    distance = clamp(distance, 0, 1);
    //add foam if near shore
    
    return mix(waterColor, atmoColor, 0.6*distance);
}


void main() {
  
    float h = texture(tex, fTexCoord).x;
    float pVal = (1.0-(h*0.01+0.25));
    out_Color0 = vec4(.8, .8, .7, 1.0)*pVal + vec4(0.4, 0.6, 0.1, 1.0) * clamp((1.0-pVal-0.5), 0.0, 1.0);
    vec4 sand = texture(sandTex, fTexCoord.st*56.0);
    out_Color0 = mix(sand, mix(texture(testTex, fTexCoord.st*8.0), 
                     out_Color0, min(pVal+0.1, 1.0)), 1.0-max(pVal-0.5, -0.1));
    vec3 N = texture(normalTex, fTexCoord).xyz;

    vec3 L = normalize(fPosition.xyz-lightPos);
    vec3 V = normalize(cameraPos.xyz-fPosition.xyz);
    vec3 R = normalize(reflect(L, N));
    float spec = max(pow(dot(R, V), 4.0) * 0.5 + 1.0, 0.75);
    spec = 0.0;//this needs more work
    //out_Color0 *= 0.75;
    out_Color0 += vec4(out_Color0)*spec;
    
    float NL = dot(N, L)*0.45+0.5;
    
    out_Color0.xyz  *= NL;
   //     out_Color0.xyz = N;
    float dH = abs(h - waterLevel)*attenuation;
    
    
    float waterStrength = 1.0-clamp((-h*attenuation-waterLevel),0.0,1.0);
    if(waterStrength < 1.0) {
	
	
	vec3 I = fPosition.xyz - cameraPos.xyz;
	vec3 fftN = computeFFTNormal(fPosition.xz, dH).xyz;
	vec3 displacement = texture(waterTex, fPosition.xz*tile).xyz;
	vec4 foamColor = vec4(1.0);
	vec4 ref = refractTerrain(displacement, fftN, dH);
	
	out_Color0 =
		    mix(water(fftN, fPosition, displacement), 
	                ref*NL, waterStrength);
			
    }
    
    
    out_Color1 = fPosition;
    
    out_Color0.w = 0.0;

    float d1 = min(min(fTriDistance.x, fTriDistance.y), fTriDistance.z);
    d1 = 1 - amplify(d1, 50, -1.0);
    out_Color1.w = d1;
}

#endif

