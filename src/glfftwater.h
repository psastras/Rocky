#ifndef GLFFTWATER_H
#define GLFFTWATER_H

#include "glcommon.h"
#include <fftw3.h>

struct GLFFTWaterParams{
	int N; //fourier grid size;
	float L; //patch size
	float V; //
	float A; //global amplitude control
	float w; //wind direction in radians
	float chop;
};

class GLFFTWater
{
public:
    GLFFTWater(GLFFTWaterParams &params);
    float3 *computeHeightfield(float t);
    GLuint heightfieldTexture();
    const GLFFTWaterParams& params() { return m_params; }

protected:
    float phillips(float kx, float ky, float& w);

    GLFFTWaterParams m_params;
    fftwf_complex *m_htilde0;
    fftwf_plan m_fftplan;
    bool m_haveFFTPlan;
    float *m_w, *m_h, *m_dx, *m_dz; //the dispersion relation
    float *m_kz, *m_kx; //precomputed kz/sqrt(kx^2+kz^2)
    float3 *m_heightmap;
    GLuint m_texId;
};

#endif // GLFFTWATER_H
