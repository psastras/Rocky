#include "glshaderprogram.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sstream>
#include <fstream>

using namespace std;
GLShaderProgram::GLShaderProgram() {
    programId_ =  glCreateProgram();
}

GLShaderProgram::~GLShaderProgram() {
    while(!shaders_.empty()) {
	glDeleteShader(shaders_.back());
	shaders_.pop_back();
    }
    glDeleteProgram(programId_);
}

void printLog(GLuint obj) {
    int infologLength = 0;
    char infoLog[1024];
	if (glIsShader(obj))
		glGetShaderInfoLog(obj, 1024, &infologLength, infoLog);
	else
		glGetProgramInfoLog(obj, 1024, &infologLength, infoLog);
    if (infologLength > 0)
		printf("%s\n", infoLog);
    fflush(stdout);
}

void GLShaderProgram::loadShaderFromSource(GLenum type, std::string source) {
    stringstream ss;
    if(type == GL_FRAGMENT_SHADER)
	ss << "#define _FRAGMENT_" << endl;
    else if(type == GL_VERTEX_SHADER)
	ss << "#define _VERTEX_" << endl;
    else if(type == GL_GEOMETRY_SHADER)
	ss << "#define _GEOMETRY_" << endl;
    else if(type == GL_TESS_EVALUATION_SHADER)
	ss << "#define _TESSEVAL_" << endl;
    else if(type == GL_TESS_CONTROL_SHADER)
	ss << "#define _TESSCONTROL_" << endl;
    ifstream file(source.c_str());
    string line;
    if (file.is_open()) {
       while (file.good()) {
	 getline(file, line);
	 ss << line << endl;
      }
      file.close();
    } else {
	cerr << "Failed to open file " << source << endl;
	return;
    }
    std::string str = ss.str();
    int length = str.length();
    const char *data = str.c_str();
    GLuint id = glCreateShader(type);
    glShaderSource(id, 1, (const char **)&data, &length);
    glCompileShader(id);
    printLog(id);
    glAttachShader(programId_, id);
    shaders_.push_back(id);
}


bool GLShaderProgram::link() {
  //  glBindAttribLocation(programId_, 0, "in_Position"); // Bind a constant attribute location for positions of vertices
//    glBindAttribLocation(programId_, 1, "in_Normal"); // Bind another constant attribute location, this time for color
//    glBindAttribLocation(programId_, 2, "in_TexCoord"); // Bind another constant attribute location, this time for color
    glLinkProgram(programId_);
    return true;
}
