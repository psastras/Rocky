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
    gl_Position.xz = vPosition.xz;
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

float lod(vec3 pos) {
    vec3 distance = cameraPos.xyz-(pos.xyz*0.5);
    //distance /= 3.0;
    distance *= 0.04; //bigger = lower tesselation
    float d = 20.0 - clamp(length(distance), 0.0, 20.0);
    return d;
}


void main() {
    tcPosition[gl_InvocationID] = vPosition[gl_InvocationID];
    tcInstance[gl_InvocationID] = vInstance[gl_InvocationID];
    tcTexCoord[gl_InvocationID] = vTexCoord[gl_InvocationID];
    if (gl_InvocationID == 0) {
	
	vec3 v0 = vPosition[0].xyz;
	vec3 v1 = vPosition[1].xyz;
	vec3 v2 = vPosition[2].xyz;
	vec3 v3 = vPosition[3].xyz;
	
	float lod1 = lod(v0+v1);
	float lod2 = lod(v1+v2);
	float lod3 = lod(v2+v3);
	float lod0 = lod(v3+v0);
	
	gl_TessLevelInner[0] = mix(lod1,lod2,0.5);
	gl_TessLevelInner[1] = mix(lod0,lod3,0.5);
	
	//this order is messed up...
	gl_TessLevelOuter[2] = lod0;
	gl_TessLevelOuter[1] = lod1;
	gl_TessLevelOuter[0] = lod2;
	gl_TessLevelOuter[3] = lod3;
    }
}

#endif

#ifdef _TESSEVAL_
layout(quads, equal_spacing, cw) in;
in vec3 tcPosition[];
in int tcInstance[];
in vec3 tcTexCoord[];
out vec3 fTexCoord;
uniform float heightScale = 100.0;
uniform vec2 textureSizeI = 1.0 / vec2(512.0, 512.0);
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
    float prec = 0.000975;
    
    //determine if we hit the last pix?
    

    
    
    
    fTexCoord = vec3(clamp(tc.st, prec, 1.0-prec),z);
    tePosition.y += texture(tex, fTexCoord).x * heightScale;
    gl_Position = projMatrix * modelviewMatrix * vec4(tePosition, 1);
    
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
