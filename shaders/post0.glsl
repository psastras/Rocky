#version 400 core

uniform sampler2D tex;
uniform sampler2D posTex;
uniform mat4 modelviewMatrix;
uniform mat4 projMatrix;

uniform vec3 lightPos;
uniform mat4 modelviewMatrixPrev;
uniform mat4 projMatrixPrev;
uniform mat4 modelviewMatrixCurr;
uniform mat4 projMatrixCurr;
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
out vec4 out_Color;

vec4 moblur(vec2 velocity) {
    velocity = clamp(velocity, vec2(-0.003, -0.003), vec2(0.003, 0.003));
     vec2 texCoord = pass_TexCoord.st;
     vec4 color = texture(tex, texCoord);  
     int nSamples = 3;
     texCoord += velocity;  
     for(int i = 1; i < nSamples; ++i, texCoord += velocity) { 
	 color += texture(tex, clamp(texCoord, vec2(0.001, 0.001), 
					       vec2(0.999, 0.999)));  
     }  
    return color / nSamples;
}


vec3 godrays(vec2 texCoord) {  
    vec4 lightPosSS = projMatrixCurr * modelviewMatrixCurr * vec4(lightPos*5000, 1.0);
    lightPosSS /= lightPosSS.w;
    float Exposure = .001f;
     
    vec3 color = vec3(0.0);
    
    if(lightPosSS.z < 1.0) { //if were facing the light source
	lightPosSS += 1.0;
	lightPosSS /= 2.0;
	const int samples = 70;
	float density = 0.975;
	float Weight = 6.65;
	
	float Decay = 1.0;
	vec2 deltaTexCoord = (texCoord - lightPosSS.xy);  
	deltaTexCoord *= 1.0f / samples * density;  
	vec2 tc = texCoord;
	float illuminationDecay = 1.0;  
	for (int i = 0; i < samples; i++) {  
	    tc -= deltaTexCoord;  
	    vec4 textureColor = texture(tex, tc);
	    vec3 sampled = textureColor.xyz * textureColor.w;
	    sampled *= illuminationDecay * Weight;  
	    color += sampled;  
	    illuminationDecay *= Decay;  
	}  
    }

    //return vec3(color * Exposure);
    return vec3(texture(tex, texCoord).xyz + color * Exposure);  
}  

void main() {
 /*   vec4 pos = texture(posTex, pass_TexCoord.st);
    vec4 previousPos = projMatrixPrev * modelviewMatrixPrev * pos;
    previousPos /= previousPos.w;  
    vec2 currentPos = vec2(pass_TexCoord.x * 2 - 1, (pass_TexCoord.y) * 2 - 1);
    vec2 velocity = (currentPos-previousPos.st) * 0.5; 
    out_Color = moblur(velocity).xyz;*/
  //  out_Color = texture(tex, pass_TexCoord.st).xyz*1.1;

    const vec3 luminace = vec3(0.2125f, 0.7154f, 0.0721f);
    out_Color = vec4(godrays(pass_TexCoord.st), 1.0);
    out_Color.w = log(dot(out_Color.xyz, luminace)+0.1);
}
#endif


//vec3 color = texture(tex, vec3(pass_TexCoord.st, 1.0)).xxx;
