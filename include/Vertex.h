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
    std::vector<Triangle> trangles;
};

struct Color{
    float red;
    float green;
    float blue;
};
struct ORM{
    float occlusion;
    float roughness;
    float meterillic;
};

struct Texture
{
    std::vector<RenderMath::Vec3D> normal;
    std::vector<Color>rgb;
    std::vector<ORM> orm;
};
