#ifndef GLFRAMEBUFFEROBJECT_H
#define GLFRAMEBUFFEROBJECT_H

#include "glcommon.h"

struct GLFramebufferObjectParams {
    int width, height, nColorAttachments, nSamples;
    bool hasDepth;
    GLenum format, depthFormat;
};

class GLFramebufferObject
{
public:
    GLFramebufferObject(GLFramebufferObjectParams &params);
    ~GLFramebufferObject();

    const GLuint *color_attachments() { return color_; }
    GLuint id() { return id_; }
    GLFramebufferObjectParams &params() { return params_;  }
    void bind();
    int width() { return params_.width; }
    int height() { return params_.height; }
    bool multisample() { return params_.nSamples > 0; }
    void bindsurface(int idx) { glBindTexture(GL_TEXTURE_2D, color_[idx]);}
    void unbindsurface() { glBindTexture(GL_TEXTURE_2D, 0); }
    void release();
    void blit(GLFramebufferObject &dst); /// blit contents into the destination framebuffer
    GLuint* texture();
    GLuint depth();

protected:

    void allocFramebuffer(GLFramebufferObjectParams &params);

    GLuint depth_, *color_, id_;
    GLFramebufferObjectParams params_;
};

#endif // GLFRAMEBUFFEROBJECT_H
