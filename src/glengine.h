#ifndef GLENGINE_H
#define GLENGINE_H

#include "glcommon.h"

#include <unordered_map>
#include <cstring>
using namespace std;
using namespace __gnu_cxx;
class KeyboardController;
class GLPrimitive;
class GLShaderProgram;
class VSML;
class GLFFTWater;
class GLPerlinTerrain;
struct WindowProperties {
    int width, height;
};

struct eqstr{
  bool operator()(const char* s1, const char* s2) const {
    return strcmp(s1,s2)==0;
  }
};


enum RenderMode {
    WIREFRAME, FILL
};

class GLEngine
{
public:
    GLEngine(WindowProperties &properties);
    ~GLEngine();




    void resize(int w, int h);
    void draw(float time, float dt, const KeyboardController *keycontroller); //time in s, dt in fraction of sec
    void mouseMove(float dx, float dy, float dt);

    int width() { return width_; }
    int height() { return height_; }
    void vsmlPersepective();
    void vsmlOrtho();
    void vsmlOrtho(int width, int height);
    VSML *vsml() { return vsml_; }
    Camera *camera() { return &camera_; }
    float3 light() { return lightPos_; }

    void setRenderMode(RenderMode mode) { renderMode_ = mode; }
    RenderMode renderMode() { return renderMode_; }

protected:

    void processKeyEvents(const KeyboardController *keycontroller, float dt);

    int width_, height_;
    Camera camera_;
    float3 lightPos_;
    VSML *vsml_;
    unordered_map<const char*, GLShaderProgram *, hash<const char*>, eqstr> shaderPrograms_;
    unordered_map<const char*, GLPrimitive *> primitives_;

    RenderMode renderMode_;
    int maxMipLevel_;
    GLPerlinTerrain *terrain_;
};


#endif // GLENGINE_H
