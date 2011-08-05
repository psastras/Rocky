#version 150 core
uniform mat4 modelviewMatrix;
uniform mat4 projMatrix;

#ifdef _VERTEX_
in vec3 in_Position;
in vec3 in_Normal;
in vec3 in_TexCoord;
void main(void) {
    gl_Position = projMatrix * modelviewMatrix * vec4(in_Position,1.0);
}

#endif

#ifdef _FRAGMENT_
out vec4 out_Color;
void main() {
    out_Color = vec4(1.0,1.0,1.0,1.0);
}
#endif
