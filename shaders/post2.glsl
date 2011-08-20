#version 400 core

uniform sampler2D tex;
uniform sampler2D depthTex;
uniform mat4 modelviewMatrix;
uniform mat4 projMatrix;
uniform float focus = 0.99995; //lol
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
//really opengl? no c style init?
const vec2 dof0[8] = vec2[]( vec2(0.15, 0.37), vec2(-0.37, 0.15), vec2(0.37, -0.15), 
		       vec2(-0.15, -0.37), vec2(-0.15, 0.37), vec2(0.37, 0.15),
		       vec2(-0.37, -0.15), vec2(0.15, -0.37));
const vec2 dof1[8] = vec2[] ( vec2(0.29, 0.29), vec2(0.4, 0.0), vec2(0.29, -0.29), 
		       vec2(0.0, -0.4), vec2(-0.29, 0.29), vec2(-0.4, 0.0), 
		       vec2(-0.29, -0.29), vec2(0.0, 0.4));


in vec3 pass_TexCoord;
out vec3 out_Color;
void main()  {
  vec2 s = textureSize(tex, 0);
  //const float blurclamp = 5.0;  // max blur amount
  const float bias = 60.0;
  vec2 aspectcorrect = vec2(1.0, s.x / s.y);  
  float depth = texture(depthTex, pass_TexCoord.st).x;
  float factor = (pow(depth, 1/256.) - focus);
  float k = factor*bias;//clamp( factor * bias, -blurclamp, blurclamp );
  vec2 dofblur = vec2(k,k);
  out_Color = texture(tex, pass_TexCoord.st).xyz;
  for(int i=0; i<8; i++) {
	out_Color += texture(tex, pass_TexCoord.st + (dof0[i] * aspectcorrect * dofblur*0.9)).xyz;
	out_Color += texture(tex, pass_TexCoord.st + (dof1[i] * aspectcorrect * dofblur*0.7)).xyz;
	out_Color += texture(tex, pass_TexCoord.st + (dof1[i] * aspectcorrect * dofblur*0.4)).xyz;
  }
  out_Color /= 25.0;
}
#endif
