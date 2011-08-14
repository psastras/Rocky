#version 400 core

uniform mat4 modelviewMatrix;
uniform mat4 projMatrix;
uniform sampler3D tex;
uniform vec2 grid;
uniform float D;

#ifdef _VERTEX_
in vec3 in_Position;
in vec3 in_Normal;
in vec3 in_TexCoord;
out vec3 vPosition;
out vec3 vTexCoord;
out int vInstance;
void main() {
    vPosition = in_Position;
    float idxX = gl_InstanceID / int(grid.x) - (grid.x / 2.f);
    float idxY = gl_InstanceID % int(grid.y) - (grid.y / 2.f);
    vInstance = gl_InstanceID;
    vTexCoord = in_TexCoord;
    vPosition.xz += vec2(D*idxX,D*idxY);
}

#endif

#ifdef _TESSCONTROL_
layout(vertices = 4) out;
in vec3 vPosition[];
in vec3 vTexCoord[];
in int vInstance[];
out vec3 tcPosition[];
out int tcInstance[];
out vec3 tcTexCoord[];
uniform float TessLevelInner;
uniform float TessLevelOuter;

//@todo: inner level should be det by distance to camera (in shader), while oter remain
// uniform

void main() {
    tcPosition[gl_InvocationID] = vPosition[gl_InvocationID];
    tcInstance[gl_InvocationID] = vInstance[gl_InvocationID];
    tcTexCoord[gl_InvocationID] = vTexCoord[gl_InvocationID];
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
in int tcInstance[];
in vec3 tcTexCoord[];
out vec3 fTexCoord;
void main() {
    float u = gl_TessCoord.x, v = gl_TessCoord.y;
    vec3 a = mix(tcPosition[1], tcPosition[0], u);
    vec3 b = mix(tcPosition[2], tcPosition[3], u);
    vec3 tePosition = mix(a, b, v);

    
    vec3 s = mix(tcTexCoord[1], tcTexCoord[0], u);
    vec3 t = mix(tcTexCoord[2], tcTexCoord[3], u);
    vec3 tc = mix(s, t, v);
     
    float instances = grid.x * grid.y;
    float z = (tcInstance[1]  + 0.5) / instances; //gl_InvocationID / instances
    fTexCoord = vec3(tc.st,z);
    gl_Position = projMatrix * modelviewMatrix * vec4(tePosition, 1);
}

#endif

#ifdef _FRAGMENT_
in vec3 fTexCoord;
out vec4 FragColor;

void main() {
    FragColor = vec4(texture(tex, fTexCoord).xxx, 1.0);//vec4(1.0, 1.0, 1.0, 1.0);
    //FragColor = vec4(fTexCoord.st, 0.0, 1.0);
}

#endif
