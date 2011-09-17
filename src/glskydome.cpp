#include "glskydome.h"
#include "glprimitive.h"
#include "vsml.h"
#include "glengine.h"

GLSkyDome::GLSkyDome(GLEngine *engine) {
    engine_ = engine;
    this->init();
}

GLSkyDome::~GLSkyDome() {
    delete domePrimitive_;
    delete skyShader_;
    
    glDeleteTextures(2, &perlinTextures_[0]);
}


void GLSkyDome::init() {
    domePrimitive_ = new GLIcosohedron(float3::zero(), float3::zero(), float3(5000, 5000, 5000));    
    skyShader_ = new GLShaderProgram();
    skyShader_->loadShaderFromSource(GL_VERTEX_SHADER, "shaders/sky.glsl");
    skyShader_->loadShaderFromSource(GL_FRAGMENT_SHADER, "shaders/sky.glsl");
    skyShader_->loadShaderFromSource(GL_TESS_CONTROL_SHADER, "shaders/sky.glsl");
    skyShader_->loadShaderFromSource(GL_TESS_EVALUATION_SHADER, "shaders/sky.glsl");
    skyShader_->link();    
    
    skyShader_->setFragDataLocation("out_Color0", 0);
    skyShader_->setFragDataLocation("out_Color1", 1);
    
    
    // create lookup textures
    
    float permutation[256] = { 151,160,137,91,90,15,
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
    float g[] = {1,1,0,-1,1,0,1,-1,0,-1,-1,0,1,0,1,-1,0,1,1,0,-1,-1,0,-1,0,1,1,
		 0,-1,1,0,1,-1,	0,-1,-1,1,1,0,0,-1,1,-1,1,0,0,-1,-1};
    
    for(int i=0;i<256;i++) permutation[i] /= 255.0;

    glGenTextures(2, &perlinTextures_[0]);
    glBindTexture(GL_TEXTURE_1D, perlinTextures_[0]);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_R16F, sizeof(permutation) / sizeof(float), 0, 
		 GL_LUMINANCE, GL_FLOAT, &permutation[0]);
    

    glBindTexture(GL_TEXTURE_1D, perlinTextures_[1]);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB16F, sizeof(g) / sizeof(float) / 3, 0, 
		 GL_RGB, GL_FLOAT, &g[0]);
    glBindTexture(GL_TEXTURE_1D, 0);
}


void GLSkyDome::draw(VSML *vsml, float time) {
    GLenum outputTex[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1}; 
    skyShader_->bind(vsml);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_1D, perlinTextures_[0]);
    skyShader_->setUniformValue("permutation", 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_1D, perlinTextures_[1]);
    skyShader_->setUniformValue("gradient", 1);
    glActiveTexture(GL_TEXTURE0);
    skyShader_->setUniformValue("lightPos", engine_->light());
    skyShader_->setUniformValue("time", time / 50.0f);
    glDrawBuffers(2, outputTex); 
    domePrimitive_->draw(skyShader_);
}
