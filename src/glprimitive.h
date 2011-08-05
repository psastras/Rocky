#ifndef GLPRIMITIVE_H
#define GLPRIMITIVE_H

#include "glcommon.h"
#include "glshaderprogram.h"
struct GLVertex
{
  float3 p, n, t;
};


class GLPrimitive {
  public:
      ~GLPrimitive();

      virtual void tesselate(float3 tess, float3 translate, float3 scale) = 0; //tesselates and reuploads into vbo
      void draw();
      void draw(GLShaderProgram *program);
      void draw(GLShaderProgram *program, int instances);

      const float3& scale() { return scale_; }
      const float3& translate() { return scale_; }
  protected:
      GLPrimitive(float3 &tess, float3 &translate, float3 &scale);

      GLuint vertexId_, indexId_, arrayId_;
      GLenum type_;
      GLuint idxCount_;
      int vOffset_, tOffset_, nOffset_;

      float3 scale_, translate_;
};

class GLQuad : public GLPrimitive {
    public:
	GLQuad(float3 tess, float3 translate, float3 scale);
	~GLQuad();

	void tesselate(float3 tess, float3 translate, float3 scale);
};

class GLPlane : public GLPrimitive {
    public:
	GLPlane(float3 tess, float3 translate, float3 scale);
	~GLPlane();

	void tesselate(float3 tess, float3 translate, float3 scale);
};

class GLIcosohedron : public GLPrimitive {
    public:
	GLIcosohedron(float3 tess, float3 translate, float3 scale);
	~GLIcosohedron();

	void tesselate(float3 tess, float3 translate, float3 scale);
};

#endif // GLPRIMITIVE_H
