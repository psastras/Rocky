#ifndef GLSKYDOME_H
#define GLSKYDOME_H

#include "glcommon.h"

class GLPrimitive;
class GLShaderProgram;
class VSML;
class GLEngine;

class GLSkyDome { 
public:
    GLSkyDome( GLEngine *engine);
    ~GLSkyDome();
    
    void draw(VSML *vsml, float time);
    GLuint *perlinTextures() { return &perlinTextures_[0]; }
    
protected:    
    void init();
    
    GLPrimitive *domePrimitive_;
    GLShaderProgram *skyShader_;
    GLEngine *engine_;
    GLuint perlinTextures_[2];
};

#endif // GLSKYDOME_H
