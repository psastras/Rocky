#include "vsml.h"
#include <math.h>

// This var keeps track of the single instance of VSML
VSML* VSML::gInstance = 0;

#define M_PI       3.14159265358979323846f

static inline float
DegToRad(float degrees)
{
	return (float)(degrees * (M_PI / 180.0f));
};

// Singleton implementation
// use this function to get the instance of VSML
VSML*
VSML::getInstance (void) {

	if (0 != gInstance)
		return gInstance;
	else
		gInstance = new VSML();

	return gInstance;
}

// VSML constructor
VSML::VSML():
		mInit(false),
		mBlocks(false)
{

}

// VSML destructor
VSML::~VSML()
{
}

// send the buffer data and offsets to VSML
void
VSML::initUniformBlock(GLuint buffer, GLuint modelviewOffset, GLuint projOffset)
{
	mInit = true;
	mBlocks = true;
	mBuffer = buffer;
	mOffset[MODELVIEW] = modelviewOffset;
	mOffset[PROJECTION] = projOffset;
}

// send the uniform locations to VSML
void
VSML::initUniformLocs(GLuint modelviewLoc, GLuint projLoc)
{
	mInit = true;
	mBlocks = false;
	mUniformLoc[MODELVIEW] = modelviewLoc;
	mUniformLoc[PROJECTION] = projLoc;
}

// glPushMatrix implementation
void
VSML::pushMatrix(MatrixTypes aType) {

	float *aux = (float *)malloc(sizeof(float) * 16);
	memcpy(aux, mMatrix[aType], sizeof(float) * 16);
	mMatrixStack[aType].push_back(aux);
}

// glPopMatrix implementation
void
VSML::popMatrix(MatrixTypes aType) {

	float *m = mMatrixStack[aType][mMatrixStack[aType].size()-1];
	memcpy(mMatrix[aType], m, sizeof(float) * 16);
	mMatrixStack[aType].pop_back();
	free(m);

#ifdef VSML_ALWAYS_SEND_TO_OPENGL
	matrixToGL(aType);
#endif

}

// glLoadIdentity implementation
void
VSML::loadIdentity(MatrixTypes aType)
{
	setIdentityMatrix(mMatrix[aType]);

#ifdef VSML_ALWAYS_SEND_TO_OPENGL
	matrixToGL(aType);
#endif
}

// glMultMatrix implementation
void
VSML::multMatrix(MatrixTypes aType, float *aMatrix)
{

	float *a, *b, res[16];
	a = mMatrix[aType];
	b = aMatrix;

	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			res[j*4 + i] = 0.0f;
			for (int k = 0; k < 4; ++k) {
				res[j*4 + i] += a[k*4 + i] * b[j*4 + k];
			}
		}
	}
	memcpy(mMatrix[aType], res, 16 * sizeof(float));

#ifdef VSML_ALWAYS_SEND_TO_OPENGL
	matrixToGL(aType);
#endif
}

// glLoadMatrix implementation
void
VSML::loadMatrix(MatrixTypes aType, float *aMatrix)
{
	memcpy(mMatrix[aType], aMatrix, 16 * sizeof(float));

#ifdef VSML_ALWAYS_SEND_TO_OPENGL
	matrixToGL(aType);
#endif
}

// glTranslate implementation with matrix selection
void
VSML::translate(MatrixTypes aType, float x, float y, float z)
{
	float mat[16];

	setIdentityMatrix(mat);
	mat[12] = x;
	mat[13] = y;
	mat[14] = z;

	multMatrix(aType,mat);

#ifdef VSML_ALWAYS_SEND_TO_OPENGL
	matrixToGL(aType);
#endif
}

// glTranslate on the MODELVIEW matrix
void
VSML::translate(float x, float y, float z)
{
	translate(MODELVIEW, x,y,z);
}

// glScale implementation with matrix selection
void
VSML::scale(MatrixTypes aType, float x, float y, float z)
{
	float mat[16];

	setIdentityMatrix(mat,4);
	mat[0] = x;
	mat[5] = y;
	mat[10] = z;

	multMatrix(aType,mat);

#ifdef VSML_ALWAYS_SEND_TO_OPENGL
	matrixToGL(aType);
#endif
}

// glScale on the MODELVIEW matrix
void
VSML::scale(float x, float y, float z)
{
	scale(MODELVIEW, x, y, z);
}

// glRotate implementation with matrix selection
void
VSML::rotate(MatrixTypes aType, float angle, float x, float y, float z)
{
	float mat[16];

	float radAngle = DegToRad(angle);
	float co = cos(radAngle);
	float si = sin(radAngle);
	float x2 = x*x;
	float y2 = y*y;
	float z2 = z*z;

	mat[0] = x2 + (y2 + z2) * co;
	mat[4] = x * y * (1 - co) - z * si;
	mat[8] = x * z * (1 - co) + y * si;
	mat[12]= 0.0f;

	mat[1] = x * y * (1 - co) + z * si;
	mat[5] = y2 + (x2 + z2) * co;
	mat[9] = y * z * (1 - co) - x * si;
	mat[13]= 0.0f;

	mat[2] = x * z * (1 - co) - y * si;
	mat[6] = y * z * (1 - co) + x * si;
	mat[10]= z2 + (x2 + y2) * co;
	mat[14]= 0.0f;

	mat[3] = 0.0f;
	mat[7] = 0.0f;
	mat[11]= 0.0f;
	mat[15]= 1.0f;

	multMatrix(aType,mat);

#ifdef VSML_ALWAYS_SEND_TO_OPENGL
	matrixToGL(aType);
#endif
}

// glRotate implementation in the MODELVIEW matrix
void
VSML::rotate(float angle, float x, float y, float z)
{
	rotate(MODELVIEW,angle,x,y,z);
}

// gluLookAt implementation
void
VSML::lookAt(float xPos, float yPos, float zPos,
					float xLook, float yLook, float zLook,
					float xUp, float yUp, float zUp)
{
	float dir[3], right[3], up[3];

	up[0] = xUp;	up[1] = yUp;	up[2] = zUp;

	dir[0] =  (xLook - xPos);
	dir[1] =  (yLook - yPos);
	dir[2] =  (zLook - zPos);
	normalize(dir);

	crossProduct(dir,up,right);
	normalize(right);

	crossProduct(right,dir,up);
	normalize(up);

	float *viewMatrix,mat[16];

	viewMatrix = mMatrix[MODELVIEW];

	viewMatrix[0]  = right[0];
	viewMatrix[4]  = right[1];
	viewMatrix[8]  = right[2];
	viewMatrix[12] = 0.0f;

	viewMatrix[1]  = up[0];
	viewMatrix[5]  = up[1];
	viewMatrix[9]  = up[2];
	viewMatrix[13] = 0.0f;

	viewMatrix[2]  = -dir[0];
	viewMatrix[6]  = -dir[1];
	viewMatrix[10] = -dir[2];
	viewMatrix[14] =  0.0f;

	viewMatrix[3]  = 0.0f;
	viewMatrix[7]  = 0.0f;
	viewMatrix[11] = 0.0f;
	viewMatrix[15] = 1.0f;

	setIdentityMatrix(mat,4);
	mat[12] = -xPos;
	mat[13] = -yPos;
	mat[14] = -zPos;

	multMatrix(MODELVIEW, mat);

#ifdef VSML_ALWAYS_SEND_TO_OPENGL
	matrixToGL(MODELVIEW);
#endif
}

// gluPerspective implementation
void
VSML::perspective(float fov, float ratio, float nearp, float farp)
{
	float *projMatrix = mMatrix[PROJECTION];

	float f = 1.0f / tan (fov * (M_PI / 360.0f));

	setIdentityMatrix(projMatrix,4);

	projMatrix[0] = f / ratio;
	projMatrix[1 * 4 + 1] = f;
	projMatrix[2 * 4 + 2] = (farp + nearp) / (nearp - farp);
	projMatrix[3 * 4 + 2] = (2.0f * farp * nearp) / (nearp - farp);
	projMatrix[2 * 4 + 3] = -1.0f;
	projMatrix[3 * 4 + 3] = 0.0f;

#ifdef VSML_ALWAYS_SEND_TO_OPENGL
	matrixToGL(PROJECTION);
#endif
}

// glOrtho implementation
void
VSML::ortho(float left, float right, float bottom, float top, float nearp, float farp)
{
	float *m = mMatrix[PROJECTION];

	setIdentityMatrix(m,4);

	m[0 * 4 + 0] = 2 / (right - left);
	m[1 * 4 + 1] = 2 / (top - bottom);
	m[2 * 4 + 2] = -2 / (farp - nearp);
	m[3 * 4 + 0] = -(right + left) / (right - left);
	m[3 * 4 + 1] = -(top + bottom) / (top - bottom);
	m[3 * 4 + 2] = -(farp + nearp) / (farp - nearp);

#ifdef VSML_ALWAYS_SEND_TO_OPENGL
	matrixToGL(PROJECTION);
#endif
}

// glFrustum implementation
void
VSML::frustum(float left, float right, float bottom, float top, float nearp, float farp)
{
	float *m = mMatrix[PROJECTION];

	setIdentityMatrix(m,4);

	m[0 * 4 + 0] = 2 * nearp / (right-left);
	m[1 * 4 + 1] = 2 * nearp / (top - bottom);
	m[2 * 4 + 0] = (right + left) / (right - left);
	m[2 * 4 + 1] = (top + bottom) / (top - bottom);
	m[2 * 4 + 2] = (farp + nearp) / (farp - nearp);
	m[2 * 4 + 3] = -1.0f;
	m[3 * 4 + 2] = 2 * farp * nearp / (farp-nearp);
	m[3 * 4 + 3] = 0.0f;

#ifdef VSML_ALWAYS_SEND_TO_OPENGL
	matrixToGL(PROJECTION);
#endif
}

// returns a pointer to the requested matrix
float *
VSML::get(MatrixTypes aType)
{
	return mMatrix[aType];
}

/* -----------------------------------------------------
             SEND MATRICES TO OPENGL
------------------------------------------------------*/

// to be used with uniform blocks
void
VSML::matrixToBuffer(MatrixTypes aType)
{
	if (mInit && mBlocks) {
		glBindBuffer(GL_UNIFORM_BUFFER,mBuffer);
		glBufferSubData(GL_UNIFORM_BUFFER, mOffset[aType], 16 * sizeof(float), mMatrix[aType]);
		glBindBuffer(GL_UNIFORM_BUFFER,0);

	}
}

// to be used with uniform variables
void
VSML::matrixToUniform(MatrixTypes aType)
{
	if (mInit && !mBlocks) {

		glUniformMatrix4fv(mUniformLoc[aType], 1, false, mMatrix[aType]);
	}
}

// universal
void
VSML::matrixToGL(MatrixTypes aType)
{
	if (mInit) {

		if (mBlocks) {
			glBindBuffer(GL_UNIFORM_BUFFER,mBuffer);
			glBufferSubData(GL_UNIFORM_BUFFER, mOffset[aType], 16 * sizeof(float), mMatrix[aType]);
			glBindBuffer(GL_UNIFORM_BUFFER,0);
		}
		else {
			glUniformMatrix4fv(mUniformLoc[aType], 1, false, mMatrix[aType]);
		}

	}
}

// -----------------------------------------------------
//                      AUX functions
// -----------------------------------------------------

// sets the square matrix mat to the identity matrix,
// size refers to the number of rows (or columns)
void
VSML::setIdentityMatrix( float *mat, int size) {

	// fill matrix with 0s
	for (int i = 0; i < size * size; ++i)
			mat[i] = 0.0f;

	// fill diagonal with 1s
	for (int i = 0; i < size; ++i)
		mat[i + i * size] = 1.0f;
}

// res = a cross b;
void
VSML::crossProduct( float *a, float *b, float *res) {

	res[0] = a[1] * b[2]  -  b[1] * a[2];
	res[1] = a[2] * b[0]  -  b[2] * a[0];
	res[2] = a[0] * b[1]  -  b[0] * a[1];
}

// Normalize a vec3
void
VSML::normalize(float *a) {

	float mag = sqrt(a[0] * a[0]  +  a[1] * a[1]  +  a[2] * a[2]);

	a[0] /= mag;
	a[1] /= mag;
	a[2] /= mag;
}
