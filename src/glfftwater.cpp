
#include "glfftwater.h"
#include <tr1/random>
#include <malloc.h>
#include <math.h>
#include <pthread.h>

GLFFTWater::GLFFTWater(GLFFTWaterParams &params) {
#ifdef _WIN32
    m_h = (float *)__mingw_aligned_malloc((sizeof(float)*(params.N+2)*(params.N)), 4);
    m_dx = (float *)__mingw_aligned_malloc((sizeof(float)*(params.N+2)*(params.N)), 4);
    m_dz = (float *)__mingw_aligned_malloc((sizeof(float)*(params.N+2)*(params.N)), 4);
    m_w = (float *)__mingw_aligned_malloc((sizeof(float)*(params.N)*(params.N)), 4);
#else
    posix_memalign((void **)&m_h,4,sizeof(float)*(params.N+2)*(params.N));
    posix_memalign((void **)&m_dx,4,sizeof(float)*(params.N+2)*(params.N));
    posix_memalign((void **)&m_dz,4,sizeof(float)*(params.N+2)*(params.N));
    posix_memalign((void **)&m_w,4,sizeof(float)*(params.N)*(params.N));
#endif

    m_htilde0 = (fftwf_complex *)fftwf_malloc(sizeof(fftwf_complex)*(params.N)*(params.N));
    m_heightmap = new float3[(params.N)*(params.N)];
    m_params = params;

    std::tr1::mt19937 prng(1337);
    std::tr1::normal_distribution<float> normal;
    std::tr1::uniform_real<float> uniform;
    std::tr1::variate_generator<std::tr1::mt19937, std::tr1::normal_distribution<float> > randn(prng,normal);
    std::tr1::variate_generator<std::tr1::mt19937, std::tr1::uniform_real<float> > randu(prng,uniform);
    for(int i=0, k=0; i<params.N; i++) {
	    float k_x = (-(params.N-1)*0.5f+i)*(2.f*3.141592654f / params.L);
	    for(int j=0; j<params.N; j++, k++) {
		    float k_y = (-(params.N-1)*0.5f+j)*(2.f*3.141592654f / params.L);
		    float A = randn();
		    float theta = randu()*2.f*3.141592654f;
		    float P = (k_x==0.f && k_y==0.0f) ? 0.f : sqrtf(phillips(k_x,k_y,m_w[k]));
		    m_htilde0[k][0] = m_htilde0[k][1] = P*A*sinf(theta);
	    }
    }


    m_kz = new float[params.N*(params.N / 2 + 1)];
    m_kx = new float[params.N*(params.N / 2 + 1)];

    const int hN = m_params.N / 2;
    for(int y=0; y<m_params.N; y++) {
	float kz = (float) (y - hN);
	for(int x=0; x<=hN; x++) {
		float kx = (float) (x - hN);
		float k = 1.f/sqrtf(kx*kx+kz*kz);
		m_kz[y*(hN+1)+x] = kz*k;
		m_kx[y*(hN+1)+x] = kx*k;
	}
    }

    if(!fftwf_init_threads()) {
	cerr << "Error initializing multithreaded fft."  << endl;
    } else {
	fftwf_plan_with_nthreads(2);
    }
  
    m_fftplan = fftwf_plan_dft_c2r_2d(m_params.N, m_params.N, (fftwf_complex *)m_h, m_h, 
				      FFTW_ESTIMATE);

    glGenTextures(1, &m_texId);
    glBindTexture(GL_TEXTURE_2D, m_texId);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, params.N, params.N, 0, GL_RGB, GL_FLOAT, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

GLFFTWater::~GLFFTWater() {
    fftwf_free(m_htilde0);
    __mingw_aligned_free(m_w);
    __mingw_aligned_free(m_h);
    __mingw_aligned_free(m_dx);
    __mingw_aligned_free(m_dz);
    delete[] m_kz;
    delete[] m_heightmap;
    delete[] m_kx; 
}

float GLFFTWater::phillips(float kx, float ky, float& w) {
	const float damping = 1.f / 2.f;
	float kk = kx*kx+ky*ky;
	float kw = kx*cosf(m_params.w)+ky*sinf(m_params.w);
	float l = m_params.V*m_params.V /  9.81;
	w = powf((9.81*sqrtf(kk)),1.0f); //compute the dispersion relation
	float p = m_params.A*expf(-1.f / (l*l*kk))/(kk*kk*kk)*(kw*kw);
	float d = expf(-kk*damping*damping);
	return kw < 0.f ? p*0.25f*d : p*d;
}

GLuint GLFFTWater::heightfieldTexture() {
    glBindTexture(GL_TEXTURE_2D, m_texId);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_params.N, m_params.N, GL_RGB, GL_FLOAT, m_heightmap);
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F_ARB, m_params.N, m_params.N, 0, GL_RGB, GL_FLOAT, m_heightmap);
    glBindTexture(GL_TEXTURE_2D, 0);
    return m_texId;
}

struct ThreadInfo {
    GLFFTWater *water;
    float time;
} info;

void *heightfieldThread(void *argument) {
   GLFFTWater *water = ((ThreadInfo *)argument)->water;
   water->computeHeightfield(((ThreadInfo *)argument)->time);
}


void GLFFTWater::startHeightfieldComputeThread(float t) {
    info.time = t;
    info.water = this;
    pthread_create(&computeThread_, 0, heightfieldThread, (void *)&info);
}

void GLFFTWater::waitForHeightfieldComputeThread() {
    pthread_join(computeThread_, 0);
    pthread_detach(computeThread_);
}

void GLFFTWater::computeHeightfield(float t) {
	const int hN = m_params.N / 2;
	const int hNp2 = m_params.N + 2;

	for(int y=0; y<m_params.N; y++) {
	    int nk_x=m_params.N-1-y;
	    for(int x=0; x<=hN; x++) {
		    int nk_y=m_params.N-1-x;
		    int idx = (nk_x)*(m_params.N)+nk_y;
		    float pcos = cosf(m_w[y*m_params.N+x]*t);
		    float psin = sinf(m_w[y*m_params.N+x]*t);
		    // @TODO: check this math simplification ...
		    float ht_r = m_htilde0[y*m_params.N+x][0]*pcos-m_htilde0[y*m_params.N+x][0]*psin+
				 m_htilde0[idx][0]*pcos-m_htilde0[idx][0]*psin;
		    float ht_c = -2.0f*m_htilde0[idx][0]*psin;

		    m_h[x*2+hNp2*y] = ht_r;
		    m_h[x*2+1+hNp2*y] = ht_c;

		    float kkx = m_kx[y*(hN+1)+x];
		    float kkz = m_kz[y*(hN+1)+x];

		    m_dx[x*2+hNp2*y] =  -ht_c*kkx;
		    m_dx[x*2+1+hNp2*y] =  ht_r*kkx;
		    m_dz[x*2+hNp2*y] =  -ht_c*kkz;
		    m_dz[x*2+1+hNp2*y] =  ht_r*kkz;
	    }
	}
	
	m_dx[m_params.N + hNp2*hN] = 0.f;
	m_dz[m_params.N + hNp2*hN] = 0.f;
	m_dx[m_params.N + hNp2*hN+1] = 0.f;
	m_dz[m_params.N + hNp2*hN+1] = 0.f;
	

	fftwf_execute_dft_c2r(m_fftplan, (fftwf_complex *)m_h, m_h);
	fftwf_execute_dft_c2r(m_fftplan, (fftwf_complex *)m_dx, m_dx);
	fftwf_execute_dft_c2r(m_fftplan, (fftwf_complex *)m_dz, m_dz);

	const float scale = m_params.L / (float)m_params.N * m_params.chop;
	for(int y=0; y<m_params.N;y++) {
		for(int x=0; x<m_params.N; x++) {
			if((x+y)%2==0) {
				m_heightmap[x+m_params.N*y].x = m_dx[x+hNp2*y]*scale;
				m_heightmap[x+m_params.N*y].y = m_h[x+hNp2*y];
				m_heightmap[x+m_params.N*y].z = m_dz[x+hNp2*y]*scale;
			} else {
				m_heightmap[x+m_params.N*y].x = -m_dx[x+hNp2*y]*scale;
				m_heightmap[x+m_params.N*y].y = -m_h[x+hNp2*y];
				m_heightmap[x+m_params.N*y].z = -m_dz[x+hNp2*y]*scale;
			}
		}
	}
}
