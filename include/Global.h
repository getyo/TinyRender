#pragma once
#include "RenderMath.h"
#include "Vertex.h"
#define __DEBUG__
// 基本全局参数
const RenderMath::Vec3D WorldUpVec = RenderMath::Vec3D(0.f,1.f,0.f); 
//阴影相关
constexpr float MinShadowFactor = 0.1;
//长度单位 cm
//摄像机成像参数，所有相机用这一套
constexpr float NearClip = 30.f;
constexpr float FarClip = 10000.0f;
constexpr float SightConeRad = RenderMath::PI / 3.0f; // 60度 FOV
//输出图像大小/屏幕大小
constexpr float ScreenHeight = 1080.0f;
constexpr float ScreenWidth = 1920.0f;
constexpr float Aspect = ScreenWidth / ScreenHeight;
const RenderMath::Mat4D ViewportTransform = RenderMath::Mat4D (
            RenderMath::Vec4D(ScreenWidth / 2.0f, 0.0f, 0.0f, 0.0f),
            RenderMath::Vec4D(0.0f, -ScreenHeight / 2.0f, 0.0f, 0.0f), // 若屏幕反转，这里改为负
            RenderMath::Vec4D(0.0f, 0.0f, 1.0f, 0.0f),
            RenderMath::Vec4D((ScreenWidth - 1.0f) / 2.0f, (ScreenHeight - 1.0f) / 2.0f, 0.0f, 1.0f)
);

const RenderMath::Mat4D ModelTransform = RenderMath::Mat4D (
            RenderMath::Vec4D(0.0f, 0.0f, -1.0f, 0.0f),
            RenderMath::Vec4D(1.0f, 0.0f, 0.0f, 0.0f), 
            RenderMath::Vec4D(0.0f, 1.0f, 0.0f, 0.0f),
            RenderMath::Vec4D(0.0f, 0.0f, 0.0f, 1.0f)
);
struct Camera {
    RenderMath::Vec3D worldPos;
    RenderMath::Vec3D worldRot;

    Camera() : worldPos(0.f, 0.f, 0.f), worldRot(0.f, 0.f, 0.f) {}
    Camera(RenderMath::Vec3D pos, RenderMath::Vec3D rot) 
        : worldPos(pos), worldRot(rot) {}
};

struct PointLight {
    RenderMath::Vec3D worldPos;
    Color color;
    float intensity;

    PointLight() : worldPos(0.f, 0.f, 0.f), color(1.f, 1.f, 1.f), intensity(1.f) {}
    PointLight(RenderMath::Vec3D pos, Color col, float inte) 
        : worldPos(pos), color(col), intensity(inte) {}
};

struct AmbientLight {
    Color color;
    float intensity;
    
    AmbientLight() : color(1.f, 1.f, 1.f), intensity(0.1f) {}
    AmbientLight(Color col, float inte) : color(col), intensity(inte) {}
};

struct WorldObject {
    RenderMath::Vec3D worldPos;
    RenderMath::Vec3D RelatetiveOffset;
    RenderMath::Vec3D scale;
    MeshData meshData;
    Texture texture;
    std::string name;
    bool isDrawn = false;
    inline static float GroundLineThickness = 5.f;
    inline static Color GroundLineColor = Colors::Blue;
    inline static Color GroundBlankColor = Colors::Grey;
    inline static ORM GroundORM = ORM(0.5,1,0);

    WorldObject() 
        : worldPos(0.f, 0.f, 0.f), RelatetiveOffset(0.f, 0.f, 0.f), scale(1.f, 1.f, 1.f) {}
    
    WorldObject(const std::string& name,RenderMath::Vec3D pos, RenderMath::Vec3D offset, RenderMath::Vec3D sca = {1.f, 1.f, 1.f}) 
        :name(name), worldPos(pos), RelatetiveOffset(offset), scale(sca) {}
    static WorldObject MakeGround(int lenght,int width);
    static bool InGroundLine(const RenderMath::Vec3D &fragWorldPos,const RenderMath::Vec3D& v0WorldPos,\
    const RenderMath::Vec3D& v1WorldPos,const RenderMath::Vec3D& v2WorldPos);
};

inline bool InScreen(const RenderMath::Vec2D &pos2D){
    return pos2D.x >= 0 && pos2D.x < ScreenWidth && pos2D.y >= 0 && pos2D.y < ScreenHeight; 
}

inline WorldObject WorldObject::MakeGround(int lenght = 1000,int width = 1000){
    int halfLen = lenght/2, halfWid = width/2;
    int edgeLen = 50;
    WorldObject ground("Ground",{0,0,0},{0,0,0});
    ground.isDrawn = true;

    int cols = width/edgeLen + 1;   // 列数
    int rows = lenght/edgeLen + 1;  // 行数
    int vCnt = rows * cols;
    int triCnt = (rows-1) * (cols-1) * 2;
    
    ground.meshData.vertices.resize(vCnt);
    ground.meshData.triangles.resize(triCnt);
    
    auto &vertices = ground.meshData.vertices;
    auto &triangles = ground.meshData.triangles;
    int xSt = -halfWid, xEnd = halfWid;
    int zSt = -halfLen, zEnd = halfLen;
    
    // 设置顶点位置和法向量
    for(int i = 0; i < rows; i++){
        for(int j = 0; j < cols; j++){
            int vertexIdx = i * cols + j;  // 正确的二维转一维
            float x = xSt + j * edgeLen;
            float z = zSt + i * edgeLen;
            vertices[vertexIdx].normal = WorldUpVec;
            vertices[vertexIdx].pos3D = RenderMath::Vec3D(x, 0.f, z);
        }
    }
    
    // 划分三角形
    int triIdx = 0;
    for(int i = 0; i < rows-1; i++){
        for(int j = 0; j < cols-1; j++){
            int buttomLeft = i * cols + j;
            int buttomRight = buttomLeft + 1;
            int topLeft = (i+1) * cols + j;
            int topRight = topLeft + 1;
            
            // 第一个三角形
            triangles[triIdx].vertexIndex[0] = buttomLeft;
            triangles[triIdx].vertexIndex[1] = topLeft;
            triangles[triIdx].vertexIndex[2] = topRight;
            triIdx++;
            
            // 第二个三角形
            triangles[triIdx].vertexIndex[0] = buttomLeft;
            triangles[triIdx].vertexIndex[1] = topRight;
            triangles[triIdx].vertexIndex[2] = buttomRight;
            triIdx++;
        }
    }
    return ground;
}

// 辅助函数：判断点是否在线段范围内
inline bool OnSegment(float px, float pz, float x1, float z1, float x2, float z2, float halfThickness) {
    // 由于顶点都是整数，线段要么水平要么垂直
    if (x1 == x2) { // 垂直线段
        if (std::abs(px - x1) <= halfThickness) {
            float minZ = std::min(z1, z2);
            float maxZ = std::max(z1, z2);
            return pz >= minZ - halfThickness && pz <= maxZ + halfThickness;
        }
    } else if (z1 == z2) { // 水平线段
        if (std::abs(pz - z1) <= halfThickness) {
            float minX = std::min(x1, x2);
            float maxX = std::max(x1, x2);
            return px >= minX - halfThickness && px <= maxX + halfThickness;
        }
    }
    return false;
}

inline bool WorldObject::InGroundLine(const RenderMath::Vec3D &fragWorldPos,const RenderMath::Vec3D& v0WorldPos,\
    const RenderMath::Vec3D& v1WorldPos,const RenderMath::Vec3D& v2WorldPos)
{
    float halfThickness = GroundLineThickness / 2.0f;
    float fx = fragWorldPos.x;
    float fz = fragWorldPos.z;
    
    // 检查三条边
    // v0 -> v1
    if (OnSegment(fx, fz, v0WorldPos.x, v0WorldPos.z, v1WorldPos.x, v1WorldPos.z, halfThickness))
        return true;
    // v1 -> v2
    if (OnSegment(fx, fz, v1WorldPos.x, v1WorldPos.z, v2WorldPos.x, v2WorldPos.z, halfThickness))
        return true;
    // v2 -> v0
    if (OnSegment(fx, fz, v2WorldPos.x, v2WorldPos.z, v0WorldPos.x, v0WorldPos.z, halfThickness))
        return true;
    
    return false;
}

