#include "glframebufferobject.h"
#include "glcommon.h"
#include "common.h"
GLFramebufferObject::GLFramebufferObject(GLFramebufferObjectParams params) {
    color_ = 0;
    params_ = params;
    // @todo: should check the parameters to maker sure they make sense
    // instead were just going to check for a glError, if one of the parameters
    // is not correct it will probably result in a gl error
    
    this->allocFramebuffer(params_);
    GLERROR("creating framebuffer object");

}

GLFramebufferObject::~GLFramebufferObject() {
    this->bind();
    if(params_.nSamples > 1) {
	glDeleteRenderbuffersEXT(params_.nColorAttachments, &color_[0]);
	glDeleteRenderbuffersEXT(1, &depth_);
    } else {
	glDeleteTextures(params_.nColorAttachments, &color_[0]);
	glDeleteTextures(1, &depth_);
    }
  
    this->release();
    glDeleteFramebuffersEXT(1, &id_);
    
    if(color_) delete[] color_;

}

void GLFramebufferObject::bindsurface(int idx) {
    if(params_.type == GL_TEXTURE_2D) {
	glBindTexture(GL_TEXTURE_2D, color_[idx]);
    }
    else if(params_.type == GL_TEXTURE_3D) {
	glBindTexture(GL_TEXTURE_3D, color_[0]);
    }
}

// @todo: add stencil buffer support and error handling (esp. for nonsupported formats)
void GLFramebufferObject::allocFramebuffer(GLFramebufferObjectParams &params) {
    
    glGenFramebuffers(1, &id_);
    
    params.nCSamples = params.nSamples * 2; //TODO: MAKE THIS AN OPTION;
    
    bool isCSAA = false;glewIsSupported( "GL_NV_framebuffer_multisample_coverage") && (params.nCSamples > params.nSamples) && params.nSamples > 1;
    GLint coverageSampleConfigs;
    GLint *coverageConfigs = 0;
    
    if(isCSAA) {
	glGetIntegerv( GL_MAX_MULTISAMPLE_COVERAGE_MODES_NV, &coverageSampleConfigs);
        coverageConfigs = new int[coverageSampleConfigs * 2 + 4];
	glGetIntegerv( GL_MULTISAMPLE_COVERAGE_MODES_NV, coverageConfigs);
    }
    
     
    
    if(!params.nColorAttachments) return;
    this->bind();
    
    if(params.type == GL_TEXTURE_2D)
	color_ = new GLuint[params.nColorAttachments];
    else if(params.type == GL_TEXTURE_3D)
	color_ = new GLuint[1];
    
    if(params.nSamples > 0) { //create multisample targets
	GLint maxSamples = 0;
        glGetIntegerv(GL_MAX_SAMPLES_EXT, &maxSamples);
	if(params.nSamples > maxSamples) {
	    cerr << "Warning: maximum number of samples supported is " << maxSamples << " but requested number of samples is " <<
		    params.nSamples << ".  Falling back to " << maxSamples << " samples." << endl;
	    params.nSamples = maxSamples;
	}
	
	if(params.type == GL_TEXTURE_2D) {
	
	    glGenRenderbuffers(params.nColorAttachments, &color_[0]);
	    for(int i=0; i<params.nColorAttachments; i++) {
		glBindRenderbuffer(GL_RENDERBUFFER, color_[i]);
		
		if(isCSAA) glRenderbufferStorageMultisampleCoverageNV(GL_RENDERBUFFER, params.nCSamples, params.nSamples, params.format, params.width, params.height);
		else  glRenderbufferStorageMultisample(GL_RENDERBUFFER, params.nSamples, params.format, params.width, params.height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_RENDERBUFFER, color_[i]);
	    }
	    glBindRenderbuffer(GL_RENDERBUFFER, 0);
	} else if(params.type == GL_TEXTURE_3D) {
	    cerr << "3D textures with multisample framebuffers currently not supported.";
	    assert(0);
	}
	
    } else { //create regular targets
	
	if(params.type == GL_TEXTURE_2D) {
	    glGenTextures(params.nColorAttachments, &color_[0]);
	    
	    for(int i=0; i<params.nColorAttachments; i++) {
		glBindTexture(params.type, color_[i]);
		glTexParameterf(params.type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(params.type, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexImage2D(GL_TEXTURE_2D, 0, params.format, params.width, params.height, 0, GL_LUMINANCE, GL_FLOAT, 0);
	        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, color_[i], 0);
	    }

	    glBindTexture(params.type, 0);
	} else if(params.type == GL_TEXTURE_3D) {
	    glGenTextures(1, &color_[0]);
	    glBindTexture(params.type, color_[0]);
	     
	    glTexParameterf(params.type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	    glTexParameterf(params.type, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	    glTexImage3D(params.type, 0, params.format, params.width, params.height, 
			 params.nColorAttachments, 0, GL_LUMINANCE, GL_FLOAT, 0);
	    for(int i=0; i<params.nColorAttachments; i++) {
		glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, color_[0], 0, i);
		//glFramebufferTexture3D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_3D, color_[0], 0, i);
	    }
	    glBindTexture(params.type, 0);
	    
	}
    }
 
    if(params.hasDepth) {
	if(params.nSamples > 0) {
	    glGenRenderbuffers(1, &depth_);
	    glBindRenderbuffer(GL_RENDERBUFFER, depth_);
	    if(isCSAA) glRenderbufferStorageMultisampleCoverageNV(GL_RENDERBUFFER, params.nCSamples, params.nSamples, params.depthFormat, params.width, params.height); 
	    else  glRenderbufferStorageMultisample(GL_RENDERBUFFER, params.nSamples, params.depthFormat, params.width, params.height);
	    
	    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_);
	    glBindRenderbuffer(GL_RENDERBUFFER, 0);
	    
	    
	} else {
	    glGenTextures(1, &depth_);
	    glBindTexture(params.type, depth_);
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	    glTexParameteri(params.type, GL_DEPTH_TEXTURE_MODE, GL_LUMINANCE);
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	    glTexImage2D(params.type, 0, params.depthFormat, params.width, params.height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_, 0);
	    glBindTexture(params.type, 0);
	}

    }
    
    //if(glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT) == GL_FRAMEBUFFER_COMPLETE_EXT) { cerr << "herp derp" << endl; }
    //else cerr << "oops" << endl;
    delete[] coverageConfigs;
    this->release();
}

void GLFramebufferObject::blit(GLFramebufferObject &dst) {
    assert(this->params_.nColorAttachments == dst.params().nColorAttachments);
   // assert(this->params().hasDepth == dst.params().hasDepth);

    for(int i=0; i<params_.nColorAttachments; i++) {
	glBindFramebufferEXT(GL_READ_FRAMEBUFFER, this->id());
	glReadBuffer(GL_COLOR_ATTACHMENT0 + i);
	glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER, dst.id());
	glDrawBuffer(GL_COLOR_ATTACHMENT0 + i);
        glBlitFramebuffer(0, 0, this->width(), this->height(), 0, 0, dst.width(), dst.height(), GL_COLOR_BUFFER_BIT, GL_NEAREST);
    }
   
    if(this->params().hasDepth && dst.params().hasDepth) {
	
	glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, this->id());
	glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, dst.id());
        glBlitFramebufferEXT(0, 0, this->width(), this->height(), 0, 0, dst.width(), dst.height(), GL_DEPTH_BUFFER_BIT, GL_NEAREST);
	
    }

    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

}

GLuint* GLFramebufferObject::texture() {
    return color_;
}

GLuint GLFramebufferObject::depth() {
    return depth_;
}

void GLFramebufferObject::bind() {
    
     glBindFramebuffer(GL_FRAMEBUFFER, id_);
}

void GLFramebufferObject::checkStatus() {
    GLuint status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    switch(status) {
	case GL_FRAMEBUFFER_COMPLETE:
	    return;
	    break;
    
    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
	std::cerr << "An attachment could not be bound to frame buffer object!" << std::endl;
	break;
    
    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
	std::cerr <<  "Attachments are missing! At least one image (texture) must be bound to the frame buffer object!" << std::endl;
	break;
    
    case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
	std::cerr <<  "The dimensions of the buffers attached to the currently used frame buffer object do not match!" << std::endl;
	break;
    
    case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
	std::cerr <<  "The formats of the currently used frame buffer object are not supported or do not fit together!" << std::endl;
	break;
    
    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
	std::cerr <<  "A Draw buffer is incomplete or undefinied. All draw buffers must specify attachment points that have images attached." << std::endl;
	break;
    
    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
	std::cerr <<  "A Read buffer is incomplete or undefinied. All read buffers must specify attachment points that have images attached." << std::endl;
	break;
    
    case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
	std::cerr <<  "All images must have the same number of multisample samples." << std::endl;
	break;
    
    case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS :
	std::cerr <<  "If a layered image is attached to one attachment, then all attachments must be layered attachments. The attached layers do not have to have the same number of layers, nor do the layers have to come from the same kind of texture." << std::endl;
	break;
    
    case GL_FRAMEBUFFER_UNSUPPORTED:
	std::cerr <<  "Attempt to use an unsupported format combinaton!" << std::endl;
	break;
    
    default:
	std::cerr <<  "Unknown error while attempting to create frame buffer object!" << std::endl;
	break;
    }
    
    assert(0);
}

void GLFramebufferObject::release() {
     glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

int GLFramebufferObject::queryMaxSamples() {
    GLint maxSamples = 0;
    glGetIntegerv(GL_MAX_SAMPLES, &maxSamples);
    return maxSamples;
}

int GLFramebufferObject::queryMaxAttachments() {
    GLint maxAttachments = 0;
    glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &maxAttachments);
    return maxAttachments;
}

void GLFramebufferObject::resize(int width, int height) {
    if(params_.width == width &&  params_.height == height) return;
    params_.width = width;
    params_.height = height;
    
    this->bind();
    if(params_.nSamples > 0) { //create multisample targets
	GLint maxSamples = 0;
        glGetIntegerv(GL_MAX_SAMPLES_EXT, &maxSamples);
	if(params_.nSamples > maxSamples) {
	    cerr << "Warning: maximum number of samples supported is " << maxSamples << " but requested number of samples is " <<
		    params_.nSamples << ".  Falling back to " << maxSamples << " samples." << endl;
	    params_.nSamples = maxSamples;
	}
	for(int i=0; i<params_.nColorAttachments; i++) {
            glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, color_[i]);
	    glRenderbufferStorageMultisampleEXT(GL_RENDERBUFFER_EXT, params_.nSamples, params_.format, params_.width, params_.height);
	    glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT + i, GL_RENDERBUFFER_EXT, color_[i]);
	}

    } else { //create regular targets
	for(int i=0; i<params_.nColorAttachments; i++) {
	    glBindTexture(GL_TEXTURE_2D, color_[i]);
	    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	    glTexImage2D(GL_TEXTURE_2D, 0, params_.format, params_.width, params_.height, 0, GL_LUMINANCE, GL_FLOAT, 0);
	    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT + i, GL_TEXTURE_2D, color_[i], 0);
	}

	glBindTexture(GL_TEXTURE_2D, 0);
    }


    if(params_.hasDepth) {
	if(params_.nSamples > 0) {
	    glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, depth_);
	    glRenderbufferStorageMultisampleEXT(GL_RENDERBUFFER_EXT, params_.nSamples, params_.depthFormat, params_.width, params_.height);
	    glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, depth_);
	    glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);
	} else {
	    glBindTexture(GL_TEXTURE_2D, depth_);
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	    glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY);
	    glTexImage2D(GL_TEXTURE_2D, 0, params_.depthFormat, params_.width, params_.height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, depth_, 0);
	    glBindTexture(GL_TEXTURE_2D, 0);

	}

    }
    this->release();
}
