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
    gl_Position = vec4(in_Position, 1.0);
}

#endif

#ifdef _FRAGMENT_
in vec3 vTexCoord;
out float out_Color0;
out float out_Color1;
out float out_Color2;
out float out_Color3;
out float out_Color4;
out float out_Color5;
out float out_Color6;
out float out_Color7;

void main() {
    vec3 size = textureSize(tex, 0);
    float prec = 0.5 / size.x;
    vec2 c2 = clamp(vTexCoord.st, prec, 1.0-prec);
    
    float h0 = -10000.0;//texture(tex, vec3(c2, layers[0])).x;
    float h1 = texture(tex, vec3(c2, layers[1])).x;
    float h2 = texture(tex, vec3(c2, layers[2])).x;
    float h3 = texture(tex, vec3(c2, layers[3])).x;
    float h4 = texture(tex, vec3(c2, layers[4])).x;
    float h5 = texture(tex, vec3(c2, layers[5])).x;
    float h6 = texture(tex, vec3(c2, layers[6])).x;
    float h7 = texture(tex, vec3(c2, layers[7])).x;
    
    out_Color0 = (h0);
    out_Color1 = (h1);
    out_Color2 = (h2);
    out_Color3 = (h3);
    out_Color4 = (h4);
    out_Color5 = (h5);
    out_Color6 = (h6);
    out_Color7 = (h7);
}
#endif

