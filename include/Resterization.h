#pragma once
#include "Vertex.h"
#include "RenderMath.h"
#include <memory>

struct BarycentricCoord
{
    float a,b,c;
    static BarycentricCoord GetBarycentricOfP(const RenderMath::Vec2D &p,const RenderMath::Vec2D &triangleV0,\
    const RenderMath::Vec2D &triangleV1,const RenderMath::Vec2D &triangleV2){
        BarycentricCoord bcCoor;
        float triangleArea = abs(RenderMath::CrossProduct(triangleV2 - triangleV0,triangleV1 - triangleV0));
        triangleArea = triangleArea < 1e-6 ? 1:triangleArea;
        bcCoor.a = RenderMath::CrossProduct(p-triangleV0 , p-triangleV2) /triangleArea; 
        bcCoor.b = RenderMath::CrossProduct(p-triangleV0 , p-triangleV2) /triangleArea; 
        bcCoor.c = 1 - bcCoor.a - bcCoor.b;
        return bcCoor;
    }
};
struct Fragment{
    RenderMath::Vec2D standardUV;
    RenderMath::Vec3D normal;
    Color color;
    ORM orm;
    int trianglePtr;
    float depth;
    float invW;
    BarycentricCoord bcCoor;
    
};
struct BoundingBox{
    int left;
    int right;
    int top;
    int bottom;
    static BoundingBox GetBoundingBox(const Vertex& v0,const Vertex& v1,const Vertex& v2){
        BoundingBox box;
        box.left = std::min({v0.posProj.x/v0.posProj.w,v1.posProj.x/v0.posProj.w,v2.posProj.x/v0.posProj.w});
        box.right = std::max({v0.posProj.x/v0.posProj.w,v1.posProj.x/v0.posProj.w,v2.posProj.x/v0.posProj.w});
        box.top = std::min({v0.posProj.y/v0.posProj.w,v1.posProj.y/v0.posProj.w,v2.posProj.y/v0.posProj.w});
        box.bottom = std::max({v0.posProj.y/v0.posProj.w,v1.posProj.y/v0.posProj.w,v2.posProj.y/v0.posProj.w});
        return box;
    }
};




class Resterization{
private:
    inline static bool isInit = false;
    Resterization() = default;
public:
    static std::shared_ptr<Resterization> RasterizationFactory(){
        if(isInit) return nullptr;
        isInit = true;
        return std::shared_ptr<Resterization>(new Resterization());
    }
    void Rasterize(const std::vector<Vertex> &vertice,const std::vector<Triangle>& triangles,
        const Texture& texture);

};