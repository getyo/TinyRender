#pragma once
#include "Vertex.h"
#include "RenderMath.h"
#include <memory>
#include "Global.h"

struct BarycentricCoord
{
    float a,b,c;
    static BarycentricCoord GetBarycentricOfP(const RenderMath::Vec2D &p,const RenderMath::Vec2D &triangleV0,\
    const RenderMath::Vec2D &triangleV1,const RenderMath::Vec2D &triangleV2){
        // 1. 计算总面积 (有向面积)
        // 叉积公式: (v1-v0) x (v2-v0)
        float areaABC = RenderMath::CrossProduct(triangleV1 - triangleV0, triangleV2 - triangleV0);
        
        // 2. 为了防止除以 0，这里给一个极小值作为 epsilon
        if (std::abs(areaABC) < 1e-6f) return {0, 0, 0}; 

        // 3. 计算子三角形的叉积 (有向面积)
        // alpha = Area(PBC) / Area(ABC)
        float areaPBC = RenderMath::CrossProduct(triangleV1 - p, triangleV2 - p);
        // beta = Area(PCA) / Area(ABC)
        float areaPCA = RenderMath::CrossProduct(triangleV2 - p, triangleV0 - p);
        
        BarycentricCoord bc;
        bc.a = areaPBC / areaABC;
        bc.b = areaPCA / areaABC;
        bc.c = 1.0f - bc.a - bc.b;
        return bc;
    }
};
struct Fragment{
    RenderMath::Vec2D standardUV;
    RenderMath::Vec3D normal;
    RenderMath::Vec3D worldPos;
    Color color;
    ORM orm;
    int trianglePtr;
    float depth;
    float w;
    BarycentricCoord bcCoor;
    int objId;
    bool isDrawn = false; 
    float shadowFactor;
};
struct BoundingBox{
    int left;
    int right;
    int top;
    int bottom;
    static BoundingBox GetBoundingBox(const RenderMath::Vec2D& v0Pos,\
        const RenderMath::Vec2D& v1Pos,const RenderMath::Vec2D& v2Pos){
        BoundingBox box;
        box.left = std::min({floor(v0Pos.x),floor(v1Pos.x),floor(v2Pos.x)});
        box.right = std::max({ceil(v0Pos.x),ceil(v1Pos.x),ceil(v2Pos.x)});
        box.top = std::min({floor(v0Pos.y),floor(v1Pos.y),floor(v2Pos.y)});
        box.bottom = std::max({ceil(v0Pos.y),ceil(v1Pos.y),ceil(v2Pos.y)});
        return box;
    }
};




class Rasterization{
private:
    inline static bool isInit = false;
    std::vector<MeshData> cutTriStore;
    std::vector<float> shadowDepth;
    Rasterization() = default;
    void CutTriangle(int triangleIt,int objId,const MeshData&);
    Vertex LerpVertex(const Vertex &curV,const Vertex&nextV);
public:
    static std::shared_ptr<Rasterization> RasterizationFactory(){
        if(isInit) return nullptr;
        isInit = true;
        return std::shared_ptr<Rasterization>(new Rasterization());
    }
    void Rasterize(std::vector<Fragment>&, std::vector<WorldObject> &worldObjs);
    void MakeShadow(std::vector<WorldObject> &worldObjs);
#ifdef __DEBUG__
    std::vector<RenderMath::Vec3D> normalFin;
    std::vector<RenderMath::Vec3D> baseColorFin;
    std::vector<RenderMath::Vec3D> ormFin;
#endif
};