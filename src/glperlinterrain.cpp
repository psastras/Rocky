#include "glperlinterrain.h"
#include "glengine.h"
#include "common.h"
#include "glcommon.h"
#include "glprimitive.h"
#include "glframebufferobject.h"

GLPerlinTerrain::GLPerlinTerrain(GLPerlinTerrainParams &params, GLEngine *engine) {
    params_ = params;
    engine_ = engine;
    drawShader_ = new GLShaderProgram();
    drawShader_->loadShaderFromSource(GL_VERTEX_SHADER, "shaders/recttess.glsl");
    drawShader_->loadShaderFromSource(GL_FRAGMENT_SHADER, "shaders/recttess.glsl");
    drawShader_->loadShaderFromSource(GL_TESS_CONTROL_SHADER, "shaders/recttess.glsl");
    drawShader_->loadShaderFromSource(GL_TESS_EVALUATION_SHADER, "shaders/recttess.glsl");
    drawShader_->link();
    
    terrain_ = new GLRect(float3(10, 0, 10),
			 float3(0, 0, 0),
			 float3(100, 1, 100));
    
    //quad used to render perlin noise to
    quad_ =  new GLQuad(float3(1, 1, 0),
			   float3(params_.resolution * 0.5, params_.resolution * 0.5, 0),
			   float3(params_.resolution, params_.resolution, 1));
    
    this->generateTerrain(engine_->vsml());
}

void GLPerlinTerrain::generateTerrain(VSML *vsml) {
    
    // create lookup textures
    
    int permutation[] = { 151,160,137,91,90,15,
    131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
    190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
    88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
    77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
    102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
    135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
    5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
    223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
    129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
    251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
    49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
    138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180
    };
    
    // gradients for 3d noise
    float g[] = {
	1,1,0,-1,1,0,1,-1,0,-1,-1,0,1,0,1,-1,0,1,1,0,-1,-1,0,-1,0,1,1,0,-1,1,0,1,-1,
	0,-1,-1,1,1,0,0,-1,1,-1,1,0,0,-1,-1,
    };
    
    GLuint textures[] = {0, 0};
    glGenTextures(2, &textures[0]);
    glBindTexture(GL_TEXTURE_1D, textures[0]);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexImage1D(GL_TEXTURE_1D, 0, 1, sizeof(permutation) / sizeof(int), 0, 
		 GL_LUMINANCE, GL_INT, &permutation[0]);
    glBindTexture(GL_TEXTURE_1D, textures[1]);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexImage1D(GL_TEXTURE_1D, 0, 1, sizeof(g) / sizeof(float) / 3, 0, 
		 GL_RGB, GL_FLOAT, &g[0]);
    glBindTexture(GL_TEXTURE_1D, 0);
    
    // create shaders
        
    perlinShader_ = new GLShaderProgram();
    perlinShader_->loadShaderFromSource(GL_VERTEX_SHADER, "shaders/perlin.glsl");
    perlinShader_->loadShaderFromSource(GL_FRAGMENT_SHADER, "shaders/perlin.glsl");
    perlinShader_->link();
    
    // create framebuffer
    
    GLFramebufferObjectParams params;
    params.width = params_.resolution;
    params.height = params_.resolution;
    
    params.hasDepth = false;
    params.nSamples = 0;
    params.type = GL_TEXTURE_3D;
    params.nColorAttachments = GLFramebufferObject::queryMaxAttachments();
    params.format = 1;
    framebuffer_ = new GLFramebufferObject(params);
    engine_->vsmlOrtho(params_.resolution, params_.resolution);
    
    // create quad
    GLPrimitive *quad = new GLQuad(float3(1, 1, 0),
				   float3(params.width * 0.5, params.height * 0.5, 0),
				   float3(params.width, params.height, 1));
    
    // draw
    
    int width = engine_->width();
    int height = engine_->height();
    
    glViewport(0, 0, params.width, params.height);
    
    framebuffer_->bind();
    perlinShader_->bind(vsml);    
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_1D, textures[0]);
    perlinShader_->setUniformValue("permutation", 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_1D, textures[1]);
    perlinShader_->setUniformValue("gradient", 1);
    glActiveTexture(GL_TEXTURE0);
    
    perlinShader_->setUniformValue("noiseScale", params_.scale);
    perlinShader_->setUniformValue("octaves", params_.octaves);
    perlinShader_->setUniformValue("lacunarity", params_.lacunarity);
    perlinShader_->setUniformValue("gain", params_.gain);
    perlinShader_->setUniformValue("offset", params_.offset);
    
    quad->draw(perlinShader_);
    
    perlinShader_->release();
    
	    
    framebuffer_->release();
    
    glViewport(0, 0, width, height); // restore the viewport
    
    glDeleteTextures(2, &textures[0]);
    delete quad;
}


void GLPerlinTerrain::draw(VSML *vsml) {
    
    float distance = 1.f / (abs(engine_->camera()->eye.y) + 0.001f);
    float tess = (float)min((max((int)(distance * 500), 3)), 25);
    
    drawShader_->bind(vsml);
    drawShader_->setUniformValue("TessLevelInner", tess);
    drawShader_->setUniformValue("TessLevelOuter", tess);
    drawShader_->setUniformValue("grid", float2(4, 4));
    drawShader_->setUniformValue("D", terrain_->scale().x);
    terrain_->draw(drawShader_, 16);
    drawShader_->release();
}
