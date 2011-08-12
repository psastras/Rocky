#version 400 core

uniform sampler1D permutation;
uniform sampler1D gradient;

uniform mat4 modelviewMatrix;
uniform mat4 projMatrix;

uniform float noiseScale;
uniform int octaves;
uniform float lacunarity;
uniform float gain;
uniform float offset;

#ifdef _VERTEX_

in vec3 in_Position;
in vec3 in_Normal;
in vec3 in_TexCoord;
out vec3 pass_TexCoord;
void main() {
    pass_TexCoord = in_TexCoord * noiseScale;
    gl_Position = projMatrix * modelviewMatrix * vec4(in_Position,1.0);
}

#endif


#ifdef _FRAGMENT_
in vec3 pass_TexCoord;
out float out_Color;

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
	vec3 P = mod(floor(p), 256.0);	// FIND UNIT CUBE THAT CONTAINS POINT
	p -= floor(p);                      // FIND RELATIVE X,Y,Z OF POINT IN CUBE.
	vec3 f = fade(p);                 // COMPUTE FADE CURVES FOR EACH OF X,Y,Z.
	P = P / 256.0;
	const float one = 1.0 / 256.0;
	float A = perm(P.x) + P.y;
	vec4 AA;
	AA.x = perm(A) + P.z;
	AA.y = perm(A + one) + P.z;
	float B =  perm(P.x + one) + P.y;
	AA.z = perm(B) + P.z;
	AA.w = perm(B + one) + P.z;
	return mix(mix(mix(grad(perm(AA.x),p),  
			   grad(perm(AA.z),p + vec3(-1, 0, 0) ), f.x),
		       mix(grad(perm(AA.y),p + vec3(0, -1, 0) ),
			   grad(perm(AA.w),p + vec3(-1, -1, 0) ), f.x), f.y),
			     
		 mix( mix( grad(perm(AA.x+one), p + vec3(0, 0, -1) ),
			     grad(perm(AA.z+one), p + vec3(-1, 0, -1) ), f.x),
		       mix( grad(perm(AA.y+one), p + vec3(0, -1, -1) ),
			     grad(perm(AA.w+one), p + vec3(-1, -1, -1) ), f.x), f.y), f.z);
}

float ridge(float h, float offset) {
    h = abs(h);
    h = offset - h;
    h = h * h;
    return h;
}

float ridgedmf(vec3 p, int noctaves, float lac, float g, float off) {
	float sum = 0;
	float freq = 1.0, amp = 0.5;
	float prev = 1.0;
	for(int i=0; i<noctaves; i++) {
		float n = ridge(inoise(p*freq), off);
		sum += n*amp*prev;
		prev = n;
		freq *= lac;
		amp *= g;
	}
	return sum;
}


void main() {
	out_Color = ridgedmf(pass_TexCoord, octaves, lacunarity, gain, offset);
	out_Color = abs(texture(permutation, pass_TexCoord.x/noiseScale).x) / 255.0;
}
#endif


