#include "Resterization.h"
#include "Global.h"
#include <cmath>
#include <iostream>
#include <cassert>

void Rasterization::Rasterize(std::vector<Fragment> &fragments, WorldObject &worldObj) {
    //深度预处理
    int screenSize = ScreenWidth * ScreenHeight;
    fragments.resize(screenSize);
#ifdef __DEBUG__
    normalFin.resize(screenSize);
    baseColorFin.resize(screenSize);
    ormFin.resize(screenSize);    
#endif
    auto& vertices = worldObj.meshData.vertices;
    auto& triangles = worldObj.meshData.triangles;
    auto& texture = worldObj.texture;
    for(int i=0; i<ScreenWidth * ScreenHeight; ++i){
        fragments[i].trianglePtr = -1;
        fragments[i].depth = std::numeric_limits<float>::max();
    }
    for(int it = 0;it < triangles.size();++it){
        auto &v0 = vertices[triangles[it].vertexIndex[0]];
        auto &v1 = vertices[triangles[it].vertexIndex[1]];
        auto &v2 = vertices[triangles[it].vertexIndex[2]];
        //暂且这么简单处理近平面剪切
        if(v0.posProj.w <0 || v1.posProj.w < 0 || v2.posProj.w < 0)
            continue;
        RenderMath::Vec4D v0Pos4D = v0.posProj / v0.posProj.w;
        v0Pos4D = ViewportTransform * v0Pos4D; 
        RenderMath::Vec4D v1Pos4D = v1.posProj / v1.posProj.w;
        v1Pos4D = ViewportTransform * v1Pos4D; 
        RenderMath::Vec4D v2Pos4D = v2.posProj / v2.posProj.w;
        v2Pos4D = ViewportTransform * v2Pos4D; 
        RenderMath::Vec2D v0Pos(v0Pos4D.x ,v0Pos4D.y);
        RenderMath::Vec2D v1Pos(v1Pos4D.x ,v1Pos4D.y );
        RenderMath::Vec2D v2Pos(v2Pos4D.x ,v2Pos4D.y );
        //不渲染在屏幕外的点
        if(!InScreen(v0Pos) || !InScreen(v1Pos) || !InScreen(v2Pos))
            continue;
        BoundingBox box = BoundingBox::GetBoundingBox(v0Pos, v1Pos, v2Pos);
        
        for(int i = box.top; i <= box.bottom; ++i){
            for(int j = box.left; j <= box.right; ++j){
                RenderMath::Vec2D fragmentPos(j + 0.5,i + 0.5);
                BarycentricCoord tempBcCoor = BarycentricCoord::GetBarycentricOfP(fragmentPos,v0Pos,v1Pos,v2Pos);
                if (tempBcCoor.a > 0 && tempBcCoor.b > 0 && tempBcCoor.c > 0) 
                {
                   
                    float tempW = tempBcCoor.a * (1.0f / v0.posProj.w) + 
                                 tempBcCoor.b * (1.0f / v1.posProj.w) + 
                                 tempBcCoor.c * (1.0f / v2.posProj.w);
                    float tempDepth = (v0.posProj.z/v0.posProj.w)  * tempBcCoor.a +
                                     (v1.posProj.z/v1.posProj.w)  * tempBcCoor.b +
                                     (v2.posProj.z/v2.posProj.w)  * tempBcCoor.c;
                    auto& curFragment = fragments[i*ScreenWidth + j]; 
                    if(tempDepth <= curFragment.depth){
                        //如果深度较小，更新这个过程算出来的结果，防止下一个循环在插值时重复计算
                        curFragment.depth = tempDepth;
                        curFragment.bcCoor = tempBcCoor;
                        curFragment.w = tempW;
                        curFragment.trianglePtr = it;
                    }  
                }
            }
        }
    }

    int totalCnt = 0;
    //正式对片元属性光栅化插值
    for(int i = 0; i < ScreenHeight;++i){
        for(int j = 0;j < ScreenWidth;++j){
            int it = ScreenWidth * i + j;
            //如果没有关联三角形，说明屏幕不显示，跳过插值
            if(fragments[it].trianglePtr == -1) continue;
            ++totalCnt;
            auto &curFragment = fragments[it];
            auto &v0 = vertices[triangles[curFragment.trianglePtr].vertexIndex[0]];
            auto &v1 = vertices[triangles[curFragment.trianglePtr].vertexIndex[1]];
            auto &v2 = vertices[triangles[curFragment.trianglePtr].vertexIndex[2]];
            //插值纹理坐标到标准空间
            float us = curFragment.bcCoor.a * (v0.textureUV.x / v0.posProj.w) +
                       curFragment.bcCoor.b * (v1.textureUV.x / v1.posProj.w) +
                       curFragment.bcCoor.c * (v2.textureUV.x / v2.posProj.w);     
            us /= curFragment.w;
            float vs = curFragment.bcCoor.a * (v0.textureUV.y / v0.posProj.w) +
                       curFragment.bcCoor.b * (v1.textureUV.y / v1.posProj.w) +
                       curFragment.bcCoor.c * (v2.textureUV.y / v2.posProj.w);     
            vs /= curFragment.w;

            //世界位置坐标插值
            curFragment.worldPos = curFragment.bcCoor.a * (1/v0.posProj.w * v0.pos3D) +
                       curFragment.bcCoor.b * (1/v1.posProj.w * v1.pos3D) +
                       curFragment.bcCoor.c * (1/v2.posProj.w * v2.pos3D);
            curFragment.worldPos = 1/curFragment.w * curFragment.worldPos;  

            //BaseColor插值
            float uBC = us * texture.rgbW;
            float vBC = (1.0f - vs) * texture.rgbH;   // 翻转V
            int sampleU = uBC, sampleV = vBC;
            int leftTop = sampleV * texture.rgbW + sampleU;
            int rightTop = sampleV * texture.rgbW + sampleU + 1;  
            int leftButtom = (sampleV+1) * texture.rgbW + sampleU;
            int rightButtom = (sampleV+1) * texture.rgbW + sampleU + 1;
            curFragment.color = RenderMath::BlinearInterploation(RenderMath::Vec2D(uBC,vBC),
                    texture.rgb[leftTop],texture.rgb[rightTop],texture.rgb[leftButtom],texture.rgb[rightButtom]);

            //法线插值
            uBC = us * texture.normalW,vBC = (1.0f - vs) * texture.normalH;
            sampleU = uBC, sampleV = vBC;
            leftTop = sampleV * texture.normalW + sampleU;
            rightTop = sampleV * texture.normalW + sampleU + 1;  
            leftButtom = (sampleV+1) * texture.normalW + sampleU;
            rightButtom = (sampleV+1) * texture.normalW + sampleU + 1;
            //1.采样法线偏移量
            curFragment.normal = RenderMath::BlinearInterploation(RenderMath::Vec2D(uBC,vBC),
                    texture.normal[leftTop],texture.normal[rightTop],texture.normal[leftButtom],texture.normal[rightButtom]);    
            //2.重心插值TBN
            RenderMath::Vec3D tangent = curFragment.bcCoor.a * (1/v0.posProj.w * v0.tangent) +
                       curFragment.bcCoor.b * (1/v1.posProj.w * v1.tangent) +
                       curFragment.bcCoor.c * (1/v2.posProj.w * v2.tangent);     
            tangent = 1/curFragment.w * tangent;
            RenderMath::Vec3D normal = curFragment.bcCoor.a * (1/v0.posProj.w * v0.normal) +
                       curFragment.bcCoor.b * (1/v1.posProj.w * v1.normal) +
                       curFragment.bcCoor.c * (1/v2.posProj.w * v2.normal); 
            normal = 1/curFragment.w * normal;
            //3.修正三向量，使其重新正交化，单位化
            normal = RenderMath::Normalize(normal);
            tangent = RenderMath::Normalize(tangent - normal * DotProduct(tangent, normal));
            RenderMath::Vec3D biTangent = RenderMath::CrossProduct(normal,tangent);
            RenderMath::Mat3D tbn(tangent,biTangent,normal);
            //4.tbn换系法线偏移
            curFragment.normal = tbn * curFragment.normal; 
            curFragment.normal = RenderMath::Normalize(curFragment.normal);
            
            //环境光遮盖，粗糙度，金属度插值
            uBC = us * texture.ormW,vBC = (1.0f - vs) * texture.ormH;
            sampleU = uBC, sampleV = vBC;
            leftTop = sampleV * texture.ormW + sampleU;
            rightTop = sampleV * texture.ormW + sampleU + 1;  
            leftButtom = (sampleV+1) * texture.ormW + sampleU;
            rightButtom = (sampleV+1) * texture.ormW + sampleU + 1;
            curFragment.orm = RenderMath::BlinearInterploation(RenderMath::Vec2D(uBC,vBC),
                    texture.orm[leftTop],texture.orm[rightTop],texture.orm[leftButtom],texture.orm[rightButtom]);
#ifdef __DEBUG__
            normalFin[it] = curFragment.normal;
            baseColorFin[it] = curFragment.color;
            ormFin[it] =  curFragment.orm;
#endif
        }
    }
#ifdef __DEBUG__
    std::cout << "Total VertexCnt :"<< totalCnt << '\n';
#endif
    return ;
}
