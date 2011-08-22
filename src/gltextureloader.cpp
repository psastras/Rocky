#include "gltextureloader.h"

GLTextureLoader::GLTextureLoader() {
    ilInit();
}


void GLTextureLoader::loadImage(const char *filename, const char *name, GLenum internalFormat,
				ILenum format, ILenum type) {
    ILuint ilId;
    GLuint glId;
    ilGenImages(1, &ilId);
    glGenTextures(1, &glId);
    ilBindImage(ilId);
    
    ilLoadImage(filename);
    ILenum err = ilGetError();
    if(err != IL_NO_ERROR) {
	std::cerr << err << std::endl;    assert(0);
    }
    ilConvertImage(format, type);
    ILubyte *data = ilGetData();
    glBindTexture(GL_TEXTURE_2D, glId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, ilGetInteger(IL_IMAGE_WIDTH),
		 ilGetInteger(IL_IMAGE_HEIGHT), 0, format, type, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
    ilDeleteImage(ilId);
    
    textures_[name].glTexId = glId;
}

