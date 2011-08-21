#version 400 core

uniform mat4 modelviewMatrix;
uniform mat4 projMatrix;
uniform sampler3D tex;
uniform sampler3D normalTex;
uniform sampler2D testTex;
uniform sampler2D sandTex;
uniform vec2 grid;
uniform float D;
uniform vec3 lightPos;
uniform vec3 cameraPos;
uniform float waterLevel = 0.0;
const float tile =0.015;
const float attenuation = 0.075;
uniform float LOD;

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
    distance *= 0.09; //bigger = lower tesselation
    float d = LOD-clamp(pow(length(distance), 0.8),0.0,LOD-1);
    return d;
}

bool offscreen(vec4 vertex){
    if((vertex.z < -0.5)) return true;
    float bound = 1.1;
    
    return any(lessThan(vertex.xy, vec2(-bound)) ||
           greaterThan(vertex.xy, vec2(bound)));
}

void main() {
    tcPosition[gl_InvocationID] = vPosition[gl_InvocationID];
    tcInstance[gl_InvocationID] = vInstance[gl_InvocationID];
    tcTexCoord[gl_InvocationID] = vTexCoord[gl_InvocationID];
    if (gl_InvocationID == 0) {
	
	float instances = grid.x * grid.y;
	float z = (vInstance[0]  + 0.5) / instances; //gl_InvocationID / instances
	vec3 size = textureSize(tex, 0);
	float prec = 0.5 / size.x;
	
	vec3 coord0 = vec3(clamp(vTexCoord[0].st, prec, 1.0-prec),z);
	vec3 coord1 = vec3(clamp(vTexCoord[1].st, prec, 1.0-prec),z);
	vec3 coord2 = vec3(clamp(vTexCoord[2].st, prec, 1.0-prec),z);
	vec3 coord3 = vec3(clamp(vTexCoord[3].st, prec, 1.0-prec),z);
	
	vec3 v0 = vPosition[0].xyz;
	vec3 v1 = vPosition[1].xyz;
	vec3 v2 = vPosition[2].xyz;
	vec3 v3 = vPosition[3].xyz;
	
	v0.y -= max(texture(tex, coord0).x, waterLevel);
	v1.y -= max(texture(tex, coord1).x, waterLevel);
	v2.y -= max(texture(tex, coord2).x, waterLevel);
	v3.y -= max(texture(tex, coord3).x, waterLevel);
	
	mat4 pmv = projMatrix*modelviewMatrix;
	vec4 ss0 = pmv*vec4(v0,1.0);
	vec4 ss1 = pmv*vec4(v1,1.0);
	vec4 ss2 = pmv*vec4(v2,1.0);
	vec4 ss3 = pmv*vec4(v3,1.0);
	ss0 /= ss0.w;
	ss1 /= ss1.w;
	ss2 /= ss2.w;
	ss3 /= ss3.w;
	
	if(all(bvec4(offscreen(ss0),
		     offscreen(ss1),
		     offscreen(ss2),
		     offscreen(ss3)
		 ))){
		     gl_TessLevelInner[0] = 0;
		     gl_TessLevelInner[1] = 0;
		     gl_TessLevelOuter[0] = 0;
		     gl_TessLevelOuter[1] = 0;
		     gl_TessLevelOuter[2] = 0;
		     gl_TessLevelOuter[3] = 0;
		 } 
	else {
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
}

#endif

#ifdef _TESSEVAL_
layout(quads, fractional_odd_spacing, cw) in;
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
    vec3 size = textureSize(tex, 0);
    float prec = 0.5 / size.x;
    gTexCoord = vec3(clamp(tc.st, prec, 1.0-prec),z);
    float tHeight = texture(tex, gTexCoord).x;
    if(tHeight > waterLevel) {
	tePosition.y -= tHeight;
    }else {
	//tePosition.y = 10.0; //lulz
    }
    gl_Position = vec4(tePosition, 1);  
}

#endif


#ifdef _GEOMETRY_
layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;
in vec4 gPatchDistance[3];
in vec3 gTexCoord[3];
out vec3 fTriDistance;
out vec3 fTexCoord;
out vec4 fPosition;
void main() {
    float adjust = 0.1;
    if(gl_in[0].gl_Position.y >= 0 && gl_in[1].gl_Position.y  >= 0&&
            gl_in[2].gl_Position.y >= 0) return;
    
    fTriDistance = vec3(1, 0, 0);
    fTexCoord = gTexCoord[2];
    fPosition =  gl_in[2].gl_Position;
    fPosition.y += adjust;
    gl_Position =  projMatrix * modelviewMatrix * fPosition; EmitVertex();

    fTriDistance = vec3(0, 1, 0);
    fTexCoord = gTexCoord[1];
    fPosition =  gl_in[1].gl_Position;
    fPosition.y += adjust;
    gl_Position =  projMatrix * modelviewMatrix * fPosition; EmitVertex();

    fTriDistance = vec3(0, 0, 1);
    fTexCoord = gTexCoord[0];
    fPosition =  gl_in[0].gl_Position;
    fPosition.y += adjust;
    gl_Position =  projMatrix * modelviewMatrix * fPosition; EmitVertex();

    EndPrimitive();
}
#endif

#ifdef _FRAGMENT_
in vec3 fTexCoord;
in vec4 fPosition;
in vec3 fTriDistance;
out vec4 out_Color0;

void main() {
  
    float h = texture(tex, fTexCoord).x;
    float pVal = (1.0-(h*0.01+0.25));
    out_Color0 = vec4(.7, .7, .6, 1.0)*pVal + vec4(0.3, 0.5, 0.1, 1.0) * clamp((1.0-pVal-0.5), 0.0, 1.0);
    vec4 sand = texture(sandTex, fTexCoord.st*56.0);
    out_Color0 = mix(sand, mix(texture(testTex, fTexCoord.st*8.0), 
                     out_Color0, min(pVal+0.1, 1.0)), 1.0-max(pVal-0.5, -0.1));
    vec3 N = texture(normalTex, fTexCoord).xyz;
    N.y *= -1;
    vec3 L = normalize(fPosition.xyz-lightPos);

    float NL = dot(N, L)*0.35+0.5;
    out_Color0.xyz *= NL;
    
    
    out_Color0.w = 1.0;
    //out_Color0 = vec4(0.0);

}

#endif

