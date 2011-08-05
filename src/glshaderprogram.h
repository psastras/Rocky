#ifndef GLSHADERPROGRAM_H
#define GLSHADERPROGRAM_H

#include "common.h"
#include "glcommon.h"

#include <unordered_map>
#include <vector>
class GLShaderProgram
{
public:
    GLShaderProgram();
    ~GLShaderProgram();

    void loadShaderFromSource(GLenum type, std::string source);

    bool link();
    void bind() { glUseProgram(programId_); }
    void release() { glUseProgram(0); }

    inline GLint getUniformLocation(const char *name) {
	if(uniforms_.find(name) == uniforms_.end()) {
	    uniforms_[name] = glGetUniformLocation(programId_, name);
	}
	return uniforms_[name];
    }

    inline GLint getAttributeLocation(const char *name) {
	if(attributes_.find(name) == attributes_.end()) {
	    attributes_[name] = glGetAttribLocation(programId_, name);
	}
	return attributes_[name];
    }


    inline void setGeometryInputType(GLenum type) {
	glProgramParameteriEXT(programId_, GL_GEOMETRY_INPUT_TYPE_EXT, type);
    }

    inline void setGeometryOutputType(GLenum type) {
	glProgramParameteriEXT(programId_, GL_GEOMETRY_OUTPUT_TYPE_EXT, type);
    }

    inline void setUniformValue(const char *name, float2 val) {
	glUniform2fv(getUniformLocation(name), 1, &val.x);
    }

    inline void setUniformValue(const char *name, float3 val) {
	glUniform3fv(getUniformLocation(name), 1, &val.x);
    }

    inline void setUniformValue(const char *name, float val) {
	glUniform1f(getUniformLocation(name), val);
    }

    inline void setUniformValue(const char *name, int val){
	glUniform1i(getUniformLocation(name), val);
    }

    inline void setUniformValue(const char *name, double val){
	glUniform1d(getUniformLocation(name), val);
    }

    inline void setUniformValue(const char *name, unsigned int val){
	glUniform1ui(getUniformLocation(name), val);
    }

    inline void setFragDataLocation(const char *name, unsigned int val) {
	glBindFragDataLocation(programId_, val, name);
    }

protected:

    std::unordered_map<const char *, GLint> uniforms_;
    std::unordered_map<const char *, GLint> attributes_;
    std::vector<GLuint> shaders_;
    GLuint programId_;

};

#endif // GLSHADERPROGRAM_H
