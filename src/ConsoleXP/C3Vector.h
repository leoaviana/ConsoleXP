#pragma once
#define _USE_MATH_DEFINES
#include <cmath> 

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

typedef struct C2Vector
{
    float x;
    float y;
};

typedef struct C3Vector
{
    float x;
    float y;
    float z;

    C3Vector(float x, float y, float z) : x(x), y(y), z(z) {}
    C3Vector() : x(0.f), y(0.f), z(0.f) {}

    C3Vector operator-(const C3Vector& v) const {
        return C3Vector(x - v.x, y - v.y, z - v.z);
    }

    C3Vector operator+(const C3Vector& v) const {
        return C3Vector(x + v.x, y + v.y, z + v.z);
    }

    C3Vector operator*(float s) const {
        return C3Vector(x * s, y * s, z * s);
    }

    float Dot(const C3Vector& v) const {
        return x * v.x + y * v.y + z * v.z;
    }

    C3Vector Cross(const C3Vector& v) const {
        return C3Vector(
            y * v.z - z * v.y,
            z * v.x - x * v.z,
            x * v.y - y * v.x
        );
    }

    float Length() const
    {
        return sqrt(pow(x, 2) + pow(y, 2) + pow(z, 2));
    }

    float Angle() const
    {
        return atan2(y, x);
    } 

    C3Vector Normalized() const {
        float len = Length();
        return len > 0 ? C3Vector(x / len, y / len, z / len) : C3Vector();
    }

} C3Vector;


inline static float distance3D(C3Vector v1, C3Vector v2) {
    float dx = v2.x - v1.x;
    float dy = v2.y - v1.y;
    float dz = v2.z - v1.z;

    return sqrt(dx * dx + dy * dy + dz * dz);
}

inline float vectorLength(const C3Vector& vec) {
    return std::hypot(vec.x, vec.y, vec.z);
} 

inline float vectorDotProduct(const C3Vector& a, const C3Vector& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline C3Vector vectorLerp(const C3Vector& a, const C3Vector& b, float t) {
    C3Vector result;
    result.x = a.x + t * (b.x - a.x);
    result.y = a.y + t * (b.y - a.y);
    result.z = a.z + t * (b.z - a.z);
    return result;
}


inline float angleBetweenVectors(const C3Vector& a, const C3Vector& b) {
    float lenA = vectorLength(a);
    float lenB = vectorLength(b);

    if (lenA == 0.0f || lenB == 0.0f) {
        return static_cast<float>(4 * M_PI);
    }

    float dotProduct = vectorDotProduct(a, b);

    float cosValue = dotProduct / (lenA * lenB);
    if (cosValue > 1.0f || cosValue < -1.0f) {
        return static_cast<float>(4 * M_PI);
    }

    return std::acos(cosValue);
}

typedef struct C3Matrix {
    float m11, m12, m13;
    float m21, m22, m23;
    float m31, m32, m33;
};


struct C4Vector {
    float x, y, z, w;

    C4Vector() : x(0), y(0), z(0), w(0) {}
    C4Vector(float _x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w) {}
};


typedef struct CMatrix4x4 {
    float m[4][4];

    CMatrix4x4() {
        for (int i = 0; i < 4; ++i)
            for (int j = 0; j < 4; ++j)
                m[i][j] = 0.0f;
    }

    static CMatrix4x4 Identity() {
        CMatrix4x4 mat;
        mat.m[0][0] = mat.m[1][1] = mat.m[2][2] = mat.m[3][3] = 1.0f;
        return mat;
    }

    static CMatrix4x4 LookAtRH(const C3Vector& eye, const C3Vector& at, const C3Vector& up) {
        C3Vector z = (eye - at).Normalized();      // Forward
        C3Vector x = up.Cross(z).Normalized();     // Right
        C3Vector y = z.Cross(x);                   // Up (recomputed)

        CMatrix4x4 mat = Identity();

        mat.m[0][0] = x.x;
        mat.m[1][0] = x.y;
        mat.m[2][0] = x.z;
        mat.m[3][0] = -x.Dot(eye);

        mat.m[0][1] = y.x;
        mat.m[1][1] = y.y;
        mat.m[2][1] = y.z;
        mat.m[3][1] = -y.Dot(eye);

        mat.m[0][2] = z.x;
        mat.m[1][2] = z.y;
        mat.m[2][2] = z.z;
        mat.m[3][2] = -z.Dot(eye);

        mat.m[0][3] = 0.0f;
        mat.m[1][3] = 0.0f;
        mat.m[2][3] = 0.0f;
        mat.m[3][3] = 1.0f;

        return mat;
    }

    static CMatrix4x4 PerspectiveFovRH(float fovY, float aspect, float zNear, float zFar) {
        float h = 1.0f / tanf(fovY * 0.5f);
        float w = h / aspect;

        CMatrix4x4 mat = {};
        mat.m[0][0] = w;
        mat.m[1][1] = h;
        mat.m[2][2] = zFar / (zNear - zFar);
        mat.m[2][3] = -1.0f;
        mat.m[3][2] = (zNear * zFar) / (zNear - zFar);
        return mat;
    }

    CMatrix4x4 Inverted() const {
        CMatrix4x4 inv;
        float* m = (float*)this->m;
        float* out = (float*)inv.m;

        out[0] = m[5] * m[10] * m[15] -
            m[5] * m[11] * m[14] -
            m[9] * m[6] * m[15] +
            m[9] * m[7] * m[14] +
            m[13] * m[6] * m[11] -
            m[13] * m[7] * m[10];

        out[4] = -m[4] * m[10] * m[15] +
            m[4] * m[11] * m[14] +
            m[8] * m[6] * m[15] -
            m[8] * m[7] * m[14] -
            m[12] * m[6] * m[11] +
            m[12] * m[7] * m[10];

        out[8] = m[4] * m[9] * m[15] -
            m[4] * m[11] * m[13] -
            m[8] * m[5] * m[15] +
            m[8] * m[7] * m[13] +
            m[12] * m[5] * m[11] -
            m[12] * m[7] * m[9];

        out[12] = -m[4] * m[9] * m[14] +
            m[4] * m[10] * m[13] +
            m[8] * m[5] * m[14] -
            m[8] * m[6] * m[13] -
            m[12] * m[5] * m[10] +
            m[12] * m[6] * m[9];

        out[1] = -m[1] * m[10] * m[15] +
            m[1] * m[11] * m[14] +
            m[9] * m[2] * m[15] -
            m[9] * m[3] * m[14] -
            m[13] * m[2] * m[11] +
            m[13] * m[3] * m[10];

        out[5] = m[0] * m[10] * m[15] -
            m[0] * m[11] * m[14] -
            m[8] * m[2] * m[15] +
            m[8] * m[3] * m[14] +
            m[12] * m[2] * m[11] -
            m[12] * m[3] * m[10];

        out[9] = -m[0] * m[9] * m[15] +
            m[0] * m[11] * m[13] +
            m[8] * m[1] * m[15] -
            m[8] * m[3] * m[13] -
            m[12] * m[1] * m[11] +
            m[12] * m[3] * m[9];

        out[13] = m[0] * m[9] * m[14] -
            m[0] * m[10] * m[13] -
            m[8] * m[1] * m[14] +
            m[8] * m[2] * m[13] +
            m[12] * m[1] * m[10] -
            m[12] * m[2] * m[9];

        out[2] = m[1] * m[6] * m[15] -
            m[1] * m[7] * m[14] -
            m[5] * m[2] * m[15] +
            m[5] * m[3] * m[14] +
            m[13] * m[2] * m[7] -
            m[13] * m[3] * m[6];

        out[6] = -m[0] * m[6] * m[15] +
            m[0] * m[7] * m[14] +
            m[4] * m[2] * m[15] -
            m[4] * m[3] * m[14] -
            m[12] * m[2] * m[7] +
            m[12] * m[3] * m[6];

        out[10] = m[0] * m[5] * m[15] -
            m[0] * m[7] * m[13] -
            m[4] * m[1] * m[15] +
            m[4] * m[3] * m[13] +
            m[12] * m[1] * m[7] -
            m[12] * m[3] * m[5];

        out[14] = -m[0] * m[5] * m[14] +
            m[0] * m[6] * m[13] +
            m[4] * m[1] * m[14] -
            m[4] * m[2] * m[13] -
            m[12] * m[1] * m[6] +
            m[12] * m[2] * m[5];

        out[3] = -m[1] * m[6] * m[11] +
            m[1] * m[7] * m[10] +
            m[5] * m[2] * m[11] -
            m[5] * m[3] * m[10] -
            m[9] * m[2] * m[7] +
            m[9] * m[3] * m[6];

        out[7] = m[0] * m[6] * m[11] -
            m[0] * m[7] * m[10] -
            m[4] * m[2] * m[11] +
            m[4] * m[3] * m[10] +
            m[8] * m[2] * m[7] -
            m[8] * m[3] * m[6];

        out[11] = -m[0] * m[5] * m[11] +
            m[0] * m[7] * m[9] +
            m[4] * m[1] * m[11] -
            m[4] * m[3] * m[9] -
            m[8] * m[1] * m[7] +
            m[8] * m[3] * m[5];

        out[15] = m[0] * m[5] * m[10] -
            m[0] * m[6] * m[9] -
            m[4] * m[1] * m[10] +
            m[4] * m[2] * m[9] +
            m[8] * m[1] * m[6] -
            m[8] * m[2] * m[5];

        float det = m[0] * out[0] + m[1] * out[4] + m[2] * out[8] + m[3] * out[12];

        if (det == 0.0f)
            return {}; // or return identity, or set a flag

        float invDet = 1.0f / det;
        for (int i = 0; i < 16; i++)
            out[i] *= invDet;

        return inv;
    }


    CMatrix4x4 operator*(const CMatrix4x4& other) const {
        CMatrix4x4 result = {};

        for (int row = 0; row < 4; ++row)
            for (int col = 0; col < 4; ++col)
                for (int k = 0; k < 4; ++k)
                    result.m[row][col] += m[row][k] * other.m[k][col];

        return result;
    }

    C4Vector operator*(const C4Vector& v) const {
        C4Vector result;
        result.x = v.x * m[0][0] + v.y * m[1][0] + v.z * m[2][0] + v.w * m[3][0];
        result.y = v.x * m[0][1] + v.y * m[1][1] + v.z * m[2][1] + v.w * m[3][1];
        result.z = v.x * m[0][2] + v.y * m[1][2] + v.z * m[2][2] + v.w * m[3][2];
        result.w = v.x * m[0][3] + v.y * m[1][3] + v.z * m[2][3] + v.w * m[3][3];
        return result;
    }


};