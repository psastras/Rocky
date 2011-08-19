#version 400 core

uniform mat4 modelviewMatrix;
uniform mat4 projMatrix;
uniform sampler3D tex;
uniform float layers[8]; //layers to sample from 
#ifdef _VERTEX_
in vec3 in_Position;
in vec3 in_Normal;
in vec3 in_TexCoord;
out vec3 vTexCoord;
void main() {
    vTexCoord = in_TexCoord;
    gl_Position = projMatrix*modelviewMatrix * vec4(in_Position, 1.0);
}

#endif

#ifdef _FRAGMENT_
in vec3 vTexCoord;
out vec3 out_Color0;
out vec3 out_Color1;
out vec3 out_Color2;
out vec3 out_Color3;
out vec3 out_Color4;
out vec3 out_Color5;
out vec3 out_Color6;
out vec3 out_Color7;

vec3 computeNormal(vec3 pos, float size) {
    float delta =(1.0/size);    
    float ndelta = -delta;
    float prec = (0.5 / size);
    
    float off0 = -delta;
    float off1 = -delta;
    float off2 = delta;
    float off3 = delta;
    
    float l0 = 0.0;
    float l1 = 0.0;
    float l2 = 0.0;
    float l3 = 0.0;
    
    // need to check deltas and sample from different layers if we run over
    if(pos.x + off0 <= 0.0)  { 
	l0 -= 1.0; 
	off0 = 1.0 - off0;
    }
    else if(pos.x + off0 >= 1.0) {
	l0 += 1.0;
	off0 = 1.0 - off0;
    }
    
    vec3 tc0 = pos+vec3(0.0,off0,l0);
    vec3 tc1 = pos+vec3(off1,0.0,l1);
    vec3 tc2 = pos+vec3(off2,0.0,l2);
    vec3 tc3 = pos+vec3(0.0,off3,l3);
    
    tc0.st = clamp(tc0.st, prec, 1.0-prec);
    tc1.st = clamp(tc1.st, prec, 1.0-prec);
    tc2.st = clamp(tc2.st, prec, 1.0-prec);
    tc3.st = clamp(tc3.st, prec, 1.0-prec);
    
    float p0 = texture(tex, tc0).x;
    float p1 = texture(tex, tc1).x;
    float p2 = texture(tex, tc2).x;
    float p3 = texture(tex, tc3).x;
    
    return normalize(vec3(p1-p2,4.0*delta,p0-p3));
}

void main() {
    vec3 s = textureSize(tex, 0);
    float prec = 0.5 / s.x;
    vec2 c2 = vTexCoord.st;//clamp(vTexCoord.st, prec, 1.0-prec);
    
    out_Color0 = computeNormal(vec3(c2, layers[0]), s.x);
    out_Color1 = computeNormal(vec3(c2, layers[1]), s.x);
    out_Color2 = computeNormal(vec3(c2, layers[2]), s.x);
    out_Color3 = computeNormal(vec3(c2, layers[3]), s.x);
    out_Color4 = computeNormal(vec3(c2, layers[4]), s.x);
    out_Color5 = computeNormal(vec3(c2, layers[5]), s.x);
    out_Color6 = computeNormal(vec3(c2, layers[6]), s.x);
    out_Color7 = computeNormal(vec3(c2, layers[7]), s.x);
}
#endif

