#ifndef GLPERLINTERRAIN_H
#define GLPERLINTERRAIN_H

#include "common.h"
#include "glcommon.h"

class GLShaderProgram;
class GLPrimitive;
class VSML;
class GLEngine;

struct GLPerlinTerrainParams {
    
    // geometry settings
    float3 tess;
    float2 scale;
    int instances;
    
    // noise settings
    int resolution, octaves;
    float gain, lacunarity, offset;
};

class GLPerlinTerrain
{
public:
    GLPerlinTerrain(GLPerlinTerrainParams &params, GLEngine *engine);
    void draw(VSML *vsml);
protected:
    
    void generateTerrain(VSML *vsml);
    
    GLuint tex_;
    GLPerlinTerrainParams params_;
    GLShaderProgram *drawShader_, *perlinShader_;
    GLPrimitive *terrain_, *quad_;
    GLEngine *engine_;
};

#endif // GLPERLINTERRAIN_H
