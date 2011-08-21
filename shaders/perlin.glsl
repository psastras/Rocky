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
uniform float heightScale = 120.0;
uniform vec2 scale; //the scale of the tile (in world space)
uniform vec2 offsets[8]; //the offsets of the tiles (in world space)

#ifdef _VERTEX_

in vec3 in_Position;
in vec3 in_Normal;
in vec3 in_TexCoord;
out vec3 pass_TexCoord;
void main() {
    pass_TexCoord = in_TexCoord;
    gl_Position = projMatrix * modelviewMatrix * vec4(in_Position,1.0);
}

#endif


#ifdef _FRAGMENT_
in vec3 pass_TexCoord;
out float out_Color0;
out float out_Color1;
out float out_Color2;
out float out_Color3;
out float out_Color4;
out float out_Color5;
out float out_Color6;
out float out_Color7;


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

float ridge(float h, float offset) {
    h = abs(h);
    h = offset - h;
    h = h * h;
    return h;
}

float ridgedmf(vec3 p, int noctaves, float lac, float g, float off) {
	float sum = 0;
	float freq = 0.5, amp = .75;
	float prev = 0.25;
	for(int i=0; i<noctaves; i++) {
		float n = ridge(inoise(p*freq), off);
		sum += n*amp*prev;
		prev = n;
		freq *= lac;
		amp *= g;
	}
	return (sum-0.5)*heightScale;
}


void main() {
	float h0 = ridgedmf(vec3((pass_TexCoord.st+offsets[0])*noiseScale, 0.0), octaves, lacunarity, gain, offset);
	float h1 = ridgedmf(vec3((pass_TexCoord.st+offsets[1])*noiseScale, 0.0), octaves, lacunarity, gain, offset);
	float h2 = ridgedmf(vec3((pass_TexCoord.st+offsets[2])*noiseScale, 0.0), octaves, lacunarity, gain, offset);
	float h3 = ridgedmf(vec3((pass_TexCoord.st+offsets[3])*noiseScale, 0.0), octaves, lacunarity, gain, offset);
	float h4 = ridgedmf(vec3((pass_TexCoord.st+offsets[4])*noiseScale, 0.0), octaves, lacunarity, gain, offset);
	float h5 = ridgedmf(vec3((pass_TexCoord.st+offsets[5])*noiseScale, 0.0), octaves, lacunarity, gain, offset);
	float h6 = ridgedmf(vec3((pass_TexCoord.st+offsets[6])*noiseScale, 0.0), octaves, lacunarity, gain, offset);
	float h7 = ridgedmf(vec3((pass_TexCoord.st+offsets[7])*noiseScale, 0.0), octaves, lacunarity, gain, offset);
	out_Color0 = h0;
	out_Color1 = h1;
	out_Color2 = h2;
	out_Color3 = h3;
	out_Color4 = h4;
	out_Color5 = h5;
	out_Color6 = h6;
	out_Color7 = h7;
}
#endif


