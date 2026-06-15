#include "Resterization.h"
#include "Global.h"
#include <cmath>
#include <iostream>

void Resterization::Rasterize(std::vector<Fragment> &fragments, std::vector<Vertex> &vertice, const std::vector<Triangle> &triangles, const Texture &texture) {
    //深度预处理
    int screenSize = ScreenWidth * ScreenHeight;
    fragments.resize(screenSize);
    
    for(int i=0; i<ScreenWidth * ScreenHeight; ++i){
        fragments[i].trianglePtr = -1;
        fragments[i].depth = std::numeric_limits<float>::max();
    }
    for(int it = 0;it < triangles.size();++it){
        auto &v0 = vertice[triangles[it].vertexIndex[0]];
        auto &v1 = vertice[triangles[it].vertexIndex[1]];
        auto &v2 = vertice[triangles[it].vertexIndex[2]];
        RenderMath::Vec2D v0Pos(v0.posProj.x / v0.posProj.w,v0.posProj.y / v0.posProj.w);
        RenderMath::Vec2D v1Pos(v1.posProj.x / v1.posProj.w,v1.posProj.y / v1.posProj.w);
        RenderMath::Vec2D v2Pos(v2.posProj.x / v2.posProj.w,v2.posProj.y / v2.posProj.w);
        //这个BoundingBox输入的参数不能除以w，函数内部还会再次除w,顶点内部属性一律不除w，除非copy到片元
        BoundingBox box = BoundingBox::GetBoundingBox(v0, v1, v2);
        for(int i = box.top; i < box.bottom; ++i){
            for(int j = box.left; j < box.right; ++j){
                RenderMath::Vec2D fragmentPos(i,j);
                BarycentricCoord tempBcCoor = BarycentricCoord::GetBarycentricOfP(fragmentPos,v0Pos,v1Pos,v2Pos);
                if(tempBcCoor.a > 0 && tempBcCoor.b > 0 &&
                   tempBcCoor.c > 0 )
                {
                   
                    float tempW = tempBcCoor.a * (1.0f / v0.posProj.w) + 
                                 tempBcCoor.b * (1.0f / v1.posProj.w) + 
                                 tempBcCoor.c * (1.0f / v2.posProj.w);
                    float tempDepth = (v0.posProj.z/v0.posProj.w)  * tempBcCoor.a +
                                     (v1.posProj.z/v1.posProj.w)  * tempBcCoor.b +
                                     (v2.posProj.z/v2.posProj.w)  * tempBcCoor.c;
                    tempDepth /= tempW;
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

    //正式对片元属性光栅化插值
    for(int i = 0; i < ScreenHeight;++i){
        for(int j = 0;j < ScreenWidth;++j){
            int it = ScreenWidth * i + j;
            //如果没有关联三角形，说明屏幕不显示，跳过插值
            if(fragments[it].trianglePtr == -1) continue;
            auto &curFragment = fragments[it];
            auto &v0 = vertice[triangles[curFragment.trianglePtr].vertexIndex[0]];
            auto &v1 = vertice[triangles[curFragment.trianglePtr].vertexIndex[1]];
            auto &v2 = vertice[triangles[curFragment.trianglePtr].vertexIndex[2]];
            //插值纹理坐标到标准空间
            float us = curFragment.bcCoor.a * (v0.textureUV.x / v0.posProj.w) +
                       curFragment.bcCoor.b * (v1.textureUV.x / v1.posProj.w) +
                       curFragment.bcCoor.c * (v2.textureUV.x / v2.posProj.w);     
            us /= curFragment.w;
            float vs = curFragment.bcCoor.a * (v0.textureUV.y / v0.posProj.w) +
                       curFragment.bcCoor.b * (v1.textureUV.y / v1.posProj.w) +
                       curFragment.bcCoor.c * (v2.textureUV.y / v2.posProj.w);     
            vs /= curFragment.w;

            //BaseColor插值
            float uBC = us * texture.rgbW,vBC = vs * texture.normalH;
            int sampleU = uBC, sampleV = vBC;
            int leftTop = sampleV * texture.rgbW + sampleU;
            int rightTop = sampleV * texture.rgbW + sampleU + 1;  
            int leftButtom = (sampleV+1) * texture.rgbW + sampleU;
            int rightButtom = sampleV * texture.rgbW + sampleU + 1;
            curFragment.color = RenderMath::BlinearInterploation(RenderMath::Vec2D(uBC,vBC),
                    texture.rgb[leftTop],texture.rgb[rightTop],texture.rgb[leftButtom],texture.rgb[rightButtom]);

            //法线插值
            uBC = us * texture.normalW,vBC = vs * texture.normalH;
            sampleU = uBC, sampleV = vBC;
            leftTop = sampleV * texture.rgbW + sampleU;
            rightTop = sampleV * texture.rgbW + sampleU + 1;  
            leftButtom = (sampleV+1) * texture.rgbW + sampleU;
            rightButtom = sampleV * texture.rgbW + sampleU + 1;
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
            // 理论上 TBN 是正交矩阵，在此不需要归一化。
            // 若发现光照异常，可取消下方注释以消除浮点累计误差。
            // curFragment.normal = RenderMath::Normalize(curFragment.normal);
            
            //环境光遮盖，粗糙度，金属度插值
            uBC = us * texture.ormW,vBC = vs * texture.ormH;
            sampleU = uBC, sampleV = vBC;
            leftTop = sampleV * texture.rgbW + sampleU;
            rightTop = sampleV * texture.rgbW + sampleU + 1;  
            leftButtom = (sampleV+1) * texture.rgbW + sampleU;
            rightButtom = sampleV * texture.rgbW + sampleU + 1;
            curFragment.orm = RenderMath::BlinearInterploation(RenderMath::Vec2D(uBC,vBC),
                    texture.orm[leftTop],texture.orm[rightTop],texture.orm[leftButtom],texture.orm[rightButtom]);
        }
    }
    return ;
}
