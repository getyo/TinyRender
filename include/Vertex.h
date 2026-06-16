#pragma once
#include "RenderMath.h"
#include <vector>
struct Vertex
{
    //世界坐标
    RenderMath::Vec3D pos3D;
    //纹理坐标
    RenderMath::Vec2D textureUV;
    //投影之后的坐标,前两项/w之后就是标准屏幕空间坐标
    RenderMath::Vec4D posProj;
    //法线和法线贴图处理要用的
    RenderMath::Vec3D normal = {0,0,0};
    RenderMath::Vec3D tangent = {0,0,0};
    RenderMath::Vec3D bitangent = {0,0,0};
    void divW(){
        
    }
};

struct Triangle
{
    //三个顶点在顶点数组的索引
    int vertexIndex[3];
    int textureId;
};

struct MeshData
{
    std::vector<Vertex> vertices;
    std::vector<Triangle> triangles;
};

struct Color{
    float red;
    float green;
    float blue;
    Color(){}
    Color(float _r, float _g, float _b) : red(_r), green(_g), blue(_b) {}
    Color(const RenderMath::Vec3D& v):red(v.x),green(v.y),blue(v.z){}
    RenderMath::Vec3D ToVec3D(){
        return RenderMath::Vec3D(red,green,blue);
    }
};
namespace Colors{
    static const Color Black = {0.0f, 0.0f, 0.0f};
    static const Color White = {1.0f, 1.0f, 1.0f};
    static const Color Red   = {1.0f, 0.0f, 0.0f};
    static const Color Green = {0.0f, 1.0f, 0.0f};
    static const Color Blue  = {0.0f, 0.0f, 1.0f};
    static const Color Gold  = {1.0f, 0.71f, 0.29f};
    static const Color Grey = {0.1f, 0.1f, 0.1f};
}

struct ORM{
    float occlusion;
    float roughness;
    float meterillic;
    ORM(){}
    ORM(float occlusion,float roughness,float meterillic):occlusion(occlusion),roughness(roughness),meterillic(meterillic){}
    ORM(const RenderMath::Vec3D& v):occlusion(v.x),roughness(v.y),meterillic(v.z){}
    RenderMath::Vec3D ToVec3D(){
        return RenderMath::Vec3D(occlusion,roughness,meterillic);
    }
};

struct Texture
{
    float normalW,normalH;
    float rgbW,rgbH;
    float ormW,ormH;
    std::vector<RenderMath::Vec3D> normal;
    std::vector<Color>rgb;
    std::vector<ORM> orm;
};
