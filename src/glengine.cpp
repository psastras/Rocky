#include "glengine.h"
#include "common.h"
#include "glcommon.h"
#include "glframebufferobject.h"
#include "glprimitive.h"
#include "glshaderprogram.h"
#include "keyboardcontroller.h"
#include "glperlinterrain.h"
#include "gltextureloader.h"
#include <vsml.h>
#include <IL/il.h>
#include <tr1/random>

GLPrimitive *pQuad;
GLEngine::GLEngine(WindowProperties &properties) {
    //std::thread t(hello);
    //renderMode_ = FILL;

    //init gl setup
    vsml_ = VSML::getInstance();
    width_ = properties.width;
    height_ = properties.height;


    glClearColor(0.0, 0.0, 0.0, 0.0);
    glViewport(0,0,width_,height_);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CW);
    glDisable(GL_DITHER);

    camera_.center = float3(0.0, 10.0, 0.0);
    camera_.eye = float3(0.0, 30.0, 50.0);
    camera_.up = float3(0.0, 1.0, 0.0);
    camera_.near = 1.0;
    camera_.far = 7000.0;
    camera_.rotx = camera_.roty = 0.f;
    camera_.fovy = 60.0;

    GLFramebufferObjectParams params;
    params.width = properties.width;
    params.height = properties.height;
    params.hasDepth = true;
    params.depthFormat = GL_DEPTH_COMPONENT16;
    params.format = GL_RGBA16F;
    params.nColorAttachments = 2;
    params.nSamples = 4;//GLFramebufferObject::queryMaxSamples();
    params.nCSamples = 4;
    params.type = GL_TEXTURE_2D;
    (*GLFramebufferManager::instance()->framebuffers())["m"] = new GLFramebufferObject(params);
    
    params.hasDepth = true;
    params.nSamples = 0;

    (*GLFramebufferManager::instance()->framebuffers())["0"] = new GLFramebufferObject(params);
    params.hasDepth = false;
    params.nColorAttachments = 1;
    (*GLFramebufferManager::instance()->framebuffers())["1"] = new GLFramebufferObject(params);
    
    primitives_["quad1"] = new GLQuad(float3(1, 1, 0),
			float3(width_ * 0.5, height_ * 0.5, 0),
			float3(width_, height_, 1));

    primitives_["plane0"] = new GLPlane(float3(40, 0, 40),
			 float3(0, 0, 0),
			 float3(20, 1, 20));
    

    primitives_["sphere0"]  = new GLIcosohedron(float3::zero(), float3::zero(), float3(5000, 5000, 5000));


    //load shader programs
    shaderPrograms_["default"] = new GLShaderProgram();
    shaderPrograms_["default"]->loadShaderFromSource(GL_VERTEX_SHADER, "shaders/default.glsl");
    shaderPrograms_["default"]->loadShaderFromSource(GL_FRAGMENT_SHADER, "shaders/default.glsl");
    shaderPrograms_["default"]->link();

    shaderPrograms_["post0"] = new GLShaderProgram();
    shaderPrograms_["post0"]->loadShaderFromSource(GL_VERTEX_SHADER, "shaders/post0.glsl");
    shaderPrograms_["post0"]->loadShaderFromSource(GL_FRAGMENT_SHADER, "shaders/post0.glsl");
    shaderPrograms_["post0"]->link();
    
    shaderPrograms_["post1"] = new GLShaderProgram();
    shaderPrograms_["post1"]->loadShaderFromSource(GL_VERTEX_SHADER, "shaders/post1.glsl");
    shaderPrograms_["post1"]->loadShaderFromSource(GL_FRAGMENT_SHADER, "shaders/post1.glsl");
    shaderPrograms_["post1"]->link();
    
    shaderPrograms_["post2"] = new GLShaderProgram();
    shaderPrograms_["post2"]->loadShaderFromSource(GL_VERTEX_SHADER, "shaders/post2.glsl");
    shaderPrograms_["post2"]->loadShaderFromSource(GL_FRAGMENT_SHADER, "shaders/post2.glsl");
    shaderPrograms_["post2"]->link();
    
    shaderPrograms_["post3"] = new GLShaderProgram();
    shaderPrograms_["post3"]->loadShaderFromSource(GL_VERTEX_SHADER, "shaders/post3.glsl");
    shaderPrograms_["post3"]->loadShaderFromSource(GL_FRAGMENT_SHADER, "shaders/post3.glsl");
    shaderPrograms_["post3"]->link();
    
    shaderPrograms_["solid"] = new GLShaderProgram();
    shaderPrograms_["solid"]->loadShaderFromSource(GL_VERTEX_SHADER, "shaders/solid.glsl");
    shaderPrograms_["solid"]->loadShaderFromSource(GL_FRAGMENT_SHADER, "shaders/solid.glsl");
    shaderPrograms_["solid"]->link();


    shaderPrograms_["icosohedron"] = new GLShaderProgram();
    shaderPrograms_["icosohedron"]->loadShaderFromSource(GL_VERTEX_SHADER, "shaders/icosohedron.glsl");
    shaderPrograms_["icosohedron"]->loadShaderFromSource(GL_FRAGMENT_SHADER, "shaders/icosohedron.glsl");
    shaderPrograms_["icosohedron"]->loadShaderFromSource(GL_TESS_CONTROL_SHADER, "shaders/icosohedron.glsl");
    shaderPrograms_["icosohedron"]->loadShaderFromSource(GL_TESS_EVALUATION_SHADER, "shaders/icosohedron.glsl");
    shaderPrograms_["icosohedron"]->link();
             	
    GLPerlinTerrainParams paramsT;
    paramsT.resolution = 128;
    paramsT.gain = 0.61;
    paramsT.grid = float2(24,24);
    paramsT.lacunarity = 1.7;
    paramsT.offset = 1;
    paramsT.noiseScale = .15;
    paramsT.tess = 512;
    paramsT.octaves = 12;
    paramsT.random = float2(0.f, 0.f);
   
    terrain_ = new GLPerlinTerrain(paramsT, this);
    
    lightPos_ = float3(1.0, 0.1, 0.0).getNormalized();
    
    shaderPrograms_["icosohedron"]->setFragDataLocation("out_Color0", 0);
    shaderPrograms_["icosohedron"]->setFragDataLocation("out_Color1", 1);
    
}


GLEngine::~GLEngine() {
    for (auto itr = shaderPrograms_.begin(); itr != shaderPrograms_.end(); ++itr)
	 delete (*itr).second;
    for (auto itr = primitives_.begin(); itr != primitives_.end(); ++itr)
	 delete (*itr).second;

    delete terrain_;
}

void GLEngine::resize(int w, int h) {
    width_ = w; height_ = h;
    glViewport(0, 0, width_, height_);
    primitives_["quad1"]->tesselate(float3(1, 1, 0),
			float3(width_ * 0.5, height_ * 0.5, 0),
			float3(width_, height_, 1));
 
    (*GLFramebufferManager::instance()->framebuffers())["m"]->resize(width_, height_);
    (*GLFramebufferManager::instance()->framebuffers())["0"]->resize(width_, height_);
    (*GLFramebufferManager::instance()->framebuffers())["1"]->resize(width_, height_);
    camera_.orthogonal_camera(width_, height_);
    
    maxMipLevel_ = (log10((float)max(width_, height_)) / log10(2.f));
   
}
float currentViewMatrices[2][16];
float previousViewMatrices[2][16];
void GLEngine::draw(float time, float dt, const KeyboardController *keyController) {
GLERROR("blah");
    processKeyEvents(keyController, dt);

    memcpy(&previousViewMatrices[0][0], &currentViewMatrices[0][0], sizeof(float)*32);
    this->vsmlPersepective();
    memcpy(&currentViewMatrices[0][0], vsml_->get(VSML::MODELVIEW), sizeof(float)*32);
    
    // draw
    glEnable(GL_DEPTH_TEST);
    GLenum outputTex[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1}; 
    terrain_->drawReflection(vsml_, time);
  
    (*GLFramebufferManager::instance()->framebuffers())["m"]->bind();
    glClear(GL_DEPTH_BUFFER_BIT);
    shaderPrograms_["icosohedron"]->bind(vsml_);
    shaderPrograms_["icosohedron"]->setUniformValue("lightPos", lightPos_);
   
    glDrawBuffers(2, outputTex); 
   
    primitives_["sphere0"]->draw(shaderPrograms_["icosohedron"]);
    
    terrain_->draw(vsml_, time);
    (*GLFramebufferManager::instance()->framebuffers())["m"]->blit(*(*GLFramebufferManager::instance()->framebuffers())["0"]);
    // start post process
    
    glDisable(GL_DEPTH_TEST);
    this->vsmlOrtho();
    
    // god rays + log luminance
    
    (*GLFramebufferManager::instance()->framebuffers())["1"]->bind();
    shaderPrograms_["post0"]->bind(vsml_);
    glActiveTexture(GL_TEXTURE0);
    (*GLFramebufferManager::instance()->framebuffers())["0"]->bindsurface(0);
    shaderPrograms_["post0"]->setUniformValue("tex", 0);
    glActiveTexture(GL_TEXTURE1);
    (*GLFramebufferManager::instance()->framebuffers())["0"]->bindsurface(1);
    shaderPrograms_["post0"]->setUniformValue("posTex", 1);
    shaderPrograms_["post0"]->setUniformValue("modelviewMatrixCurr", currentViewMatrices[0]);
    shaderPrograms_["post0"]->setUniformValue("projMatrixCurr", currentViewMatrices[1]);
    shaderPrograms_["post0"]->setUniformValue("lightPos", lightPos_);
    primitives_["quad1"]->draw(shaderPrograms_["post0"]);
    // tone map
    
    (*GLFramebufferManager::instance()->framebuffers())["0"]->bind();
     glDrawBuffers(1, outputTex); 
    shaderPrograms_["post1"]->bind(vsml_);
    glActiveTexture(GL_TEXTURE0);
    (*GLFramebufferManager::instance()->framebuffers())["1"]->bindsurface(0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); 
    glGenerateMipmap(GL_TEXTURE_2D);
    shaderPrograms_["post1"]->setUniformValue("tex", 0);
    shaderPrograms_["post1"]->setUniformValue("maxMipLevel", (float)maxMipLevel_);
    primitives_["quad1"]->draw(shaderPrograms_["post1"]);
    
    // dof
    
    (*GLFramebufferManager::instance()->framebuffers())["1"]->bind();
    shaderPrograms_["post2"]->bind(vsml_);
    glActiveTexture(GL_TEXTURE0);
    (*GLFramebufferManager::instance()->framebuffers())["0"]->bindsurface(0);
    shaderPrograms_["post2"]->setUniformValue("tex", 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, (*GLFramebufferManager::instance()->framebuffers())["0"]->depth());
    shaderPrograms_["post2"]->setUniformValue("depthTex", 1);
    primitives_["quad1"]->draw(shaderPrograms_["post2"]);
  
    // moblur
    (*GLFramebufferManager::instance()->framebuffers())["0"]->bind();
    shaderPrograms_["post3"]->bind(vsml_);
    if(renderMode() == WIREFRAME) shaderPrograms_["post3"]->setUniformValue("wireframe", true);
    else shaderPrograms_["post3"]->setUniformValue("wireframe", false);
    glActiveTexture(GL_TEXTURE0);
    (*GLFramebufferManager::instance()->framebuffers())["1"]->bindsurface(0);
    shaderPrograms_["post3"]->setUniformValue("tex", 0);
    glActiveTexture(GL_TEXTURE1);
    (*GLFramebufferManager::instance()->framebuffers())["0"]->bindsurface(1);
    shaderPrograms_["post3"]->setUniformValue("posTex", 1);
    shaderPrograms_["post3"]->setUniformValue("modelviewMatrixPrev", previousViewMatrices[0]);
    shaderPrograms_["post3"]->setUniformValue("projMatrixPrev", previousViewMatrices[1]);
    shaderPrograms_["post3"]->setUniformValue("modelviewMatrixCurr", currentViewMatrices[0]);
    shaderPrograms_["post3"]->setUniformValue("projMatrixCurr", currentViewMatrices[1]);
    primitives_["quad1"]->draw(shaderPrograms_["post3"]);
    (*GLFramebufferManager::instance()->framebuffers())["0"]->release();
    
    //glReadPixels(0, 0, this->width(), this->height(), GL_RGBA);
    
    shaderPrograms_["default"]->bind(vsml_);
    glActiveTexture(GL_TEXTURE0);
    (*GLFramebufferManager::instance()->framebuffers())["0"]->bindsurface(0);
    shaderPrograms_["default"]->setUniformValue("tex", 0);
    primitives_["quad1"]->draw(shaderPrograms_["default"]);;
}

void GLEngine::vsmlOrtho() {
    vsml_->loadIdentity(VSML::PROJECTION);
    vsml_->ortho(0.f,(float)width_,(float)height_,0.f);
    vsml_->loadIdentity(VSML::MODELVIEW);
}

void GLEngine::vsmlOrtho(int width, int height) {
    vsml_->loadIdentity(VSML::PROJECTION);
    vsml_->ortho(0.f,(float)width,(float)height,0.f);
    vsml_->loadIdentity(VSML::MODELVIEW);
}

void GLEngine::vsmlPersepective() {
    int w = width_, h = height_;
    vsml_->perspective(camera_.fovy, w / (float)h, camera_.near, camera_.far);
    vsml_->loadIdentity(VSML::MODELVIEW);
     
    vsml_->rotate(camera_.rotx, 1.f, 0.f, 0.f);
    vsml_->rotate(camera_.roty, 0.f, 1.f, 0.f);
    vsml_->translate(-camera_.eye.x, -camera_.eye.y, -camera_.eye.z);
}

float sensitivity = 0.01f;
void GLEngine::mouseMove(float dx, float dy, float dt) {
    if(dt == 0.f) return;
    float deltax = -dx*sensitivity/dt;
    float deltay = -dy*sensitivity/dt;
    camera_.roty -= deltax;
    camera_.rotx += deltay;
}
std::tr1::mt19937 prng(1337);
std::tr1::uniform_real<float> uniform;
std::tr1::variate_generator<std::tr1::mt19937, std::tr1::uniform_real<float> > randu(prng,uniform);
void GLEngine::processKeyEvents(const KeyboardController *keycontroller, float dt) {

    // todo: this is so bad.,,
#ifdef _WIN32
#define KEY_W 87
#define KEY_A 65
#define KEY_S 83
#define KEY_D 68
#define KEY_G 71
#define KEY_SPACE 32
#define KEY_1 49
#define KEY_2 50
#define KEY_UP 38
#define KEY_DOWN 40
#else
#define KEY_W 25
#define KEY_A 38
#define KEY_S 39
#define KEY_D 40
#define KEY_SPACE 65
#endif

    float delta = sensitivity * 20.0 / dt;
    if(keycontroller->isKeyDown(KEY_W)) { //W
	float yrotrad = camera_.roty / 180 * 3.141592654f;
	float xrotrad = camera_.rotx / 180 * 3.141592654f;
	camera_.eye.x += sinf(yrotrad)*delta;
	camera_.eye.z -= cosf(yrotrad)*delta;
	camera_.eye.y -= sinf(xrotrad)*delta;
    } if(keycontroller->isKeyDown(KEY_A)) { //A
	float yrotrad = (camera_.roty / 180 * 3.141592654f);
	camera_.eye.x -= cosf(yrotrad)*delta;
	camera_.eye.z -= sinf(yrotrad)*delta;
    } if(keycontroller->isKeyDown(KEY_S)) { //S
	float yrotrad = camera_.roty / 180 * 3.141592654f;
	float xrotrad = camera_.rotx / 180 * 3.141592654f;
	camera_.eye.x -= sinf(yrotrad)*delta;
	camera_.eye.z += cosf(yrotrad)*delta;
	camera_.eye.y += sinf(xrotrad)*delta;
    } if(keycontroller->isKeyDown(KEY_D)) { //D
	float yrotrad = (camera_.roty / 180 * 3.141592654f);
	camera_.eye.x += cosf(yrotrad)*delta;
	camera_.eye.z += sinf(yrotrad)*delta;
    } if(keycontroller->isKeyDown(KEY_SPACE)) { //space
	camera_.eye.y += delta;
    } if(keycontroller->isKeyPress(KEY_1)) {
	this->setRenderMode(FILL);
    } if(keycontroller->isKeyPress(KEY_2)) {
	this->setRenderMode(WIREFRAME);
    } if(keycontroller->isKeyDown(KEY_UP)) {
	terrain_->setLod(terrain_->lod() + 1);
    } if(keycontroller->isKeyDown(KEY_DOWN)) {
	terrain_->setLod(max(terrain_->lod() - 1, 1.f));
    } if(keycontroller->isKeyPress(KEY_G)) {
	GLPerlinTerrainParams paramsT = terrain_->params();
	delete terrain_;
	terrain_ = 0;
	paramsT.random = float2(randu()*100000.f,
				randu()*100000.f);
	terrain_ = new GLPerlinTerrain(paramsT, this);
    }
    
    if(camera_.eye.y <= 10.0) camera_.eye.y = 10.0;
}
