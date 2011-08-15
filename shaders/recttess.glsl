#version 400 core

uniform mat4 modelviewMatrix;
uniform mat4 projMatrix;
uniform sampler3D tex;
uniform vec2 grid;
uniform float D;
uniform vec3 cameraPos;

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
    
    vec3 distance = abs(cameraPos.xyz*3.0 - vPosition[0].xyz - vPosition[1].xyz - vPosition[2].xyz);
    //distance /= 3.0;
    distance *= 0.025; //bigger = lower tesselation
    float d = 20.0 - clamp(length(distance), 0.0, 20.0);
    
    tcPosition[gl_InvocationID] = vPosition[gl_InvocationID];
    tcInstance[gl_InvocationID] = vInstance[gl_InvocationID];
    tcTexCoord[gl_InvocationID] = vTexCoord[gl_InvocationID];
    if (gl_InvocationID == 0) {
	gl_TessLevelInner[0] = int(d);
	gl_TessLevelInner[1] = int(d);
	gl_TessLevelOuter[0] = int(d);
	gl_TessLevelOuter[1] = int(d);
	gl_TessLevelOuter[2] = int(d);
	gl_TessLevelOuter[3] = int(d);
    }
}

#endif

#ifdef _TESSEVAL_
layout(quads, equal_spacing, cw) in;
in vec3 tcPosition[];
in int tcInstance[];
in vec3 tcTexCoord[];
out vec3 gTexCoord;
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
    float prec = 0.001;
    gTexCoord = vec3(clamp(tc.st, prec, 1.0-prec),z);
    gl_Position = projMatrix * modelviewMatrix * vec4(tePosition, 1);
}

#endif

#ifdef _GEOMETRY_

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;
in vec3 tePosition[3];
in vec3 tePatchDistance[3];
in vec3 gTexCoord[3];
out vec3 fTexCoord;

uniform float heightScale = 250.0;

void main() {
    
    fTexCoord = gTexCoord[0];
    gl_Position = gl_in[0].gl_Position; 
    gl_Position.y += texture(tex, fTexCoord).x * heightScale;
    EmitVertex();
    
    fTexCoord = gTexCoord[1];
    gl_Position = gl_in[1].gl_Position; 
    gl_Position.y += texture(tex, fTexCoord).x * heightScale;
    EmitVertex();
    
    fTexCoord = gTexCoord[2];
    gl_Position = gl_in[2].gl_Position; 
    gl_Position.y += texture(tex, fTexCoord).x * heightScale;
    EmitVertex();
    EndPrimitive();
}

#endif

#ifdef _FRAGMENT_
in vec3 fTexCoord;
out vec4 FragColor;

void main() {
    FragColor = vec4(texture(tex, fTexCoord).xxx+vec3(0.5,0.5,0.5), 1.0);//vec4(1.0, 1.0, 1.0, 1.0);
    //FragColor = vec4(fTexCoord.st, 0.0, 1.0);
}

#endif
