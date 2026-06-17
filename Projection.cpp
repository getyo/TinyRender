#include "Projection.h"
#include <cassert>

void Projection::Project(WorldObject &worldObj,bool doModelTran){
    //求解摄像机视角变化
    ViewTransform = RenderMath::LookAt(cameraPos,targetPos,{0,1,0});
    //变化到标准空间的矩阵
    RenderMath::Mat4D M = PerspectiveProjection * ViewTransform;
    auto &vertices = worldObj.meshData.vertices;
    auto &triangles = worldObj.meshData.triangles;
    for(auto &v:vertices){
        
        //投影位置计算
        if(doModelTran){
            auto m = ModelTransform; 
            v.pos3D = m.ToMat3D()* v.pos3D;
        }
        v.pos3D = v.pos3D + worldObj.worldPos + worldObj.RelatetiveOffset;
        RenderMath::Vec4D worldPos(v.pos3D, 1.0f);
        v.posProj = M * worldPos;

        if (v.posProj.w < 0.0f) continue; 

        //为下一步累加做准备
        v.normal = RenderMath::Vec3D(0,0,0);
        v.tangent = RenderMath::Vec3D(0,0,0);
        v.bitangent = RenderMath::Vec3D(0,0,0);
    }
    
    // 阶段 1：遍历三角形，计算面属性并【累加】到三个顶点
    for(auto &triangle: triangles){
        auto &v0 = vertices[triangle.vertexIndex[0]];
        auto &v1 = vertices[triangle.vertexIndex[1]];
        auto &v2 = vertices[triangle.vertexIndex[2]];
        
        RenderMath::Vec3D edge1 = v1.pos3D - v0.pos3D;
        RenderMath::Vec3D edge2 = v2.pos3D - v0.pos3D;
        
        RenderMath::Vec3D faceNormal = RenderMath::CrossProduct(edge1, edge2);
        
        //计算纹理切线
        float du1 = v1.textureUV.x - v0.textureUV.x;
        float du2 = v2.textureUV.x - v0.textureUV.x;
        float dv1 = v1.textureUV.y - v0.textureUV.y;
        float dv2 = v2.textureUV.y - v0.textureUV.y;

        float det = du1 * dv2 - du2 * dv1;
        float invDet = (det == 0.0f) ? 1.0f : (1.0f / det); // 防止除零崩溃

        RenderMath::Vec3D faceT = (dv2 * edge1 - dv1 * edge2) * invDet;
        RenderMath::Vec3D faceB = (du1 * edge2 - du2 * edge1) * invDet;
        
        // 累加到三个顶点上
        v0.normal = v0.normal + faceNormal;
        v1.normal = v1.normal + faceNormal;
        v2.normal = v2.normal + faceNormal;

        v0.tangent = v0.tangent + faceT;
        v1.tangent = v1.tangent + faceT;
        v2.tangent = v2.tangent + faceT;

        v0.bitangent = v0.bitangent + faceB;
        v1.bitangent = v1.bitangent + faceB;
        v2.bitangent = v2.bitangent + faceB;
    }

    // 阶段 2：遍历顶点，进行正交化
    for(auto &v : vertices){
        // 1. 法线单位化 
        v.normal = RenderMath::Normalize(v.normal);
        //Model矩阵镜像后需要反转
        v.normal = v.normal *-1;

        // 2. Tangent 施密特正交化
        v.tangent = RenderMath::Normalize(v.tangent - v.normal * RenderMath::DotProduct(v.normal, v.tangent));

        // 3. 修正B ,因为你默认用的右手系，但是纹理本身是多张平铺在物体表面，所以处于一个物体的对称面
        //纹理的变化方向是反的，当你确定了一个tangent，normal朝外之后，bitangent叉积出来可能是左手系，
        //所以你需要反转
        RenderMath::Vec3D stdB = RenderMath::CrossProduct(v.normal, v.tangent);
        float w = (RenderMath::DotProduct(stdB, v.bitangent) < 0.0f) ? -1.0f : 1.0f;
        v.bitangent = stdB * w; 
    }
}