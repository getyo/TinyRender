#include "Resterization.h"
#include "Global.h"
#include <cmath>
#include <iostream>

void Resterization::Rasterize(const std::vector<Vertex> &vertice, const std::vector<Triangle> &triangles, const Texture &texture) {
    //深度预处理
    int screenSize = ScreenWidth * ScreenHeight;
    std::vector<Fragment> fragments(screenSize);
    
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
                   
                    float tempInvW = tempBcCoor.a * (1.0f / v0.posProj.w) + 
                                 tempBcCoor.b * (1.0f / v1.posProj.w) + 
                                 tempBcCoor.c * (1.0f / v2.posProj.w);
                    float tempDepth = (v0.posProj.z/v0.posProj.w)  * tempBcCoor.a +
                                     (v1.posProj.z/v1.posProj.w)  * tempBcCoor.b +
                                     (v2.posProj.z/v2.posProj.w)  * tempBcCoor.c;
                    tempDepth /= tempInvW;
                    auto& curFragment = fragments[i*ScreenWidth + j]; 
                    if(tempDepth <= curFragment.depth){
                        //如果深度较小，更新这个过程算出来的结果，防止下一个循环在插值时重复计算
                        curFragment.depth = tempDepth;
                        curFragment.bcCoor = tempBcCoor;
                        curFragment.invW = tempInvW;
                        curFragment.trianglePtr = it;
                    }  
                }
            }
        }
    }
    for(int i = 0; i < ScreenHeight;++i){
        for(int j = 0;j < ScreenWidth;++j){
            int it = ScreenWidth * i + j;
            if(fragments[it].trianglePtr != -1)
                std::cout <<  i <<","<< j << "  ";
        }
    }
    return ;
}
