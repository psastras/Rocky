#include "glengine.h"
#include "common.h"
#include "glcommon.h"
#include "glframebufferobject.h"
#include "glprimitive.h"
#include "glshaderprogram.h"
#include "keyboardcontroller.h"
#include "glperlinterrain.h"
#include <vsml.h>

GLFramebufferObject *pMultisampleFramebuffer, *pFramebuffer;
GLPrimitive *pQuad;
GLEngine::GLEngine(WindowProperties &properties) {

    renderMode_ = FILL;

    //init gl setup
    vsml_ = VSML::getInstance();
    width_ = properties.width;
    height_ = properties.height;


    glClearColor(0.0, 0.0, 0.0, 1.0);
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
    camera_.fovy = 50.0;

    GLFramebufferObjectParams params;
    params.width = properties.width;
    params.height = properties.height;
    params.hasDepth = true;
    params.depthFormat = GL_DEPTH_COMPONENT;
    params.format = GL_RGBA16F;
    params.nColorAttachments = 1;
    params.nSamples = GLFramebufferObject::queryMaxSamples() / 2;
    params.type = GL_TEXTURE_2D;
    pMultisampleFramebuffer = new GLFramebufferObject(params);

    params.hasDepth = false;
    params.nSamples = 0;

    pFramebuffer = new GLFramebufferObject(params);

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
    paramsT.grid = float2(30,30);
    paramsT.lacunarity = 1.7;
    paramsT.offset = 1;
    paramsT.noiseScale = .1;
    paramsT.tess = 512;
    paramsT.octaves = 11;
    terrain_ = new GLPerlinTerrain(paramsT, this);
    lightPos_ = float3(1.0, 0.5, 0.0).getNormalized();
}


GLEngine::~GLEngine() {
//    for (auto itr = shaderPrograms_.cbegin(); itr != shaderPrograms_.cend(); ++itr)
//	    delete &itr;
    //delete shaderPrograms_["default"];
}

void GLEngine::resize(int w, int h) {
    width_ = w; height_ = h;
    glViewport(0, 0, width_, height_);
    primitives_["quad1"]->tesselate(float3(1, 1, 0),
			float3(width_ * 0.5, height_ * 0.5, 0),
			float3(width_, height_, 1));
   pMultisampleFramebuffer->resize(width_, height_);
   pFramebuffer->resize(width_, height_);
}

void GLEngine::draw(float time, float dt, const KeyboardController *keyController) {
    processKeyEvents(keyController, dt);
    GLenum outputTex[8] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1,
			   GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3,
			   GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT5,
			   GL_COLOR_ATTACHMENT6, GL_COLOR_ATTACHMENT7}; 
    glEnable(GL_DEPTH_TEST);
    
    
    GLERROR("check-2");
    this->vsmlPersepective();
GLERROR("check-1");
    pMultisampleFramebuffer->bind();
     GLERROR("check0");
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    shaderPrograms_["icosohedron"]->bind(vsml_);
  //  glDrawBuffers(2, outputTex); 
    shaderPrograms_["icosohedron"]->setFragDataLocation("out_Color0", 0);
    shaderPrograms_["icosohedron"]->setFragDataLocation("out_Color1", 1);
    shaderPrograms_["icosohedron"]->setUniformValue("lightPos", lightPos_);
    primitives_["sphere0"]->draw(shaderPrograms_["icosohedron"]);
    shaderPrograms_["icosohedron"]->release();
    terrain_->draw(vsml_, time);
    pMultisampleFramebuffer->release();
    pMultisampleFramebuffer->blit(*pFramebuffer);
    
    GLERROR("check");
    
    glDisable(GL_DEPTH_TEST);
    this->vsmlOrtho();
    shaderPrograms_["default"]->bind(vsml_);
    glActiveTexture(GL_TEXTURE0);
    pFramebuffer->bindsurface(0);
    shaderPrograms_["default"]->setUniformValue("tex", 0);
    primitives_["quad1"]->draw(shaderPrograms_["default"]);
    pFramebuffer->unbindsurface();
    shaderPrograms_["default"]->release();
    camera_.orthogonal_camera(width_, height_);
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

float sensitivity = 0.001f;
void GLEngine::mouseMove(float dx, float dy, float dt) {
    if(dt == 0.f) return;
    float deltax = -dx*sensitivity/dt;
    float deltay = -dy*sensitivity/dt;
    camera_.roty -= deltax;
    camera_.rotx += deltay;
}

void GLEngine::processKeyEvents(const KeyboardController *keycontroller, float dt) {

    // @todo move these key defs somewhere.
#ifdef _WIN32
#define KEY_W 87
#define KEY_A 65
#define KEY_S 83
#define KEY_D 68
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

    float delta = sensitivity * 10.0 / dt;
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
    }
}
