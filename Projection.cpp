#include "Projection.h"

void Projection::Project(std::vector<Vertex> &vertice){
    //求解摄像机视角变化
    //把相机平移到原点
    RenderMath::Mat4D T(
        RenderMath::Vec4D(1,0,0,0),
        RenderMath::Vec4D(0,1,0,0),
        RenderMath::Vec4D(0,0,1,0),
        RenderMath::Vec4D(-cameraPos.x,-cameraPos.y,-cameraPos.z,1)
    );
    ViewTransform = RenderMath::LookAt(cameraPos,{0,0,-1},{0,1,0}) * T;
    //变化到标准空间的矩阵
    RenderMath::Mat4D M = PerspectiveProjection * ViewTransform;
    for(auto &v:vertice){
        
        RenderMath::Vec4D worldPos(v.pos3D.x + objPos.x, v.pos3D.y + objPos.y, v.pos3D.z + objPos.z, 1.0f);

        v.posProj = M * worldPos;
    
        if (v.posProj.w <= 0.0f) continue; 

        v.posProj = (1.f / v.posProj.w) * v.posProj;
    }
}