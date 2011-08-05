#ifndef GLCOMMON_H
#define GLCOMMON_H
#ifdef _WIN32
#include <windows.h>
#endif
#include <GL/glew.h>
#include <GL/wglew.h>
#include "common.h"
#ifdef near
#undef near
#endif
#ifdef far
#undef far
#endif

#define GLERROR(DESC) \
{ \
    GLenum err = glGetError(); \
    if(err != GL_NO_ERROR) { \
	cerr << "Error in " << DESC << ":" << endl; \
	cerr << "\t>> " << gluErrorString(err) << endl; \
	assert(err == GL_NO_ERROR); \
    } \
}

struct Camera {
    float3 eye, center, up;
    float fovy, near, far;
    float rotx, roty;

    void perspective_camera(int w,int h) {

	float ratio = w / static_cast<float>(h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fovy,ratio,near,far);
	/*gluLookAt(eye.x,eye.y,eye.z,
		  center.x,center.y,center.z,
		  up.x,up.y,up.z);*/
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glRotatef(rotx,1.0,0.0,0.0);  //rotate our camera on teh

	glRotatef(roty,0.0,1.0,0.0);
	glTranslatef(-eye.x, -eye.y, -eye.z);
	//glLoadIdentity();
    }


    void orthogonal_camera(int w,int h) {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0,static_cast<float>(w),static_cast<float>(h),0.f,-1.f,1.f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
    }
};

#endif // GLCOMMON_H
