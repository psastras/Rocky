#version 400 core

uniform mat4 modelviewMatrix;
uniform mat4 projMatrix;

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
uniform float TessLevelInner;
uniform float TessLevelOuter;

void main() {
    tcPosition[gl_InvocationID] = vPosition[gl_InvocationID];
    if (gl_InvocationID == 0) {
	/*vec4 p0 =  projMatrix * modelviewMatrix * vec4(tcPosition[0], 1);
	vec4 p1 =  projMatrix * modelviewMatrix * vec4(tcPosition[1], 1);
	vec4 p2 =  projMatrix * modelviewMatrix * vec4(tcPosition[2], 1);
	float a = int((abs(p0.x*(p1.y-p2.y)+p1.x*(p2.y-p0.y)+p2.x*(p0.y-p1.y))) * 0.5);*/
	gl_TessLevelInner[0] = TessLevelInner;
	gl_TessLevelOuter[0] = TessLevelOuter;
	gl_TessLevelOuter[1] = TessLevelOuter;
	gl_TessLevelOuter[2] = TessLevelOuter;
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
    tePosition = normalize(p0 + p1 + p2)*1000.f;//length(tcPosition[0]);
    gl_Position = projMatrix * modelviewMatrix * vec4(tePosition, 1);
}

#endif

#ifdef _FRAGMENT_
in vec3 tePosition;
out vec4 FragColor;

void main() {
    vec4 c0 = vec4(0.172, 0.290, 0.486, 1.000);
    vec4 c1 = vec4(0.321, 0.482, 0.607, 1.000);
    if(tePosition.y >= 0.0)
	FragColor = mix(c1,c0,tePosition.y / 1000.0);
    else
	FragColor = c1;
}

#endif
