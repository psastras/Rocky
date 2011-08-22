#include "glperlinterrain.h"
#include "glengine.h"
#include "common.h"
#include "glcommon.h"
#include "glprimitive.h"
#include "glframebufferobject.h"
#include "glfftwater.h"
#include "gltextureloader.h"
#include <IL/il.h>
GLPerlinTerrain::GLPerlinTerrain(GLPerlinTerrainParams &params, GLEngine *engine) {
    params_ = params;
    engine_ = engine;
    drawShader_ = new GLShaderProgram();
    drawShader_->loadShaderFromSource(GL_VERTEX_SHADER, "shaders/recttess.glsl");
    drawShader_->loadShaderFromSource(GL_FRAGMENT_SHADER, "shaders/recttess.glsl");
    drawShader_->loadShaderFromSource(GL_GEOMETRY_SHADER, "shaders/recttess.glsl");
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
    
    GLFFTWaterParams fftparams;
    fftparams.A = 0.0000005f;
    fftparams.V = 10.0f;
    fftparams.w = 200 * 3.14159f / 180.0f;
    fftparams.L = 200.0;
    fftparams.N = 256;
    fftparams.chop = 2.0;
    fftwater_ = new GLFFTWater(fftparams);
    fftwater_->startHeightfieldComputeThread(0.f);
    lod_ = 20.f;
    
   //182,790,400
    reflectShader_ = new GLShaderProgram();
    reflectShader_->loadShaderFromSource(GL_VERTEX_SHADER, "shaders/reflect.glsl");
    reflectShader_->loadShaderFromSource(GL_FRAGMENT_SHADER, "shaders/reflect.glsl");
    reflectShader_->loadShaderFromSource(GL_GEOMETRY_SHADER, "shaders/reflect.glsl");
    reflectShader_->loadShaderFromSource(GL_TESS_EVALUATION_SHADER, "shaders/reflect.glsl");
    reflectShader_->loadShaderFromSource(GL_TESS_CONTROL_SHADER, "shaders/reflect.glsl");
    reflectShader_->link();
    
    drawShader_->setFragDataLocation("out_Color0", 0);
    drawShader_->setFragDataLocation("out_Color1", 1);
    reflectShader_->setFragDataLocation("out_Color0", 0);
    //reflectionFramebuffer_ = new GLFramebufferObject();
    
    this->generateTerrain(engine_->vsml());
}

GLPerlinTerrain::~GLPerlinTerrain() {
    delete terrain_; 
    delete quad_;
    delete drawShader_;
    delete fftwater_;
    delete reflectShader_;
    glDeleteTextures(1, &normalmap_);
    glDeleteTextures(1, &heightmap_);
    delete[] framebuffers_;
 
}

void GLPerlinTerrain::generateTerrain(VSML *vsml) {
  
    // there be dragons ahead
    
    int maxAttachments = 8;//GLFramebufferObject::queryMaxAttachments(); /* your platform better support 8 attachmenets or too bad */
    instances_ = params_.grid.x * params_.grid.y;
    int noBuffers = (int)ceilf(instances_ / (float)maxAttachments);
    framebuffers_ = new GLFramebufferObject*[noBuffers];
    
    //create the framebuffer
    
    GLFramebufferObjectParams params;
    params.width = params_.resolution;
    params.height = params_.resolution;
    
    params.hasDepth = false;
    params.nSamples = 0;
    params.type = GL_TEXTURE_3D;
    params.nColorAttachments = 0;
    params.format = GL_R16F;
    
    //create the 3d textire
    glGenTextures(1, &heightmap_);
    glBindTexture(GL_TEXTURE_3D, heightmap_);
    glTexParameterf(params.type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(params.type, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_R16F, params_.resolution, params_.resolution, 
		 instances_, 0, GL_LUMINANCE, GL_FLOAT, 0);
    glBindTexture(GL_TEXTURE_3D, 0);

   
    
    
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

    GLuint textures[] = {0, 0};
    glGenTextures(2, &textures[0]);
    glBindTexture(GL_TEXTURE_1D, textures[0]);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_R16F, sizeof(permutation) / sizeof(float), 0, 
		 GL_LUMINANCE, GL_FLOAT, &permutation[0]);
    

    glBindTexture(GL_TEXTURE_1D, textures[1]);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB16F, sizeof(g) / sizeof(float) / 3, 0, 
		 GL_RGB, GL_FLOAT, &g[0]);
    glBindTexture(GL_TEXTURE_1D, 0);
    
    // create shaders
        
    perlinShader_ = new GLShaderProgram();
    perlinShader_->loadShaderFromSource(GL_VERTEX_SHADER, "shaders/perlin.glsl");
    perlinShader_->loadShaderFromSource(GL_FRAGMENT_SHADER, "shaders/perlin.glsl");
    perlinShader_->link();
    

   

    
    // create quad
    GLPrimitive *quad = new GLQuad(float3(1, 1, 0),
				   float3(params.width * 0.5, params.height * 0.5, 0),
				   float3(params.width, params.height, 1));
    
    // draw
    float2 random = params_.random;
    float2 *offsets = new float2[instances_ + (instances_ % 8)];
    for(int x=0, i=0; x<params_.grid.x; x++) {
	for(int y=0; y<params_.grid.y; y++, i++) {
	    offsets[i] = float2(x-1.0/params_.resolution*x,
				y-1.0/params_.resolution*y) ;
	    offsets[i].x += random.x;
	    offsets[i].y += random.y;
	}
    }
    
    int width = engine_->width();
    int height = engine_->height();
    
    glViewport(0, 0, params.width, params.height);
    engine_->vsmlOrtho(params_.resolution, params_.resolution);
    GLenum outputTex[8] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1,
			   GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3,
			   GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT5,
			   GL_COLOR_ATTACHMENT6, GL_COLOR_ATTACHMENT7}; 
    // manually attach 3d layers to the framebuffers
    for(int i=0, k=0;i<noBuffers;i++) {
	framebuffers_[i] = new GLFramebufferObject(params);
	framebuffers_[i]->bind();
	glBindTexture(GL_TEXTURE_3D, heightmap_);
	for(int j=0; j<maxAttachments; j++, k++) {
	    if(k >= instances_) break;
	     glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0+j, heightmap_, 0, k);
	}
	glBindTexture(GL_TEXTURE_3D, 0);
	framebuffers_[i]->release();
    }
   
    for(int i=0; i<noBuffers; i++) { //todo: need to set MRT fragment outs
	framebuffers_[i]->bind();
	perlinShader_->bind(vsml);    
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_1D, textures[0]);
	perlinShader_->setUniformValue("permutation", 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_1D, textures[1]);
	perlinShader_->setUniformValue("gradient", 1);
	glActiveTexture(GL_TEXTURE0);
	perlinShader_->setUniformValue("noiseScale", params_.noiseScale);
	perlinShader_->setUniformValue("octaves", params_.octaves);
	perlinShader_->setUniformValue("lacunarity", params_.lacunarity);
	perlinShader_->setUniformValue("gain", params_.gain);
	perlinShader_->setUniformValue("offset", params_.offset);
	perlinShader_->setUniformValue("scale", params_.scale);
	perlinShader_->setUniformValue("offsets", &offsets[8*i], 8);
	perlinShader_->setFragDataLocation("out_Color0", 0);
	perlinShader_->setFragDataLocation("out_Color1", 1);
	perlinShader_->setFragDataLocation("out_Color2", 2);
	perlinShader_->setFragDataLocation("out_Color3", 3);
	perlinShader_->setFragDataLocation("out_Color4", 4);
	perlinShader_->setFragDataLocation("out_Color5", 5);
	perlinShader_->setFragDataLocation("out_Color6", 6);
	perlinShader_->setFragDataLocation("out_Color7", 7);
	glDrawBuffers(8, outputTex); 
	quad->draw(perlinShader_);
	glDrawBuffers(1, outputTex); 
	perlinShader_->release();
	framebuffers_[i]->release();
    }
    
    
    
    //create normal map
    glGenTextures(1, &normalmap_);
    glBindTexture(GL_TEXTURE_3D, normalmap_);
    glTexParameterf(params.type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(params.type, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB16F, params_.resolution, params_.resolution, 
		 instances_, 0, GL_LUMINANCE, GL_FLOAT, 0);
    glBindTexture(GL_TEXTURE_3D, 0);
    // manually attach 3d layers to the framebuffers
    for(int i=0, k=0;i<noBuffers;i++) {
	framebuffers_[i]->bind();
	glBindTexture(GL_TEXTURE_3D, normalmap_);
	for(int j=0; j<maxAttachments; j++, k++) {
	    if(k >= instances_) break;
	     glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0+j, normalmap_, 0, k);
	}
	glBindTexture(GL_TEXTURE_3D, 0);
	framebuffers_[i]->release();
    }
    
    lightingShader_ = new GLShaderProgram();
    lightingShader_->loadShaderFromSource(GL_VERTEX_SHADER, "shaders/normals.glsl");
    lightingShader_->loadShaderFromSource(GL_FRAGMENT_SHADER, "shaders/normals.glsl");
    lightingShader_->link();
    
    float *layers = new float[instances_ + (instances_ % 8)];
    for(int x=0, i=0; x<params_.grid.x; x++) {
	for(int y=0; y<params_.grid.y; y++, i++) {
	    layers[i] = (i+0.5) / (float)instances_;
	}
    }

    for(int i=0; i<noBuffers; i++) { 
	framebuffers_[i]->bind();
	
	lightingShader_->bind(vsml);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, heightmap_);
	lightingShader_->setUniformValue("tex", 0);
	lightingShader_->setUniformValue("layers", &layers[8*i], 8);
	lightingShader_->setFragDataLocation("out_Color0", 0);
	lightingShader_->setFragDataLocation("out_Color1", 1);
	lightingShader_->setFragDataLocation("out_Color2", 2);
	lightingShader_->setFragDataLocation("out_Color3", 3);
	lightingShader_->setFragDataLocation("out_Color4", 4);
	lightingShader_->setFragDataLocation("out_Color5", 5);
	lightingShader_->setFragDataLocation("out_Color6", 6);
	lightingShader_->setFragDataLocation("out_Color7", 7);
	lightingShader_->setUniformValue("gird", params_.grid);
	glDrawBuffers(8, outputTex); 
	quad->draw(lightingShader_);
	
	lightingShader_->release();
	glBindTexture(GL_TEXTURE_3D, 0);
	framebuffers_[i]->release();
	delete framebuffers_[i];
    }
    
    
    
//    glDrawBuffers(1, outputTex); 
   
    
    
    glViewport(0, 0, width, height); // restore the viewport
    glBindTexture(GL_TEXTURE_1D, 0);
    glDeleteTextures(2, &textures[0]);
    delete quad;
    delete[] offsets;
    delete lightingShader_;
    //delete[] layers;

    
    GLTextureLoader::instance()->loadImage("textures/hello.bmp", "grass",
					   GL_RGB, IL_RGB, IL_FLOAT);
    GLTextureLoader::instance()->loadImage("textures/sand.bmp", "sand",
					   GL_RGB, IL_RGB, IL_FLOAT);
}

void GLPerlinTerrain::drawReflection(VSML *vsml, float time) {
    (*GLFramebufferManager::instance()->framebuffers())["1"]->bind();
    GLenum outputTex[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1}; 
    glDrawBuffers(1, outputTex); 
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    reflectShader_->bind(vsml);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, heightmap_);
    reflectShader_->setUniformValue("tex", 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_3D, normalmap_);
    reflectShader_->setUniformValue("normalTex", 1);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, (*GLTextureLoader::instance()->textures())["grass"].glTexId);
    reflectShader_->setUniformValue("testTex", 3);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, (*GLTextureLoader::instance()->textures())["sand"].glTexId);
    reflectShader_->setUniformValue("sandTex", 4);
    reflectShader_->setUniformValue("cameraPos", engine_->camera()->eye);
    reflectShader_->setUniformValue("LOD", lod_-5);
    reflectShader_->setUniformValue("lightPos", engine_->light());
    reflectShader_->setUniformValue("grid", float2(params_.grid.x, params_.grid.y));
    reflectShader_->setUniformValue("D", terrain_->scale().x);

    terrain_->draw(reflectShader_, instances_);

}


void GLPerlinTerrain::draw(VSML *vsml, float time) {    
   
   fftwater_->waitForHeightfieldComputeThread();
     //fftwater_->computeHeightfield(time);
    GLuint tex = fftwater_->heightfieldTexture();
   fftwater_->startHeightfieldComputeThread(time);
    
    
    drawShader_->bind(vsml);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, heightmap_);
    drawShader_->setUniformValue("tex", 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, tex);
    drawShader_->setUniformValue("waterTex", 1);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_3D, normalmap_);
    drawShader_->setUniformValue("normalTex", 2);
    
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, (*GLTextureLoader::instance()->textures())["grass"].glTexId);
    drawShader_->setUniformValue("testTex", 3);
    
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, (*GLTextureLoader::instance()->textures())["sand"].glTexId);
    drawShader_->setUniformValue("sandTex", 4);
    
    glActiveTexture(GL_TEXTURE5);
    (*GLFramebufferManager::instance()->framebuffers())["1"]->bindsurface(0);
    drawShader_->setUniformValue("reflTex", 5);
    
    drawShader_->setUniformValue("cameraPos", engine_->camera()->eye);
    drawShader_->setUniformValue("LOD", lod_);
    drawShader_->setUniformValue("lightPos", engine_->light());
    drawShader_->setUniformValue("grid", float2(params_.grid.x, params_.grid.y));
    drawShader_->setUniformValue("D", terrain_->scale().x);

    terrain_->draw(drawShader_, instances_);

}
