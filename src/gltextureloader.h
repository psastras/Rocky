#ifndef GLTEXTURELOADER_H
#define GLTEXTURELOADER_H

#include "glcommon.h"
#include <IL/il.h>
#include <unordered_map>

#include "glframebufferobject.h"

class GLTextureLoader {
public:
    static GLTextureLoader *instance() {
	if(!s_instance) s_instance = new GLTextureLoader();
	return s_instance;
    }
    struct Texture {
	GLuint glTexId;
    };
    void loadImage(const char *filename, const char *name, GLenum internalFormat,
		   ILenum format, ILenum type);
    
    std::unordered_map<const char*, Texture> * textures() { return &textures_; }
    
protected:
    GLTextureLoader();
    static GLTextureLoader *s_instance;
  
    std::unordered_map<const char*, Texture> textures_;    
};


class GLFramebufferManager {
public:
    static GLFramebufferManager *instance() {
	if(!s_instance) s_instance = new GLFramebufferManager();
	return s_instance;
    }
    
    std::unordered_map<std::string, GLFramebufferObject *> *framebuffers() { return &framebuffers_; }
    
protected:
    GLFramebufferManager() {}
    static GLFramebufferManager *s_instance;
  
    std::unordered_map<std::string, GLFramebufferObject *> framebuffers_;    
};

#endif // GLTEXTURELOADER_H
