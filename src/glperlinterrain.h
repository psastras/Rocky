#ifndef GLPERLINTERRAIN_H
#define GLPERLINTERRAIN_H

#include "common.h"
#include "glcommon.h"

class GLShaderProgram;
class GLPrimitive;
class GLFramebufferObject;
class VSML;
class GLEngine;

struct GLPerlinTerrainParams {
    
    // geometry settings
    float3 tess;
    float2 scale;
    float2 grid;
    
    // noise settings
    int resolution, octaves;
    float gain, lacunarity, offset, noiseScale;
};

class GLPerlinTerrain
{
public:
    GLPerlinTerrain(GLPerlinTerrainParams &params, GLEngine *engine);
    void draw (VSML *vsml);
    
    GLFramebufferObject *framebuffer() const { return framebuffers_[0]; } //@todo: remove this
    
protected:
    
    void generateTerrain(VSML *vsml);
    int instances_;
    GLuint heightmap_, normalmap_;
    GLPerlinTerrainParams params_;
    GLShaderProgram *drawShader_, *perlinShader_;
    GLFramebufferObject **framebuffers_;
    GLPrimitive *terrain_, *quad_;
    GLEngine *engine_;
};

#endif // GLPERLINTERRAIN_H
