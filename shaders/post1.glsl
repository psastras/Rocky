/*float4 ToneMapping( PS_INPUT psInput ) : SV_Target
{
   int2 iScreenCoord = int2(psInput.Tex * depthInfo.yz);
   float4 sample = tex2D.Load(int3(iScreenCoord, 0));
   float4 ramp = colorramp.Sample(linearSampler, float2(cOffset, 0.f));
   sample.r += ramp.r - 0.5 * (ramp.g + ramp.b);
   sample.g += ramp.g - 0.5 * (ramp.r + ramp.b);
   sample.b += ramp.b - 0.5 * (ramp.r + ramp.g);
   float Y = dot(sample, CIEXYZ);
   float Yw = 0.95;
   float Yp = exp(tex2D.Load(int3(0,0,maxMipmapLevel-1)).w); //adaptation luminance
   float Yr = alpha * Y / Yp;
   float D = (Yr * (1.0 + Yr / (Yw * Yw)) / (1 + Yr));
   return sample * D / Y;
}
*/


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
vec4 tonemap(vec4 color) {
   const vec3 luminace = vec3(0.2125f, 0.7154f, 0.0721f);
   float Y = dot(color.xyz, luminace);
   float Yw = 0.95;
   float Yp = exp(textureLod(tex, vec2(0.5, 0.5), 10.0).w); //adaptation luminance
   float Yr = alpha * Y / Yp;
   float D = (Yr * (1.0 + Yr / (Yw * Yw)) / (1 + Yr));
   return color * D / Y;
}

void main() {
    vec4 color = tonemap(texture(tex, pass_TexCoord.st));
    out_Color = color.xyz;
}
#endif
