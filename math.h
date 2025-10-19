#ifndef _MATH_H_
#define _MATH_H_

/* Header containing the Math Definitions and Operations
 *
 * math.h
 * gam670/dps905
 * version 1.0
 * Jan 11 2010
 * Chris Szalwinski
 */

#include <math.h>
#include "Settings.h" // for ZAXIS_DIRECTION

struct Matrix;

struct Vector {
    float x;
    float y;
    float z;
    Vector() : x(0), y(0), z(0) {}
	Vector(float a) : x(a), y(a), z(a) {}
    Vector(float xx, float yy, float zz) : x(xx), y(yy), z(zz) {}
    Vector  operator+() const;
    Vector  operator-() const;
    Vector& operator+=(const Vector& a);
    Vector& operator-=(const Vector& a);
    Vector  operator*(const Matrix& m);
    Vector  operator*(const Matrix& m) const;
    Vector  operator*=(const Matrix& m);
    float   length() const;
};

inline Vector operator+(const Vector& a, const Vector& b) {

    return Vector(a.x + b.x, a.y + b.y, a.z + b.z);
}

inline Vector operator-(const Vector& a, const Vector& b) {

    return Vector(a.x - b.x, a.y - b.y, a.z - b.z);
}

inline Vector Vector::operator+() const {

    return *this;
}

inline Vector Vector::operator-() const {

    return Vector(-x, -y, -z);
}

inline Vector operator*(float s, const Vector& a) {

    return Vector(s * a.x, s * a.y, s * a.z);
}

inline Vector operator*(const Vector& a, float s) {

    return Vector(s * a.x, s * a.y, s * a.z);
}

inline Vector operator/(const Vector& a, float x) {

    return Vector(x ? a.x / x : a.x, x ? a.y / x : a.y, x ? a.z / x : a.z);
}

inline Vector& Vector::operator+=(const Vector& a) {

    x += a.x;
    y += a.y;
    z += a.z;
    return *this;
}

inline Vector& Vector::operator-=(const Vector& a) {

    x -= a.x;
    y -= a.y;
    z -= a.z;
    return *this;
}

inline Vector cross(const Vector& a, const Vector& b) {

    return Vector(a.y * b.z - a.z * b.y,
                  a.z * b.x - a.x * b.z,
                  a.x * b.y - a.y * b.x);
}

inline float dot(const Vector& a, const Vector& b) {

    return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline float Vector::length() const {

    return sqrtf(dot(*this, *this));
}

inline Vector normal(const Vector& a) {

    return a / a.length();
}

struct Plane {

    Vector n;
    float d;
    Plane() : n(Vector(0, 0, 0)), d(0) {}
    Plane(const Vector& nn, float dd) : n(nn), d(dd) {}
};

struct Matrix {
    float m11, m12, m13, m14;
    float m21, m22, m23, m24;
    float m31, m32, m33, m34;
    float m41, m42, m43, m44;
    Matrix() : m11(0), m12(0), m13(0), m14(0),
               m21(0), m22(0), m23(0), m24(0),
               m31(0), m32(0), m33(0), m34(0),
               m41(0), m42(0), m43(0), m44(0) {}
    Matrix(float x) : m11(x), m12(0), m13(0), m14(0),
                      m21(0), m22(x), m23(0), m24(0),
                      m31(0), m32(0), m33(x), m34(0),
                      m41(0), m42(0), m43(0), m44(1) {}
    Matrix(const Vector& v) : m11(v.x), m12(0), m13(0), m14(0),
                              m21(0), m22(v.y), m23(0), m24(0),
                              m31(0), m32(0), m33(v.z), m34(0),
                              m41(0), m42(0), m43(0), m44(1) {}
    Matrix(float i11, float i12, float i13, float i14,
           float i21, float i22, float i23, float i24,
           float i31, float i32, float i33, float i34,
           float i41, float i42, float i43, float i44) :
               m11(i11), m12(i12), m13(i13), m14(i14),
               m21(i21), m22(i22), m23(i23), m24(i24),
               m31(i31), m32(i32), m33(i33), m34(i34),
               m41(i41), m42(i42), m43(i43), m44(i44) {}
    Matrix& isIdentity();
	Matrix  operator+ (const Matrix& a);
    Matrix& operator+=(const Matrix& a);
    Matrix& operator-=(const Matrix& a);
    Matrix& operator*=(const Matrix& m);
    Matrix  transpose();
};

inline Vector Vector::operator*(const Matrix& m) {

    return Vector(x * m.m11 + y * m.m21 + z * m.m31 + m.m41,
                  x * m.m12 + y * m.m22 + z * m.m32 + m.m42,
                  x * m.m13 + y * m.m23 + z * m.m33 + m.m43);
}

inline Vector Vector::operator*(const Matrix& m) const {

    return Vector(x * m.m11 + y * m.m21 + z * m.m31 + m.m41,
                  x * m.m12 + y * m.m22 + z * m.m32 + m.m42,
                  x * m.m13 + y * m.m23 + z * m.m33 + m.m43);
}

inline Vector Vector::operator*=(const Matrix& m) {

    *this = Vector(x * m.m11 + y * m.m21 + z * m.m31 + m.m41,
                   x * m.m12 + y * m.m22 + z * m.m32 + m.m42,
                   x * m.m13 + y * m.m23 + z * m.m33 + m.m43);
    return *this;
}

inline Matrix& Matrix::isIdentity() {

    return *this = Matrix(1);
}

inline Matrix Matrix::transpose() {
    return Matrix(m11, m21, m31, m41,
     m12, m22, m32, m42,
     m13, m23, m33, m43,
     m14, m24, m34, m44);
}

inline Matrix& Matrix::operator+=(const Matrix& a) {

    m11 += a.m11;
    m12 += a.m12;
    m13 += a.m13;
    m14 += a.m14;
    m21 += a.m21;
    m22 += a.m22;
    m23 += a.m23;
    m24 += a.m24;
    m31 += a.m31;
    m32 += a.m32;
    m33 += a.m33;
    m34 += a.m34;
    m41 += a.m41;
    m42 += a.m42;
    m43 += a.m43;
    m44 += a.m44;
    return *this;
}

inline Matrix Matrix::operator+(const Matrix& a) {

    return Matrix(   
	m11 + a.m11,
    m12 + a.m12,
    m13 + a.m13,
    m14 + a.m14,
    m21 + a.m21,
    m22 + a.m22,
    m23 + a.m23,
    m24 + a.m24,
    m31 + a.m31,
    m32 + a.m32,
    m33 + a.m33,
    m34 + a.m34,
    m41 + a.m41,
    m42 + a.m42,
    m43 + a.m43,
    m44 + a.m44);
}

// projectOnto returns the vector projection of a onto b
// (a dot b)/(b dot b) * b - for use in orthonormalize
//
inline Vector projectOnto(const Vector& a, const Vector& b) {

	return (dot(a, b) / dot(b, b)) * b;
}

inline Matrix orthoNormalize(const Matrix& m) {

	Vector row1 = normal(Vector(m.m11, m.m12, m.m13));
	Vector pr   = projectOnto(row1, Vector(m.m21, m.m22, m.m23));
	Vector row2 = normal(Vector(m.m21, m.m22, m.m23) - pr);
	Vector row3 = normal(cross(row1, row2));
	row2 = cross(row3, row1);

	return Matrix(row1.x, row1.y, row1.z, m.m14,
				  row2.x, row2.y, row2.z, m.m24,
				  row3.x, row3.y, row3.z, m.m34,
			       m.m41,  m.m42,  m.m43, m.m44);

}

inline Matrix& Matrix::operator-=(const Matrix& a) {

    m11 -= a.m11;
    m12 -= a.m12;
    m13 -= a.m13;
    m14 -= a.m14;
    m21 -= a.m21;
    m22 -= a.m22;
    m23 -= a.m23;
    m24 -= a.m24;
    m31 -= a.m31;
    m32 -= a.m32;
    m33 -= a.m33;
    m34 -= a.m34;
    m41 -= a.m41;
    m42 -= a.m42;
    m43 -= a.m43;
    m44 -= a.m44;
    return *this;
}

inline Matrix operator*(const Matrix& a, const Matrix& b) {

    return Matrix(a.m11 * b.m11 + a.m12 * b.m21 + a.m13 * b.m31 + a.m14 * b.m41,
                  a.m11 * b.m12 + a.m12 * b.m22 + a.m13 * b.m32 + a.m14 * b.m42,
                  a.m11 * b.m13 + a.m12 * b.m23 + a.m13 * b.m33 + a.m14 * b.m43,
                  a.m11 * b.m14 + a.m12 * b.m24 + a.m13 * b.m34 + a.m14 * b.m44,
                  a.m21 * b.m11 + a.m22 * b.m21 + a.m23 * b.m31 + a.m24 * b.m41,
                  a.m21 * b.m12 + a.m22 * b.m22 + a.m23 * b.m32 + a.m24 * b.m42,
                  a.m21 * b.m13 + a.m22 * b.m23 + a.m23 * b.m33 + a.m24 * b.m43,
                  a.m21 * b.m14 + a.m22 * b.m24 + a.m23 * b.m34 + a.m24 * b.m44,
                  a.m31 * b.m11 + a.m32 * b.m21 + a.m33 * b.m31 + a.m34 * b.m41,
                  a.m31 * b.m12 + a.m32 * b.m22 + a.m33 * b.m32 + a.m34 * b.m42,
                  a.m31 * b.m13 + a.m32 * b.m23 + a.m33 * b.m33 + a.m34 * b.m43,
                  a.m31 * b.m14 + a.m32 * b.m24 + a.m33 * b.m34 + a.m34 * b.m44,
                  a.m41 * b.m11 + a.m42 * b.m21 + a.m43 * b.m31 + a.m44 * b.m41,
                  a.m41 * b.m12 + a.m42 * b.m22 + a.m43 * b.m32 + a.m44 * b.m42,
                  a.m41 * b.m13 + a.m42 * b.m23 + a.m43 * b.m33 + a.m44 * b.m43,
                  a.m41 * b.m14 + a.m42 * b.m24 + a.m43 * b.m34 + a.m44 * b.m44);
}

inline Matrix& Matrix::operator*=(const Matrix& a) {

    return *this =
           Matrix(m11 * a.m11 + m12 * a.m21 + m13 * a.m31 + m14 * a.m41,
                  m11 * a.m12 + m12 * a.m22 + m13 * a.m32 + m14 * a.m42,
                  m11 * a.m13 + m12 * a.m23 + m13 * a.m33 + m14 * a.m43,
                  m11 * a.m14 + m12 * a.m24 + m13 * a.m34 + m14 * a.m44,
                  m21 * a.m11 + m22 * a.m21 + m23 * a.m31 + m24 * a.m41,
                  m21 * a.m12 + m22 * a.m22 + m23 * a.m32 + m24 * a.m42,
                  m21 * a.m13 + m22 * a.m23 + m23 * a.m33 + m24 * a.m43,
                  m21 * a.m14 + m22 * a.m24 + m23 * a.m34 + m24 * a.m44,
                  m31 * a.m11 + m32 * a.m21 + m33 * a.m31 + m34 * a.m41,
                  m31 * a.m12 + m32 * a.m22 + m33 * a.m32 + m34 * a.m42,
                  m31 * a.m13 + m32 * a.m23 + m33 * a.m33 + m34 * a.m43,
                  m31 * a.m14 + m32 * a.m24 + m33 * a.m34 + m34 * a.m44,
                  m41 * a.m11 + m42 * a.m21 + m43 * a.m31 + m44 * a.m41,
                  m41 * a.m12 + m42 * a.m22 + m43 * a.m32 + m44 * a.m42,
                  m41 * a.m13 + m42 * a.m23 + m43 * a.m33 + m44 * a.m43,
                  m41 * a.m14 + m42 * a.m24 + m43 * a.m34 + m44 * a.m44);
}

inline Vector operator*(const Matrix& a, const Vector& b) {

    return Vector(a.m11 * b.x + a.m12 * b.y + a.m13 * b.z + a.m14,
                  a.m21 * b.x + a.m22 * b.y + a.m23 * b.z + a.m24,
                  a.m31 * b.x + a.m32 * b.y + a.m33 * b.z + a.m34);
}

inline Matrix& scale(Matrix& m, float x, float y, float z) {

    return m = Matrix(x, 0, 0, 0,
                      0, y, 0, 0,
                      0, 0, z, 0,
                      0, 0, 0, 1);
}

inline Matrix& scale(Matrix& m, const Vector& v) {

    return scale(m, v.x, v.y, v.z);
}

inline Matrix& translate(Matrix& m, float x, float y, float z) {

    return m = Matrix(1, 0, 0, 0,
                      0, 1, 0, 0,
                      0, 0, 1, 0,
                      x, y, ZAXIS_DIRECTION * z, 1);
}

inline Matrix& translate(Matrix& m, const Vector& v) {

    return translate(m, v.x, v.y, v.z);
}

inline Matrix& rotatex(Matrix& m, float rad) {

    float c = cos(rad);
    float s = ZAXIS_DIRECTION * sin(rad);
    m = Matrix(1, 0, 0, 0,
               0, c, s, 0,
               0,-s, c, 0,
               0, 0, 0, 1);
    return m;
}

inline Matrix& rotatey(Matrix& m, float rad) {

    float c = cos(rad);
    float s = ZAXIS_DIRECTION * sin(rad);
    m = Matrix(c, 0, s, 0,
               0, 1, 0, 0,
              -s, 0, c, 0,
               0, 0, 0, 1);
    return m;
}

inline Matrix& rotatez(Matrix& m, float rad) {

    float c = cos(rad);
    float s = ZAXIS_DIRECTION * sin(rad);
    m = Matrix(c, s, 0, 0,
              -s, c, 0, 0,
               0, 0, 1, 0,
               0, 0, 0, 1);
    return m;
}

inline Matrix rotate(Vector& axis, float rad) {

    float c = cos(rad);
    float s = ZAXIS_DIRECTION * sin(rad);
    float t = 1.f - c;
    Vector a = normal(axis);
    return Matrix(t*a.x*a.x + c,     t*a.x*a.y + a.z*s, t*a.x*a.z - a.y*s, 0,
                  t*a.x*a.y - a.z*s, t*a.y*a.y + c,     t*a.y*a.z + a.x*s, 0,
                  t*a.x*a.z + a.y*s, t*a.y*a.z - a.x*s, t*a.z*a.z + c,     0,
                  0,                 0,                 0,                 1);
}

// position extracts the position vector from a homogeneous transformation
//
inline Vector position(const Matrix& m) {

    return Vector(m.m41, m.m42, m.m43);
}

// rotation extracts the rotation transformation from a homogeneous
// transformation assuming that there has not been any scaling
//
inline Matrix rotation(const Matrix& m) {

    return Matrix(m.m11, m.m12, m.m13, 0,
                  m.m21, m.m22, m.m23, 0,
                  m.m31, m.m32, m.m33, 0,
                      0,     0,     0, 1);
}

inline Matrix& view(Matrix& m, const Vector& p, const Vector& d,
 const Vector& u) {

    Vector z = normal(d - p);
    Vector x = normal(cross(u, z));
    Vector y = cross(z, x);
    m = Matrix(      x.x,        y.x,        z.x, 0,
                     x.y,        y.y,        z.y, 0,
                     x.z,        y.z,        z.z, 0,
              -dot(x, p), -dot(y, p), -dot(z, p), 1);
    return m;
}

inline Matrix& projectionFov(Matrix& m, float fov, float aspect,
 float near_cp, float far_cp) {    
	float sy = 1.0f / tan(fov * 0.5f);
    float sx = sy / aspect;
	float sz = far_cp/(far_cp-near_cp);
    m = Matrix(sx,  0,           0, 0,
                0, sy,           0, 0,
                0,  0,          sz, 1,
                0,  0, -near_cp*sz, 0);
    return m;
}

struct Colour {
    float r;
    float g;
    float b;
    float a;
    Colour(float red = 0, float grn = 0, float blu = 0, float alp = 1) :
     r(red), g(grn), b(blu), a(alp) {}
    operator const float* () const { return &r; }
};

#endif