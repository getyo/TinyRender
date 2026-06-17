#pragma once
#include "RenderMath.h"
#include <cmath>
#include "Global.h"
#include "Vertex.h"
#include <vector>
#include <memory>

/*
* 坐标系和虎书一致：x轴正向为右，y轴正向为上，z轴正向向内（右手系中看往 -z）
*/
class Projection{
private:
    inline static bool isInit = false;
    // 世界坐标资产位置
    RenderMath::Vec3D cameraPos;
    RenderMath::Vec3D lightPos;
    RenderMath::Vec3D targetPos;

    //变化矩阵，其中ViewTransform需要随摄像机以及跟随对象跟新
    //其他两者在屏幕不变的情况下不变
    RenderMath::Mat4D ViewTransform;
    RenderMath::Mat4D PerspectiveProjection;
    

    Projection(const RenderMath::Vec3D& cameraPos,const RenderMath::Vec3D& lightPost,const RenderMath::Vec3D targetPos):cameraPos(cameraPos), \
    lightPos(lightPos), targetPos(targetPos)
    {
        float tanHalfFov = tanf(SightConeRad / 2.0f);
        PerspectiveProjection = RenderMath::Mat4D(
            RenderMath::Vec4D(1.0f / (Aspect * tanHalfFov), 0.0f, 0.0f, 0.0f),
            RenderMath::Vec4D(0.0f, 1.0f / tanHalfFov, 0.0f, 0.0f),
            // 严格对齐右手系：将 Z 映射到 [-1, 1] 或 [0, 1]
            RenderMath::Vec4D(0.0f, 0.0f, -(FarClip + NearClip) / (FarClip - NearClip), -1.0f),
            RenderMath::Vec4D(0.0f, 0.0f, -(2.0f * FarClip * NearClip) / (FarClip - NearClip), 0.0f)
        );

      
    }

public:
    static std::shared_ptr<Projection> ProjectionFactory(const RenderMath::Vec3D &cameraPos,
    const RenderMath::Vec3D &lightPos,const RenderMath::Vec3D targetPos)
    {
        if(isInit) return nullptr;
        isInit = true;
        return std::shared_ptr<Projection>(new Projection(cameraPos,lightPos,targetPos));
    }

    void SetCameraPos(const RenderMath::Vec3D &newPos){
        cameraPos = newPos;
    }
    void SetLightPos(const RenderMath::Vec3D &newPos){
        lightPos = newPos;
    }

    void Project(WorldObject &worldObj,bool doModelTran = true);
};
