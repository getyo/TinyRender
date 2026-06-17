#pragma once
#include "RenderMath.h"
#include "Vertex.h"
#define __DEBUG__
// 基本全局参数
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
            RenderMath::Vec4D(0.0f, ScreenHeight / 2.0f, 0.0f, 0.0f), // 若屏幕反转，这里改为负
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

    WorldObject() 
        : worldPos(0.f, 0.f, 0.f), RelatetiveOffset(0.f, 0.f, 0.f), scale(1.f, 1.f, 1.f) {}
    
    WorldObject(RenderMath::Vec3D pos, RenderMath::Vec3D offset, RenderMath::Vec3D sca = {1.f, 1.f, 1.f}) 
        : worldPos(pos), RelatetiveOffset(offset), scale(sca) {}
};

inline bool InScreen(const RenderMath::Vec2D &pos2D){
    return pos2D.x >= 0 && pos2D.x < ScreenWidth && pos2D.y >= 0 && pos2D.y < ScreenHeight; 
}
