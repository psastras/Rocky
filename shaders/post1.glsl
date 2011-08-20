#version 400 core

uniform sampler2D tex;

uniform mat4 modelviewMatrix;
uniform mat4 projMatrix;

#ifdef _VERTEX_
in vec3 in_Position;
in vec3 in_Normal;
in vec3 in_TexCoord;
out vec3 pass_TexCoord;
void main(void) {
    pass_TexCoord  = in_TexCoord;
    gl_Position = projMatrix * modelviewMatrix * vec4(in_Position,1.0);
}

#endif

#ifdef _FRAGMENT_
in vec3 pass_TexCoord;
out vec3 out_Color;
uniform float alpha = 0.75;
uniform float maxMipLevel;

vec4 tonemap(vec4 color) {
   const vec3 luminace = vec3(0.2125f, 0.7154f, 0.0721f);
   float Y = dot(color.xyz, luminace);
   float Yw = 0.95;
   float Yp = exp(textureLod(tex, vec2(0.5, 0.5), maxMipLevel).w); //adaptation luminance
   float Yr = alpha * Y / Yp;
   float D = (Yr * (1.0 + Yr / (Yw * Yw)) / (1 + Yr));
   return color * D / Y;
}


void main() {
    vec4 color = tonemap(texture(tex, pass_TexCoord.st));
    out_Color = color.xyz;
}
#endif
