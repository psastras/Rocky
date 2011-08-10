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
    /*
    //generate perlin textures
    glGenTextures(1, &tex_);
    glBindTexture(GL_TEXTURE_3D, tex_);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);
    //glReadBuffer();
    glTexImage3D(GL_TEXTURE_3D, 0, 1, params_.resolution, params_.resolution, 
		 params_.instances, 0, GL_LUMINANCE, GL_FLOAT, 0);
    glBindTexture(GL_TEXTURE_3D, 0);
    
    //glCopyTexSubImage3D
    */
    
    perlinShader_ = new GLShaderProgram();
    
    
    GLFramebufferObjectParams params;
    params.width = params_.resolution;
    params.height = params_.resolution;
    
    params.hasDepth = false;
    params.nSamples = 0;
    params.type = GL_TEXTURE_3D;
    params.nColorAttachments = GLFramebufferObject::queryMaxAttachments();
    params.format = 1;
    GLFramebufferObject *fbo = new GLFramebufferObject(params);
    engine_->vsmlOrtho(params_.resolution, params_.resolution);
    
    return;
    
    fbo->bind();
    perlinShader_->bind();    
    
    
    perlinShader_->release();
    fbo->release();
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
