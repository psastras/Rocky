#ifndef COMMON_H
#define COMMON_H

#include <iostream>

#include <math.h>
#include <assert.h>

using std::cerr;
using std::cout;
using std::endl;


#define SAFE_DELETE(x) if((x)) { delete (x); (x) = NULL; }
#define MAX(x, y) (x) > (y) ? (x) : (y)
#define MIN(x, y) (x) < (y) ? (x) : (y)
#define PI 3.14159265358979323846
#define DRAW_GLQUAD \
    glBegin(GL_QUADS); \
    glTexCoord2f(0.0f,1.0f); \
    glVertex2f(0.0f,0.0f); \
    glTexCoord2f(1.0f,1.0f); \
    glVertex2f(w,0.0f); \
    glTexCoord2f(1.0f,0.0f); \
    glVertex2f(w,h); \
    glTexCoord2f(0.0f,0.0f); \
    glVertex2f(0.0f,h); \
    glEnd(); \

struct float3 {


    float3(float v0 = 0, float v1 = 0, float v2 = 0) : x(v0), y(v1), z(v2) { }
    float3(float *data) { x = data[0]; y = data[1]; z = data[2]; }

    static inline float3 zero() { return float3(0,0,0); }
    static inline float3 one() { return float3(1,1,1); }

    #define VECOP_PCW(op) { x op rhs.x; y op rhs.y; z op rhs.z; return *this; }
    #define VECOP_SCA(op) { x op rhs;   y op rhs  ; z op rhs  ; return *this; }

    inline float3& operator  = (const float3& rhs) VECOP_PCW( =) /// equality assignment
    inline float3& operator += (const float3& rhs) VECOP_PCW(+=) /// piecewise addition operator
    inline float3& operator -= (const float3& rhs) VECOP_PCW(-=) /// piecewise subtraction operator
    inline float3& operator *= (const float3& rhs) VECOP_PCW(*=) /// piecewise mult operator
    inline float3& operator /= (const float3& rhs) VECOP_PCW(/=) /// piecewise div operator

    inline float3  operator  + (const float3& rhs) const { return float3(*this) += rhs; } /// piecewise addition
    inline float3  operator  - (const float3& rhs) const { return float3(*this) -= rhs; } /// piecewise subtraction
    inline float3  operator  * (const float3& rhs) const { return float3(*this) *= rhs; } /// piecewise mult
    inline float3  operator  / (const float3& rhs) const { return float3(*this) /= rhs; } /// piecewise div

    inline float3& operator += (const float  rhs)  VECOP_SCA(+=) /// scalar addition operator
    inline float3& operator -= (const float  rhs)  VECOP_SCA(-=) /// scalar subtraction operator
    inline float3& operator *= (const float  rhs)  VECOP_SCA(*=) /// scalar multiplication operator
    inline float3& operator /= (const float  rhs)  VECOP_SCA(/=) /// scalar division operator

    inline float3  operator  + (const float  rhs) const { return float3(*this) += rhs; } /// piecewise addition
    inline float3  operator  - (const float  rhs) const { return float3(*this) -= rhs; } /// piecewise subtraction
    inline float3  operator  * (const float  rhs) const { return float3(*this) *= rhs; } /// piecewise multiplication
    inline float3  operator  / (const float  rhs) const { return float3(*this) /= rhs; } /// piecewise multiplication

    #undef VECOP_PCW
    #undef VECOP_SCA

    inline float dot(const float3 &rhs) const {
	    return x * rhs.x + y * rhs.y + z * rhs.z;
    }

    inline float normalize() {
	    float m = getMagnitude();
	    x /= m, y /= m, z /= m;
	    return m;
    }

    inline float3 getNormalized() {
	    float m = getMagnitude();
	    return float3(x / m, y / m, z / m);
    }

    inline float getMagnitude() const {
	    return sqrt(getMagnitude2());
    }

    inline float getMagnitude2() const {
	    return x * x + y * y + z * z;
    }

    inline float getDistance(const float3 &rhs) const {
	    return sqrt(getDistance2(rhs));
    }

    inline float getDistance2(const float3 &rhs) const {
	    return (rhs.x - x) * (rhs.x - x) + (rhs.y - y) * (rhs.y - y) +
		    (rhs.z - z) * (rhs.z - z);
    }

    inline float3 cross(const float3& rhs) const {
	    return float3(data[1] * rhs.data[2] - data[2] * rhs.data[1],
			   data[2] * rhs.data[0] - data[0] * rhs.data[2],
			   data[0] * rhs.data[1] - data[1] * rhs.data[0]);
    }

    /// rotate around y axis
    inline float3 rotateY(float theta) {
	    return float3(z*sinf(theta)+x*cosf(theta),
			  y,
			  z*cosf(theta)-x*sinf(theta));
    }

    /// rotate around x axis
    inline float3 rotateX(float theta) {
	    return float3(x,
			  y*cosf(theta)-z*sinf(theta),
			  y*sinf(theta)+z*cosf(theta));
    }

    inline bool operator==(const float3 &rhs) {
	    return (x == rhs.x && y == rhs.y && z == rhs.z);
    }

    inline bool operator!=(const float3 &rhs) {
	    return (x != rhs.x || y != rhs.y || z != rhs.z);
    }

    union {
	struct {
	    float x, y, z;
	};
	struct {
	    float r, g, b;
	};
	float data[3];
    };
};

inline static float dot(const float3 &v1, const float3 &v2) {
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

inline float3 operator*(const float scale, const float3 &rhs) {
    return float3(rhs.x * scale, rhs.y * scale, rhs.z * scale);
}

inline float3 operator/(const float scale, const float3 &rhs) {
    return float3( scale / rhs.x , scale / rhs.y , scale / rhs.z);
}

inline float3 operator-(const float3 &rhs) {
    return float3(-rhs.x, -rhs.y, -rhs.z);
}

inline std::ostream& operator<<(std::ostream& os, const float3& f) {
	os <<"[";
	for (unsigned i = 0; i < 3; ++i) {
	    os << f.data[i] << ",";
	}
	os << "]";
	return os;
}

struct float2 {
    union {
	struct {
	    float x, y;
	};
	float data[2];
    };

    float2(float v0 = 0, float v1 = 0) : x(v0), y(v1){ }
    float2(float *data) { x = data[0]; y = data[1]; }
};

#endif
