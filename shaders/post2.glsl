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
void main() {
    vec4 color = texture(tex, pass_TexCoord.st);
    out_Color = color.xyz;
}
#endif
