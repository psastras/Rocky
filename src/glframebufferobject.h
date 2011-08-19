#ifndef GLFRAMEBUFFEROBJECT_H
#define GLFRAMEBUFFEROBJECT_H

#include "glcommon.h"

struct GLFramebufferObjectParams {
    int width, height, nColorAttachments, nSamples;
    bool hasDepth;
    GLenum format, depthFormat, type;
};

class GLFramebufferObject
{
public:
    GLFramebufferObject(GLFramebufferObjectParams params);
    ~GLFramebufferObject();

    const GLuint *color_attachments() { return color_; }
    GLuint id() { return id_; }
    GLFramebufferObjectParams &params() { return params_;  }
    void bind();
    int width() { return params_.width; }
    int height() { return params_.height; }
    bool multisample() { return params_.nSamples > 0; }
    void bindsurface(int idx);
    void unbindsurface() { glBindTexture(params_.type, 0); }
    void release();
 
    void resize(int width, int height);
    void blit(GLFramebufferObject &dst); /// blit contents into the destination framebuffer
    GLuint* texture();
    GLuint depth();
    
    void checkStatus();

    static int queryMaxSamples();
    static int queryMaxAttachments();
    
protected:

    
    void allocFramebuffer(GLFramebufferObjectParams &params);

    GLuint depth_, *color_, id_;
    GLFramebufferObjectParams params_;
};

#endif // GLFRAMEBUFFEROBJECT_H
