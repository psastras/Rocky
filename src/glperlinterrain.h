#ifndef GLPERLINTERRAIN_H
#define GLPERLINTERRAIN_H

#include "common.h"
#include "glcommon.h"
#include <unordered_map>

using namespace std;
class GLShaderProgram;
class GLPrimitive;
class GLFramebufferObject;
class VSML;
class GLEngine;
class GLFFTWater;
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
    ~GLPerlinTerrain();
    void draw (VSML *vsml, float time);
    void drawReflection(VSML *vsml, float time);
    GLFramebufferObject *framebuffer() const { return framebuffers_[0]; } //@todo: remove this
    float lod() { return lod_; }
    void setLod(float lod) {  lod_ = lod; }
protected:
    
    void generateTerrain(VSML *vsml);
    int instances_;
    GLuint heightmap_, normalmap_;
    GLPerlinTerrainParams params_;
    GLShaderProgram *drawShader_, *perlinShader_, *lightingShader_, *reflectShader_;
    GLFramebufferObject **framebuffers_, *reflectionFramebuffer_;
    GLPrimitive *terrain_, *quad_;
    GLEngine *engine_;
    GLuint vboPosID_, vaoID_;
    
    GLFFTWater *fftwater_;
    float lod_;
};

#endif // GLPERLINTERRAIN_H
