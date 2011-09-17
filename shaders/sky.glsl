#version 400 core

uniform mat4 modelviewMatrix;
uniform mat4 projMatrix;
uniform vec3 lightPos;
uniform sampler1D permutation;
uniform sampler1D gradient;

#ifdef _VERTEX_
in vec3 in_Position;
in vec3 in_Normal;
in vec3 in_TexCoord;
out vec3 vPosition;

void main() {
    vPosition = in_Position;
}

#endif

#ifdef _TESSCONTROL_
layout(vertices = 3) out;
in vec3 vPosition[];
out vec3 tcPosition[];

void main() {
    tcPosition[gl_InvocationID] = vPosition[gl_InvocationID];
    if (gl_InvocationID == 0) {
	gl_TessLevelInner[0] = 16;
	gl_TessLevelOuter[0] = 16;
	gl_TessLevelOuter[1] = 16;
	gl_TessLevelOuter[2] = 16;
    }
}

#endif

#ifdef _TESSEVAL_
layout(triangles, equal_spacing, cw) in;
in vec3 tcPosition[];
out vec3 tePosition;
void main() {
    vec3 p0 = gl_TessCoord.x * tcPosition[0];
    vec3 p1 = gl_TessCoord.y * tcPosition[1];
    vec3 p2 = gl_TessCoord.z * tcPosition[2];
    tePosition = normalize(p0 + p1 + p2)*5000.f;//length(tcPosition[0]);
    gl_Position = projMatrix * modelviewMatrix * vec4(tePosition, 1);
}

#endif

#ifdef _FRAGMENT_
in vec3 tePosition;
out vec4 out_Color0;
out vec4 out_Color1;

const float sharpness = 0.995;
const float gain = 0.65;
const float lacunarity = 1.53;
const float cover = 100;
const int octaves = 12;
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
    
    if(pos.y > .01) {
	float cloudCover = clouds(skypos);
	atmoColor = mix(atmoColor, vec4(1.0), clamp(cloudCover, 0.0, 1.0));
	atmoColor.w = 1.0;	
    } else {
	
	atmoColor.w = 0.0;
    }
    
    return atmoColor;
}

void main() {
    out_Color0 = atmosphere(tePosition / 5000);
    out_Color1 = vec4(tePosition.xyz, 0.0);
}

#endif
