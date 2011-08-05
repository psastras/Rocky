#version 400 core

uniform mat4 modelviewMatrix;
uniform mat4 projMatrix;

#ifdef _VERTEX_
in vec3 in_Position;
in vec3 in_Normal;
in vec3 in_TexCoord;
out vec3 vPosition;
uniform vec2 grid;
uniform float D;
void main() {
    vPosition = in_Position;
    float idxX = gl_InstanceID / int(grid.x) - (grid.x / 2.f);
    float idxY = gl_InstanceID % int(grid.y) - (grid.y / 2.f);
    
    vPosition.xz += vec2(D*idxX,D*idxY);
}

#endif

#ifdef _TESSCONTROL_
layout(vertices = 4) out;
in vec3 vPosition[];
out vec3 tcPosition[];
uniform float TessLevelInner;
uniform float TessLevelOuter;

//@todo: inner level should be det by distance to camera (in shader), while oter remain
// uniform

void main() {
    tcPosition[gl_InvocationID] = vPosition[gl_InvocationID];
    if (gl_InvocationID == 0) {
	gl_TessLevelInner[0] = TessLevelInner;
	gl_TessLevelInner[1] = TessLevelInner;
	gl_TessLevelOuter[0] = TessLevelOuter;
	gl_TessLevelOuter[1] = TessLevelOuter;
	gl_TessLevelOuter[2] = TessLevelOuter;
	gl_TessLevelOuter[3] = TessLevelOuter;
    }
}

#endif

#ifdef _TESSEVAL_
layout(quads, equal_spacing, cw) in;
in vec3 tcPosition[];

void main() {
    float u = gl_TessCoord.x, v = gl_TessCoord.y;
    vec3 a = mix(tcPosition[1], tcPosition[0], u);
    vec3 b = mix(tcPosition[2], tcPosition[3], u);
    vec3 tePosition = mix(a, b, v);
    
   
    
    gl_Position = projMatrix * modelviewMatrix * vec4(tePosition, 1);
}

#endif

#ifdef _FRAGMENT_
out vec4 FragColor;

void main() {
    FragColor = vec4(1.0, 1.0, 1.0, 1.0);
}

#endif
