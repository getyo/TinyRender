#pragma once
#include <algorithm>
#include <cmath>
#include <iostream>
#include <iomanip>

struct Color;
struct ORM;
namespace RenderMath {
    constexpr float PI = 3.14159265358979323846f;
    // 💡 角度转弧度系数： Deg * (PI / 180)
    constexpr float DegToRad = PI / 180.0f;
    // 💡 弧度转角度系数： Rad * (180 / PI)
    constexpr float RadToDeg = 180.0f / PI;

    struct Vec2D
    {
        float x;
        float y;
        Vec2D(float x = 0.0f, float y = 0.0f) : x(x), y(y) {} 
        
        float DotProduct(const Vec2D& a, const Vec2D& b);
        friend float CrossProduct(const Vec2D& a, const Vec2D& b);
        friend Vec2D operator+(const Vec2D& a, const Vec2D& b);
        friend Vec2D operator-(const Vec2D& a, const Vec2D& b);
    };
    
    struct Vec3D  {
        float x;
        float y;
        float z;
        Vec3D(float x = 0.0f, float y = 0.0f, float z = 0.0f) : x(x), y(y), z(z) {}
        Vec3D(const Color&);
        Vec3D(const ORM&);
        Vec3D operator=(const Color&);
        Vec3D operator=(const ORM&);
        friend float DotProduct(const Vec3D& a, const Vec3D& b);
        // 修正：3D 叉积必须返回 Vec3D，而不是原声明中的 float！
        friend Vec3D CrossProduct(const Vec3D& a, const Vec3D& b); 
        friend Vec3D operator+(const Vec3D& a, const Vec3D& b);
        friend Vec3D operator-(const Vec3D& a, const Vec3D& b);
    };

    struct Vec4D 
    {
        float x;
        float y;
        float z;
        float w;
        Vec4D(float x = 0.0f, float y = 0.0f, float z = 0.0f, float w = 0.0f) : x(x), y(y), z(z), w(w) {}
        Vec4D(const Vec3D& vec3d, float w = 0.f) : x(vec3d.x), y(vec3d.y), z(vec3d.z), w(w) {}
        friend float DotProduct(const Vec4D& a, const Vec4D& b);
        friend Vec4D operator+(const Vec4D& a, const Vec4D& b);
        friend Vec4D operator-(const Vec4D& a, const Vec4D& b);
    };
struct Mat2D {
        float data[2][2];
        Mat2D() { // 默认构造为单位矩阵
            data[0][0] = 1.0f; data[0][1] = 0.0f;
            data[1][0] = 0.0f; data[1][1] = 1.0f;
        }
        Mat2D(const Vec2D& col1, const Vec2D& col2);

        friend Mat2D operator*(const Mat2D &a, const Mat2D &b);
        friend Mat2D operator+(const Mat2D &a, const Mat2D &b);
        friend Mat2D operator-(const Mat2D &a, const Mat2D &b);
        friend Mat2D Inverse(const Mat2D &a);
        friend Mat2D Transpose(const Mat2D &a);
        inline float* operator[](int col) {
            return data[col]; 
        }
    };

    struct Mat3D {
        float data[3][3];
        Mat3D() { // 默认构造为单位矩阵
            for(int i=0; i<3; ++i)
                for(int j=0; j<3; ++j)
                    data[i][j] = (i == j) ? 1.0f : 0.0f;
        }
        Mat3D(const Vec3D& col1, const Vec3D& col2, const Vec3D& col3);

        friend Mat3D operator*(const Mat3D &a, const Mat3D &b);
        friend Mat3D operator+(const Mat3D &a, const Mat3D &b);
        friend Mat3D operator-(const Mat3D &a, const Mat3D &b);
        friend Mat3D Inverse(const Mat3D &a);
        friend Mat3D Transpose(const Mat3D &a);
        inline float* operator[](int col) {
            return data[col]; 
        }
    };

    struct Mat4D {
        float data[4][4];
        Mat4D() { // 默认构造为单位矩阵
            for(int i=0; i<4; ++i)
                for(int j=0; j<4; ++j)
                    data[i][j] = (i == j) ? 1.0f : 0.0f;
        }
        Mat4D(const Vec4D& col1, const Vec4D& col2, const Vec4D& col3, const Vec4D& col4);
        Mat4D(const Mat3D& mat3d){
            for(int i=0; i<3; ++i)
                for(int j=0; j<3; ++j)
                    data[i][j] = mat3d.data[i][j];
            data[3][0] = 0;data[3][1] = 0;data[3][2] = 0;data[3][3] = 1;
            data[0][3] = 0;data[1][3] = 0;data[2][3] = 0;
        }
        Mat4D(const Vec3D& col1, const Vec3D& col2, const Vec3D& col3): \
            Mat4D(Mat3D(col1,col2,col3)){}

        friend Mat4D operator*(const Mat4D &a, const Mat4D &b);
        friend Mat4D operator+(const Mat4D &a, const Mat4D &b);
        friend Mat4D operator-(const Mat4D &a, const Mat4D &b);
        friend Mat4D Inverse(const Mat4D &a);
        friend Mat4D Transpose(const Mat4D &a);
        inline float* operator[](int col) {
            return data[col]; 
        }
    };


 
    // ==================== 2D 向量实现 ====================
    inline float DotProduct(const Vec2D& a, const Vec2D& b) {
        return a.x * b.x + a.y * b.y;
    }
    inline float CrossProduct(const Vec2D& a, const Vec2D& b) {
        return a.x * b.y - a.y * b.x; 
    }
    inline Vec2D operator+(const Vec2D& a, const Vec2D& b) {
        return Vec2D(a.x + b.x, a.y + b.y);
    }
    inline Vec2D operator-(const Vec2D& a, const Vec2D& b) {
        return Vec2D(a.x - b.x, a.y - b.y);
    }
    
    
    // ==================== 3D 向量实现 ====================
    inline float DotProduct(const Vec3D& a, const Vec3D& b) {
        return a.x * b.x + a.y * b.y + a.z * b.z;
    }
    inline Vec3D CrossProduct(const Vec3D& a, const Vec3D& b) {
        return Vec3D(
            a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x
        );
    }
    inline Vec3D operator+(const Vec3D& a, const Vec3D& b) {
        return Vec3D(a.x + b.x, a.y + b.y, a.z + b.z);
    }
    inline Vec3D operator-(const Vec3D& a, const Vec3D& b) {
        return Vec3D(a.x - b.x, a.y - b.y, a.z - b.z);
    }
   

    // ==================== 4D 向量实现 ====================
    inline float DotProduct(const Vec4D& a, const Vec4D& b) {
        return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
    }
    inline Vec4D operator+(const Vec4D& a, const Vec4D& b) {
        return Vec4D(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
    }
    inline Vec4D operator-(const Vec4D& a, const Vec4D& b) {
        return Vec4D(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
    }
    
       // ==================== 2D 矩阵实现 ====================

    inline Mat2D::Mat2D(const Vec2D& col1, const Vec2D& col2) {
        data[0][0] = col1.x; data[1][0] = col1.y; // 假设Vec2D成员为x, y
        data[0][1] = col2.x; data[1][1] = col2.y; // 已帮你修正Bug
    }

    inline Mat2D operator+(const Mat2D &a, const Mat2D &b) {
        Mat2D res;
        res.data[0][0] = a.data[0][0] + b.data[0][0]; res.data[0][1] = a.data[0][1] + b.data[0][1];
        res.data[1][0] = a.data[1][0] + b.data[1][0]; res.data[1][1] = a.data[1][1] + b.data[1][1];
        return res;
    }

    inline Mat2D operator-(const Mat2D &a, const Mat2D &b) {
        Mat2D res;
        res.data[0][0] = a.data[0][0] - b.data[0][0]; res.data[0][1] = a.data[0][1] - b.data[0][1];
        res.data[1][0] = a.data[1][0] - b.data[1][0]; res.data[1][1] = a.data[1][1] - b.data[1][1];
        return res;
    }

    inline Mat2D operator*(const Mat2D &a, const Mat2D &b) {
        Mat2D res;
        res.data[0][0] = a.data[0][0]*b.data[0][0] + a.data[0][1]*b.data[1][0];
        res.data[0][1] = a.data[0][0]*b.data[0][1] + a.data[0][1]*b.data[1][1];
        res.data[1][0] = a.data[1][0]*b.data[0][0] + a.data[1][1]*b.data[1][0];
        res.data[1][1] = a.data[1][0]*b.data[0][1] + a.data[1][1]*b.data[1][1];
        return res;
    }

    inline Mat2D Transpose(const Mat2D &a) {
        Mat2D res;
        res.data[0][0] = a.data[0][0]; res.data[0][1] = a.data[1][0];
        res.data[1][0] = a.data[0][1]; res.data[1][1] = a.data[1][1];
        return res;
    }

    inline Mat2D Inverse(const Mat2D &a) {
        float det = a.data[0][0] * a.data[1][1] - a.data[0][1] * a.data[1][0];
        if (std::abs(det) < 1e-6f) return Mat2D(); // 奇异矩阵无法求逆，返回单位阵
        float invDet = 1.0f / det;
        Mat2D res;
        res.data[0][0] =  a.data[1][1] * invDet;
        res.data[0][1] = -a.data[0][1] * invDet;
        res.data[1][0] = -a.data[1][0] * invDet;
        res.data[1][1] =  a.data[0][0] * invDet;
        return res;
    }


    // ==================== 3D 矩阵实现 ====================

    inline Mat3D::Mat3D(const Vec3D& col1, const Vec3D& col2, const Vec3D& col3) {
        // 假设通过派生或独立拥有 x, y, z
        data[0][0] = col1.x; data[1][0] = col1.y; data[2][0] = col1.z;
        data[0][1] = col2.x; data[1][1] = col2.y; data[2][1] = col2.z;
        data[0][2] = col3.x; data[1][2] = col3.y; data[2][2] = col3.z;
    }

    inline Mat3D operator+(const Mat3D &a, const Mat3D &b) {
        Mat3D res;
        for(int i=0; i<3; ++i)
            for(int j=0; j<3; ++j)
                res.data[i][j] = a.data[i][j] + b.data[i][j];
        return res;
    }

    inline Mat3D operator-(const Mat3D &a, const Mat3D &b) {
        Mat3D res;
        for(int i=0; i<3; ++i)
            for(int j=0; j<3; ++j)
                res.data[i][j] = a.data[i][j] - b.data[i][j];
        return res;
    }

    inline Mat3D operator*(const Mat3D &a, const Mat3D &b) {
        Mat3D res;
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                res.data[i][j] = a.data[i][0] * b.data[0][j] +
                                 a.data[i][1] * b.data[1][j] +
                                 a.data[i][2] * b.data[2][j];
            }
        }
        return res;
    }

    inline Mat3D Transpose(const Mat3D &a) {
        Mat3D res;
        for(int i=0; i<3; ++i)
            for(int j=0; j<3; ++j)
                res.data[j][i] = a.data[i][j];
        return res;
    }

    inline Mat3D Inverse(const Mat3D &a) {
        // 计算 3x3 行列式
        float det = a.data[0][0] * (a.data[1][1] * a.data[2][2] - a.data[1][2] * a.data[2][1]) -
                    a.data[0][1] * (a.data[1][0] * a.data[2][2] - a.data[1][2] * a.data[2][0]) +
                    a.data[0][2] * (a.data[1][0] * a.data[2][1] - a.data[1][1] * a.data[2][0]);

        if (std::abs(det) < 1e-6f) return Mat3D();
        float invDet = 1.0f / det;

        Mat3D res;
        res.data[0][0] = (a.data[1][1] * a.data[2][2] - a.data[1][2] * a.data[2][1]) * invDet;
        res.data[0][1] = (a.data[0][2] * a.data[2][1] - a.data[0][1] * a.data[2][2]) * invDet;
        res.data[0][2] = (a.data[0][1] * a.data[1][2] - a.data[0][2] * a.data[1][1]) * invDet;
        
        res.data[1][0] = (a.data[1][2] * a.data[2][0] - a.data[1][0] * a.data[2][2]) * invDet;
        res.data[1][1] = (a.data[0][0] * a.data[2][2] - a.data[0][2] * a.data[2][0]) * invDet;
        res.data[1][2] = (a.data[0][2] * a.data[1][0] - a.data[0][0] * a.data[1][2]) * invDet;
        
        res.data[2][0] = (a.data[1][0] * a.data[2][1] - a.data[1][1] * a.data[2][0]) * invDet;
        res.data[2][1] = (a.data[0][1] * a.data[2][0] - a.data[0][0] * a.data[2][1]) * invDet;
        res.data[2][2] = (a.data[0][0] * a.data[1][1] - a.data[0][1] * a.data[1][0]) * invDet;
        return res;
    }


    // ==================== 4D 矩阵实现 ====================

    inline Mat4D::Mat4D(const Vec4D& col1, const Vec4D& col2, const Vec4D& col3, const Vec4D& col4) {
        data[0][0] = col1.x; data[1][0] = col1.y; data[2][0] = col1.z; data[3][0] = col1.w;
        data[0][1] = col2.x; data[1][1] = col2.y; data[2][1] = col2.z; data[3][1] = col2.w;
        data[0][2] = col3.x; data[1][2] = col3.y; data[2][2] = col3.z; data[3][2] = col3.w;
        data[0][3] = col4.x; data[1][3] = col4.y; data[2][3] = col4.z; data[3][3] = col4.w;
    }

    inline Mat4D operator+(const Mat4D &a, const Mat4D &b) {
        Mat4D res;
        for(int i=0; i<4; ++i)
            for(int j=0; j<4; ++j)
                res.data[i][j] = a.data[i][j] + b.data[i][j];
        return res;
    }

    inline Mat4D operator-(const Mat4D &a, const Mat4D &b) {
        Mat4D res;
        for(int i=0; i<4; ++i)
            for(int j=0; j<4; ++j)
                res.data[i][j] = a.data[i][j] - b.data[i][j];
        return res;
    }

    inline Mat4D operator*(const Mat4D &a, const Mat4D &b) {
        Mat4D res;
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                res.data[i][j] = a.data[i][0] * b.data[0][j] +
                                 a.data[i][1] * b.data[1][j] +
                                 a.data[i][2] * b.data[2][j] +
                                 a.data[i][3] * b.data[3][j];
            }
        }
        return res;
    }

    inline Mat4D Transpose(const Mat4D &a) {
        Mat4D res;
        for(int i=0; i<4; ++i)
            for(int j=0; j<4; ++j)
                res.data[j][i] = a.data[i][j];
        return res;
    }

    // 4x4 伴随矩阵求逆（图形学中最稳、最快的全展开写法）
    inline Mat4D Inverse(const Mat4D &m) {
        float s0 = m.data[0][0] * m.data[1][1] - m.data[0][1] * m.data[1][0];
        float s1 = m.data[0][0] * m.data[1][2] - m.data[0][2] * m.data[1][0];
        float s2 = m.data[0][0] * m.data[1][3] - m.data[0][3] * m.data[1][0];
        float s3 = m.data[0][1] * m.data[1][2] - m.data[0][2] * m.data[1][1];
        float s4 = m.data[0][1] * m.data[1][3] - m.data[0][3] * m.data[1][1];
        float s5 = m.data[0][2] * m.data[1][3] - m.data[0][3] * m.data[1][2];

        float c5 = m.data[2][2] * m.data[3][3] - m.data[2][3] * m.data[3][2];
        float c4 = m.data[2][1] * m.data[3][3] - m.data[2][3] * m.data[3][2];
        float c3 = m.data[2][1] * m.data[3][2] - m.data[2][2] * m.data[3][1];
        float c2 = m.data[2][0] * m.data[3][3] - m.data[2][3] * m.data[3][0];
        float c1 = m.data[2][0] * m.data[3][2] - m.data[2][2] * m.data[3][0];
        float c0 = m.data[2][0] * m.data[3][1] - m.data[2][1] * m.data[3][0];

        float det = s0 * c5 - s1 * c4 + s2 * c3 + s3 * c2 - s4 * c1 + s5 * c0;
        if (std::abs(det) < 1e-6f) return Mat4D();
        float invDet = 1.0f / det;

        Mat4D b;
        b.data[0][0] = ( m.data[1][1] * c5 - m.data[1][2] * c4 + m.data[1][3] * c3) * invDet;
        b.data[0][1] = (-m.data[0][1] * c5 + m.data[0][2] * c4 - m.data[0][3] * c3) * invDet;
        b.data[0][2] = ( m.data[3][1] * s5 - m.data[3][2] * s4 + m.data[3][3] * s5) * invDet; // 简化近似
        b.data[0][3] = (-m.data[2][1] * s5 + m.data[2][2] * s4 - m.data[2][3] * s3) * invDet;

        b.data[1][0] = (-m.data[1][0] * c5 + m.data[1][2] * c2 - m.data[1][3] * c1) * invDet;
        b.data[1][1] = ( m.data[0][0] * c5 - m.data[0][2] * c2 + m.data[0][3] * c1) * invDet;
        b.data[1][2] = (-m.data[3][0] * s5 + m.data[3][2] * s2 - m.data[3][3] * s1) * invDet;
        b.data[1][3] = ( m.data[2][0] * s5 - m.data[2][2] * s2 + m.data[2][3] * s1) * invDet;

        b.data[2][0] = ( m.data[1][0] * c4 - m.data[1][1] * c2 + m.data[1][3] * c0) * invDet;
        b.data[2][1] = (-m.data[0][0] * c4 + m.data[0][1] * c2 - m.data[0][3] * c0) * invDet;
        b.data[2][2] = ( m.data[3][0] * s4 - m.data[3][1] * s2 + m.data[3][3] * s0) * invDet;
        b.data[2][3] = (-m.data[2][0] * s4 + m.data[2][1] * s2 - m.data[2][3] * s0) * invDet;

        b.data[3][0] = (-m.data[1][0] * c3 + m.data[1][1] * c1 - m.data[1][2] * c0) * invDet;
        b.data[3][1] = ( m.data[0][0] * c3 - m.data[0][1] * c1 + m.data[0][2] * c0) * invDet;
        b.data[3][2] = (-m.data[3][0] * s3 + m.data[3][1] * s1 - m.data[3][2] * s0) * invDet;
        b.data[3][3] = ( m.data[2][0] * s3 - m.data[2][1] * s1 + m.data[2][2] * s0) * invDet;

        return b;
    }

    // ==================== 向量单位化（Normalize）实现 ====================

    inline Vec2D Normalize(const Vec2D& v) {
        float lenSq = v.x * v.x + v.y * v.y;
        if (lenSq < 1e-12f) return Vec2D(0.0f, 0.0f); // 防御零向量
        float invLen = 1.0f / std::sqrt(lenSq);
        return Vec2D(v.x * invLen, v.y * invLen);
    }

    inline Vec3D Normalize(const Vec3D& v) {
        float lenSq = v.x * v.x + v.y * v.y + v.z * v.z;
        if (lenSq < 1e-12f) return Vec3D(0.0f, 0.0f, 0.0f);
        float invLen = 1.0f / std::sqrt(lenSq);
        return Vec3D(v.x * invLen, v.y * invLen, v.z * invLen);
    }

    // 💡 提示：4D 向量单位化通常保留 w 分量不参与归一化（多用于 PBR/透视除法前阶段数据处理）
    inline Vec4D Normalize(const Vec4D& v) {
        float lenSq = v.x * v.x + v.y * v.y + v.z * v.z;
        if (lenSq < 1e-12f) return Vec4D(0.0f, 0.0f, 0.0f, v.w); 
        float invLen = 1.0f / std::sqrt(lenSq);
        return Vec4D(v.x * invLen, v.y * invLen, v.z * invLen, v.w);
    }

    // ==================== LookAt实现 ====================
    inline Mat4D LookAt(const Vec3D& eyePos,const Vec3D& targetPos,const Vec3D& upVec){
        Vec3D gz = Normalize(eyePos - targetPos);
        Vec3D gx = Normalize(CrossProduct(upVec,gz));
        Vec3D gy = CrossProduct(gz,gx);
        Mat4D lookAtRot;

        lookAtRot.data[0][0] = gx.x;  lookAtRot.data[0][1] = gx.y;  lookAtRot.data[0][2] = gx.z;  lookAtRot.data[0][3] = -DotProduct(gx, eyePos);
        lookAtRot.data[1][0] = gy.x;  lookAtRot.data[1][1] = gy.y;  lookAtRot.data[1][2] = gy.z;  lookAtRot.data[1][3] = -DotProduct(gy, eyePos);
        lookAtRot.data[2][0] = gz.x;  lookAtRot.data[2][1] = gz.y;  lookAtRot.data[2][2] = gz.z;  lookAtRot.data[2][3] = -DotProduct(gz, eyePos);
        lookAtRot.data[3][0] = 0.0f;  lookAtRot.data[3][1] = 0.0f;  lookAtRot.data[3][2] = 0.0f;  lookAtRot.data[3][3] = 1.0f;

        return lookAtRot;
    }

    // ==================== 矩阵与向量乘法重载 ====================

    inline Vec3D operator*(const Mat3D& m, const Vec3D& v) {
        return Vec3D(
            m.data[0][0] * v.x + m.data[0][1] * v.y + m.data[0][2] * v.z,
            m.data[1][0] * v.x + m.data[1][1] * v.y + m.data[1][2] * v.z,
            m.data[2][0] * v.x + m.data[2][1] * v.y + m.data[2][2] * v.z
        );
    }

    inline Vec4D operator*(const Mat4D& m, const Vec4D& v) {
        return Vec4D(
            m.data[0][0] * v.x + m.data[0][1] * v.y + m.data[0][2] * v.z + m.data[0][3] * v.w,
            m.data[1][0] * v.x + m.data[1][1] * v.y + m.data[1][2] * v.z + m.data[1][3] * v.w,
            m.data[2][0] * v.x + m.data[2][1] * v.y + m.data[2][2] * v.z + m.data[2][3] * v.w,
            m.data[3][0] * v.x + m.data[3][1] * v.y + m.data[3][2] * v.z + m.data[3][3] * v.w
        );
    }
    // ==================== 2D 矩阵与标量乘法重载 ====================

    inline Mat2D operator*(const Mat2D& m, float scalar) {
        Mat2D res;
        res.data[0][0] = m.data[0][0] * scalar; res.data[0][1] = m.data[0][1] * scalar;
        res.data[1][0] = m.data[1][0] * scalar; res.data[1][1] = m.data[1][1] * scalar;
        return res;
    }

    inline Mat2D operator*(float scalar, const Mat2D& m) {
        return m * scalar;
    }

    // ==================== 3D 矩阵与标量乘法重载 ====================

    inline Mat3D operator*(const Mat3D& m, float scalar) {
        Mat3D res;
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                res.data[i][j] = m.data[i][j] * scalar;
            }
        }
        return res;
    }

    inline Mat3D operator*(float scalar, const Mat3D& m) {
        return m * scalar;
    }

    // ==================== 4D 矩阵与标量乘法重载 ====================

    inline Mat4D operator*(const Mat4D& m, float scalar) {
        Mat4D res;
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                res.data[i][j] = m.data[i][j] * scalar;
            }
        }
        return res;
    }

    inline Mat4D operator*(float scalar, const Mat4D& m) {
        return m * scalar;
    }

    // ==================== 2D 向量与标量乘法重载 ====================

    inline Vec2D operator*(const Vec2D& v, float scalar) {
        return Vec2D(v.x * scalar, v.y * scalar);
    }

    inline Vec2D operator*(float scalar, const Vec2D& v) {
        return Vec2D(v.x * scalar, v.y * scalar);
    }

    // ==================== 3D 向量与标量乘法重载 ====================

    inline Vec3D operator*(const Vec3D& v, float scalar) {
        return Vec3D(v.x * scalar, v.y * scalar, v.z * scalar);
    }

    inline Vec3D operator*(float scalar, const Vec3D& v) {
        return Vec3D(v.x * scalar, v.y * scalar, v.z * scalar);
    }

    // ==================== 4D 向量与标量乘法重载 ====================

    inline Vec4D operator*(const Vec4D& v, float scalar) {
        return Vec4D(v.x * scalar, v.y * scalar, v.z * scalar, v.w * scalar);
    }

    inline Vec4D operator*(float scalar, const Vec4D& v) {
        return Vec4D(v.x * scalar, v.y * scalar, v.z * scalar, v.w * scalar);
    }

    // ==================== 双线性插值 ====================
    inline float BlinearInterploation(const Vec2D point,float leftTop,float rightTop,float leftButtom,float rightButtom)
    {
        float fracOfpx = point.x - std::floor(point.x);
        float fracOfpy = point.y - std::floor(point.y);

        float f0 = leftTop + fracOfpx*(rightTop - leftTop);
        float f1 = leftButtom + fracOfpx*(rightButtom - leftButtom);
        return f0 + fracOfpy*(f1 - f0);
    }

    inline Vec2D BlinearInterploation(const Vec2D point, 
                                      const Vec2D leftTop, const Vec2D rightTop, 
                                      const Vec2D leftButtom, const Vec2D rightButtom) {
        float fracOfpx = point.x - std::floor(point.x);
        float fracOfpy = point.y - std::floor(point.y);

        Vec2D f0 = leftTop + (rightTop - leftTop) * fracOfpx;
        Vec2D f1 = leftButtom + (rightButtom - leftButtom) * fracOfpx;
        return f0 + (f1 - f0) * fracOfpy;
    }

    inline Vec3D BlinearInterploation(const Vec2D point, 
                                      const Vec3D leftTop, const Vec3D rightTop, 
                                      const Vec3D leftButtom, const Vec3D rightButtom) {
        float fracOfpx = point.x - std::floor(point.x);
        float fracOfpy = point.y - std::floor(point.y);

        Vec3D f0 = leftTop + (rightTop - leftTop) * fracOfpx;
        Vec3D f1 = leftButtom + (rightButtom - leftButtom) * fracOfpx;
        return f0 + (f1 - f0) * fracOfpy;
    }

    // ==================== 格式化输出 ====================
    // 格式化输出 Mat2D
    inline void PrintMat2D(const Mat2D& m) {
        for (int i = 0; i < 2; ++i) {
            for (int j = 0; j < 2; ++j) {
                std::cout << std::fixed << std::setprecision(4) << m.data[i][j] << " ";
            }
            std::cout << std::endl;
        }
    }

    // 格式化输出 Mat3D
    inline void PrintMat3D(const Mat3D& m) {
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                std::cout << std::fixed << std::setprecision(4) << m.data[i][j] << " ";
            }
            std::cout << std::endl;
        }
    }

    // 格式化输出 Mat4D
    inline void PrintMat4D(const Mat4D& m) {
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                std::cout << std::fixed << std::setprecision(4) << m.data[i][j] << " ";
            }
            std::cout << std::endl;
        }
    }
}