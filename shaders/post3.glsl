#version 400 core

uniform sampler2D tex;
uniform sampler2D posTex;
uniform mat4 modelviewMatrix;
uniform mat4 projMatrix;

uniform mat4 modelviewMatrixPrev;
uniform mat4 projMatrixPrev;
uniform mat4 modelviewMatrixCurr;
uniform mat4 projMatrixCurr;

uniform bool wireframe = false;
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
    float s = 5.0;
     velocity = clamp(velocity, vec2(-0.01, -0.01)*s, vec2(0.01, 0.01)*s);
     velocity *= 0.05;
   
     vec2 texCoord = pass_TexCoord.st;
     vec4 color = texture(tex, texCoord);  
     if(abs(velocity.s) + abs(velocity.t) < 0.0035) return color;
     int nSamples = 8;
     texCoord += velocity;  
     for(int i = 1; i < nSamples; ++i, texCoord += velocity) { 
	 color += texture(tex, clamp(texCoord, vec2(0.001, 0.001), 
					       vec2(0.999, 0.999)));  
     }  
    return color / nSamples;
}


void main() {
    vec4 pos = texture(posTex, pass_TexCoord.st);
    float d1 = pos.w;
    pos.w = 1.0;
    vec4 previousPos = projMatrixPrev * modelviewMatrixPrev * pos;
    previousPos /= previousPos.w;  
    
   // vec4 currentPos = projMatrixCurr * modelviewMatrixCurr * pos;
   // currentPos /= currentPos.w;  
    vec2 currentPos = vec2(pass_TexCoord.x * 2 - 1, (pass_TexCoord.y) * 2 - 1);
    vec2 velocity = (currentPos.st-previousPos.st) * 0.5; 
    //out_Color = moblur(velocity);
    out_Color = texture(tex, pass_TexCoord.st);
    const vec4 wireframeColor = vec4(1.0, 0.0, 1.0, 0.0);
    if(wireframe) out_Color = mix(out_Color, wireframeColor, d1);
}
#endif

