#include "Rasterization.h"
#include "Global.h"
#include <cmath>
#include <iostream>
#include <cassert>
#include "Projection.h"

void Rasterization::Rasterize(std::vector<Fragment> &fragments, std::vector<WorldObject> &worldObjs)
{
    // 深度预处理
    int screenSize = ScreenWidth * ScreenHeight;
    int objCnt = worldObjs.size();
    fragments.resize(screenSize);
    cutTriStore.resize(objCnt);
#ifdef __DEBUG__
    normalFin.resize(screenSize);
    baseColorFin.resize(screenSize);
    ormFin.resize(screenSize);
#endif
    for (int i = 0; i < ScreenWidth * ScreenHeight; ++i)
    {
        fragments[i].trianglePtr = -1;
        fragments[i].depth = std::numeric_limits<float>::max();
    }
    for (int objIt = 0; objIt < objCnt; ++objIt)
    {
        cutTriStore[objIt].vertices.clear();
        cutTriStore[objIt].triangles.clear();
        auto &addedTri = cutTriStore[objIt].triangles;
        auto &addedVtx = cutTriStore[objIt].vertices;
        auto &worldObj = worldObjs[objIt];
        auto &vertices = worldObj.meshData.vertices;
        auto &triangles = worldObj.meshData.triangles;
        auto &texture = worldObj.texture;
        int originSize = triangles.size();
        bool hasCut = false;
        for (int it = 0; it < originSize + addedTri.size(); ++it)
        {
            Vertex *v0, *v1, *v2;
            if (it < originSize)
            {
                v0 = &vertices[triangles[it].vertexIndex[0]];
                v1 = &vertices[triangles[it].vertexIndex[1]];
                v2 = &vertices[triangles[it].vertexIndex[2]];
            }
            else
            {
                hasCut = true;
                v0 = &addedVtx[addedTri[it - originSize].vertexIndex[0]];
                v1 = &addedVtx[addedTri[it - originSize].vertexIndex[1]];
                v2 = &addedVtx[addedTri[it - originSize].vertexIndex[2]];
            }
            // 没一个顶点在，直接全剪掉
            if (v0->posProj.w < 0 && v1->posProj.w < 0 && v2->posProj.w < 0)
                continue;
            else if ((v0->posProj.w < 0 || v1->posProj.w < 0 || v2->posProj.w < 0) && !hasCut)
            {
                CutTriangle(it, objIt, worldObj.meshData);
                continue;
            }
            RenderMath::Vec4D v0Pos4D = v0->posProj / v0->posProj.w;
            v0Pos4D = ViewportTransform * v0Pos4D;
            RenderMath::Vec4D v1Pos4D = v1->posProj / v1->posProj.w;
            v1Pos4D = ViewportTransform * v1Pos4D;
            RenderMath::Vec4D v2Pos4D = v2->posProj / v2->posProj.w;
            v2Pos4D = ViewportTransform * v2Pos4D;
            RenderMath::Vec2D v0Pos(v0Pos4D.x, v0Pos4D.y);
            RenderMath::Vec2D v1Pos(v1Pos4D.x, v1Pos4D.y);
            RenderMath::Vec2D v2Pos(v2Pos4D.x, v2Pos4D.y);
            BoundingBox box = BoundingBox::GetBoundingBox(v0Pos, v1Pos, v2Pos);
            // 不渲染在屏幕外的点
            box.top = std::max(0, box.top);
            box.bottom = std::min(ScreenHeight - 1, static_cast<float>(box.bottom));
            box.left = std::max(0, box.left);
            box.right = std::min(ScreenWidth - 1, static_cast<float>(box.right));

            for (int i = box.top; i <= box.bottom; ++i)
            {
                for (int j = box.left; j <= box.right; ++j)
                {
                    RenderMath::Vec2D fragmentPos(j + 0.5, i + 0.5);
                    BarycentricCoord tempBcCoor = BarycentricCoord::GetBarycentricOfP(fragmentPos, v0Pos, v1Pos, v2Pos);
                    if (tempBcCoor.a >= 0 && tempBcCoor.b >= 0 && tempBcCoor.c >= 0)
                    {

                        float tempW = tempBcCoor.a * (1.0f / v0->posProj.w) +
                                      tempBcCoor.b * (1.0f / v1->posProj.w) +
                                      tempBcCoor.c * (1.0f / v2->posProj.w);
                        float tempDepth = (v0->posProj.z / v0->posProj.w) * tempBcCoor.a +
                                          (v1->posProj.z / v1->posProj.w) * tempBcCoor.b +
                                          (v2->posProj.z / v2->posProj.w) * tempBcCoor.c;
                        auto &curFragment = fragments[i * ScreenWidth + j];
                        if (tempDepth <= curFragment.depth)
                        {
                            // 如果深度较小，更新这个过程算出来的结果，防止下一个循环在插值时重复计算
                            curFragment.depth = tempDepth;
                            curFragment.bcCoor = tempBcCoor;
                            curFragment.w = tempW;
                            curFragment.trianglePtr = it;
                            curFragment.objId = objIt;
                        }
                    }
                }
            }
        }
    }
    int totalCnt = 0;
    // 正式对片元属性光栅化插值
    for (int i = 0; i < ScreenHeight; ++i)
    {
        for (int j = 0; j < ScreenWidth; ++j)
        {
            int it = ScreenWidth * i + j;
            // 如果没有关联三角形，说明屏幕不显示，跳过插值
            if (fragments[it].trianglePtr == -1)
                continue;
            ++totalCnt;
            auto &curFragment = fragments[it];
            auto &texture = worldObjs[curFragment.objId].texture;
            Vertex *v0, *v1, *v2;
            auto &worldObjTri = worldObjs[curFragment.objId].meshData.triangles;
            auto &worldObjVtx = worldObjs[curFragment.objId].meshData.vertices;
            int originSize = worldObjTri.size();
            if (curFragment.trianglePtr < originSize)
            {
                v0 = &worldObjVtx[worldObjTri[curFragment.trianglePtr].vertexIndex[0]];
                v1 = &worldObjVtx[worldObjTri[curFragment.trianglePtr].vertexIndex[1]];
                v2 = &worldObjVtx[worldObjTri[curFragment.trianglePtr].vertexIndex[2]];
            }
            else
            {
                int newIt = curFragment.trianglePtr - originSize;
                auto &addedTri = cutTriStore[curFragment.objId].triangles;
                auto &addedVtx = cutTriStore[curFragment.objId].vertices;
                v0 = &addedVtx[addedTri[newIt].vertexIndex[0]];
                v1 = &addedVtx[addedTri[newIt].vertexIndex[1]];
                v2 = &addedVtx[addedTri[newIt].vertexIndex[2]];
            }

            // 世界位置坐标插值
            curFragment.worldPos = curFragment.bcCoor.a * (1 / v0->posProj.w * v0->pos3D) +
            curFragment.bcCoor.b * (1 / v1->posProj.w * v1->pos3D) +
            curFragment.bcCoor.c * (1 / v2->posProj.w * v2->pos3D);
            curFragment.worldPos = 1 / curFragment.w * curFragment.worldPos;

            // 更新阴影
            RenderMath::Vec4D lightPos = Projection::LightTrans  * RenderMath::Vec4D(curFragment.worldPos, 1.f);
            if (lightPos.w > 0)
            {
                lightPos = lightPos / lightPos.w;
                float lightZ = lightPos.z;
                lightPos = ViewportTransform * lightPos;
                int shadowDepthIt = std::floor(lightPos.y) * ScreenWidth + std::floor(lightPos.x);
                if (!InScreen(RenderMath::Vec2D(lightPos.x,lightPos.y))) 
                    curFragment.shadowFactor = 1.f;
                else if ((lightZ -shadowDepth[shadowDepthIt]) > ShadowThreshold) 
                    curFragment.shadowFactor = MinShadowFactor;
                else 
                    curFragment.shadowFactor = 1.f;
            }
            else
            {
                curFragment.shadowFactor = 1.f;
            }
            
            if (worldObjs[curFragment.objId].isDrawn)
            {
                curFragment.normal = WorldUpVec;
                curFragment.orm = WorldObject::GroundORM;
                curFragment.isDrawn = true;
                if (WorldObject::InGroundLine(curFragment.worldPos, v0->pos3D, v1->pos3D, v2->pos3D))
                curFragment.color = WorldObject::GroundLineColor;
                else
                curFragment.color = WorldObject::GroundBlankColor;
            }
            else
            {
                // 插值纹理坐标到标准空间
                float us = curFragment.bcCoor.a * (v0->textureUV.x / v0->posProj.w) +
                curFragment.bcCoor.b * (v1->textureUV.x / v1->posProj.w) +
                           curFragment.bcCoor.c * (v2->textureUV.x / v2->posProj.w);
                us /= curFragment.w;
                float vs = curFragment.bcCoor.a * (v0->textureUV.y / v0->posProj.w) +
                           curFragment.bcCoor.b * (v1->textureUV.y / v1->posProj.w) +
                           curFragment.bcCoor.c * (v2->textureUV.y / v2->posProj.w);
                vs /= curFragment.w;

                // BaseColor插值
                float uBC = us * texture.rgbW;
                float vBC = (1.0f - vs) * texture.rgbH; // 翻转V
                int sampleU = uBC, sampleV = vBC;
                int leftTop = sampleV * texture.rgbW + sampleU;
                int rightTop = sampleV * texture.rgbW + sampleU + 1;
                int leftButtom = (sampleV + 1) * texture.rgbW + sampleU;
                int rightButtom = (sampleV + 1) * texture.rgbW + sampleU + 1;
                curFragment.color = RenderMath::BlinearInterploation(RenderMath::Vec2D(uBC, vBC),
                                                                     texture.rgb[leftTop], texture.rgb[rightTop], texture.rgb[leftButtom], texture.rgb[rightButtom]);

                // 法线插值
                uBC = us * texture.normalW, vBC = (1.0f - vs) * texture.normalH;
                sampleU = uBC, sampleV = vBC;
                leftTop = sampleV * texture.normalW + sampleU;
                rightTop = sampleV * texture.normalW + sampleU + 1;
                leftButtom = (sampleV + 1) * texture.normalW + sampleU;
                rightButtom = (sampleV + 1) * texture.normalW + sampleU + 1;
                // 1.采样法线偏移量
                curFragment.normal = RenderMath::BlinearInterploation(RenderMath::Vec2D(uBC, vBC),
                                                                      texture.normal[leftTop], texture.normal[rightTop], texture.normal[leftButtom], texture.normal[rightButtom]);
                // 2.重心插值TBN
                RenderMath::Vec3D tangent = curFragment.bcCoor.a * (1 / v0->posProj.w * v0->tangent) +
                                            curFragment.bcCoor.b * (1 / v1->posProj.w * v1->tangent) +
                                            curFragment.bcCoor.c * (1 / v2->posProj.w * v2->tangent);
                tangent = 1 / curFragment.w * tangent;
                RenderMath::Vec3D normal = curFragment.bcCoor.a * (1 / v0->posProj.w * v0->normal) +
                                           curFragment.bcCoor.b * (1 / v1->posProj.w * v1->normal) +
                                           curFragment.bcCoor.c * (1 / v2->posProj.w * v2->normal);
                normal = 1 / curFragment.w * normal;
                // 3.修正三向量，使其重新正交化，单位化
                normal = RenderMath::Normalize(normal);
                tangent = RenderMath::Normalize(tangent - normal * DotProduct(tangent, normal));
                RenderMath::Vec3D biTangent = RenderMath::CrossProduct(normal, tangent);
                RenderMath::Mat3D tbn(tangent, biTangent, normal);
                // 4.tbn换系法线偏移
                curFragment.normal = tbn * curFragment.normal;
                curFragment.normal = RenderMath::Normalize(curFragment.normal);

                // 环境光遮盖，粗糙度，金属度插值
                uBC = us * texture.ormW, vBC = (1.0f - vs) * texture.ormH;
                sampleU = uBC, sampleV = vBC;
                leftTop = sampleV * texture.ormW + sampleU;
                rightTop = sampleV * texture.ormW + sampleU + 1;
                leftButtom = (sampleV + 1) * texture.ormW + sampleU;
                rightButtom = (sampleV + 1) * texture.ormW + sampleU + 1;
                curFragment.orm = RenderMath::BlinearInterploation(RenderMath::Vec2D(uBC, vBC),
                                                                   texture.orm[leftTop], texture.orm[rightTop], texture.orm[leftButtom], texture.orm[rightButtom]);
            }

#ifdef __DEBUG__
            normalFin[it] = curFragment.normal;
            baseColorFin[it] = curFragment.color;
            ormFin[it] = curFragment.orm;
#endif
        }
    }

#ifdef __DEBUG__
    std::cout << "Total VertexCnt :" << totalCnt << '\n';
#endif
    return;
}

void Rasterization::CutTriangle(int trianglePtr, int objId, const MeshData &meshData)
{
    auto &t = meshData.triangles[trianglePtr];
    std::vector<int> outputVIt;
    // 裁剪必须解决的顶点属性：
    // 1. 世界空间坐标
    // 2. 透视空间坐标
    // 3. 纹理空间坐标
    // 4. tb二向量
    int resIt = cutTriStore[objId].vertices.size();
    for (int i = 0; i < 3; ++i)
    {
        Vertex curV = meshData.vertices[t.vertexIndex[i]],
               nextV = meshData.vertices[t.vertexIndex[(i + 1) % 3]];
        // 两个顶点都没被剪掉
        if (curV.posProj.w >= 0 && nextV.posProj.w >= 0)
        {
            // 保留cur，下同
            cutTriStore[objId].vertices.push_back(curV);
            outputVIt.push_back(resIt++);
        }
        // next被剪掉
        else if (curV.posProj.w >= 0 && nextV.posProj.w < 0)
        {
            cutTriStore[objId].vertices.push_back(curV);
            outputVIt.push_back(resIt++);
            // 在next和cur里面插值一个Vertex
            cutTriStore[objId].vertices.push_back(LerpVertex(curV, nextV));
            outputVIt.push_back(resIt++);
        }
        // cur被剪掉
        else if (curV.posProj.w < 0 && nextV.posProj.w >= 0)
        {
            cutTriStore[objId].vertices.push_back(LerpVertex(nextV, curV));
            outputVIt.push_back(resIt++);
        }
    }

    // 生成三角形
    int size = outputVIt.size();
    // 切了一个角变成小三角
    if (size == 3)
    {
        cutTriStore[objId].triangles.push_back({outputVIt[0], outputVIt[1], outputVIt[2]});
    }
    else
    {
        assert(size == 4 && "Cut triangle failed");
        cutTriStore[objId].triangles.push_back({outputVIt[0], outputVIt[1], outputVIt[2]});
        cutTriStore[objId].triangles.push_back({outputVIt[0], outputVIt[2], outputVIt[3]});
    }
    return;
}

Vertex Rasterization::LerpVertex(const Vertex &insideV, const Vertex &outsideV)
{
    const float EPSILON = 1.f;
    float s = (EPSILON - insideV.posProj.w) / (outsideV.posProj.w - insideV.posProj.w);
    if (s < 0.0f)
        s = 0.0f;
    if (s > 1.0f)
        s = 1.0f;

    Vertex vNew;
    vNew.posProj = s * outsideV.posProj + (1.0f - s) * insideV.posProj;
    // 避免后续透视除法出问题
    if (vNew.posProj.w <= 0.0f)
        vNew.posProj.w = EPSILON;

    vNew.pos3D = s * outsideV.pos3D + (1.0f - s) * insideV.pos3D;
    float invW_in  = 1.0f / insideV.posProj.w;
    float invW_out = 1.0f / outsideV.posProj.w;

    // 计算新顶点精确的 1/w (1/w 在齐次空间是线性变化的)
    float invW_new = invW_in + (invW_out - invW_in) * s;
    // 得到新顶点在相机视锥体空间里真正的非线性权重 W
    float w_new = 1.0f / invW_new; 

    // 对所有属性执行 [属性/w] 的齐次线性插值，再乘以新顶点的 w 还原
    // 公式：Attribute_New = ( (Attr_in / w_in) * (1-s) + (Attr_out / w_out) * s ) * w_new
    
    // --- 插值 TextureUV ---
    RenderMath::Vec2D uvOverW_in  = insideV.textureUV * invW_in;
    RenderMath::Vec2D uvOverW_out = outsideV.textureUV * invW_out;
    RenderMath::Vec2D uvOverW_new = uvOverW_in + (uvOverW_out - uvOverW_in) * s;
    vNew.textureUV = uvOverW_new * w_new;

    // --- 插值 Normal ---
    RenderMath::Vec3D nOverW_in  = insideV.normal * invW_in;
    RenderMath::Vec3D nOverW_out = outsideV.normal * invW_out;
    RenderMath::Vec3D nOverW_new = nOverW_in + (nOverW_out - nOverW_in) * s;
    vNew.normal = RenderMath::Normalize(nOverW_new * w_new); // 还原后记得重新单位化

    // --- 插值 Tangent ---
    RenderMath::Vec3D tOverW_in  = insideV.tangent * invW_in;
    RenderMath::Vec3D tOverW_out = outsideV.tangent * invW_out;
    RenderMath::Vec3D tOverW_new = tOverW_in + (tOverW_out - tOverW_in) * s;
    vNew.tangent = RenderMath::Normalize(tOverW_new * w_new);

    // --- 插值 Bitangent ---
    RenderMath::Vec3D bOverW_in  = insideV.bitangent * invW_in;
    RenderMath::Vec3D bOverW_out = outsideV.bitangent * invW_out;
    RenderMath::Vec3D bOverW_new = bOverW_in + (bOverW_out - bOverW_in) * s;
    vNew.bitangent = RenderMath::Normalize(bOverW_new * w_new);

    return vNew;
}

void Rasterization::MakeShadow(std::vector<WorldObject> &worldObjs)
{
    int screenSize = ScreenWidth * ScreenHeight;
    int objCnt = worldObjs.size();
    cutTriStore.resize(objCnt);
    shadowDepth.resize(screenSize);

    for (int i = 0; i < ScreenWidth * ScreenHeight; ++i)
    {
        shadowDepth[i] = std::numeric_limits<float>::max();
    }
    for (int objIt = 0; objIt < objCnt; ++objIt)
    {
        auto &worldObj = worldObjs[objIt];
        auto &vertices = worldObj.meshData.vertices;
        auto &triangles = worldObj.meshData.triangles;
        auto &texture = worldObj.texture;
        int triCnt = triangles.size();

        for (int it = 0; it < triCnt; ++it)
        {
            Vertex *v0, *v1, *v2;
            v0 = &vertices[triangles[it].vertexIndex[0]];
            v1 = &vertices[triangles[it].vertexIndex[1]];
            v2 = &vertices[triangles[it].vertexIndex[2]];
            
         

            if ((v0->lightProj.w < 0 || v1->lightProj.w < 0 || v2->lightProj.w < 0))
            {
                continue;
            }
            RenderMath::Vec4D v0Pos4D = v0->lightProj / v0->lightProj.w;
            v0Pos4D = ViewportTransform * v0Pos4D;
            RenderMath::Vec4D v1Pos4D = v1->lightProj / v1->lightProj.w;
            v1Pos4D = ViewportTransform * v1Pos4D;
            RenderMath::Vec4D v2Pos4D = v2->lightProj / v2->lightProj.w;
            v2Pos4D = ViewportTransform * v2Pos4D;
            RenderMath::Vec2D v0Pos(v0Pos4D.x, v0Pos4D.y);
            RenderMath::Vec2D v1Pos(v1Pos4D.x, v1Pos4D.y);
            RenderMath::Vec2D v2Pos(v2Pos4D.x, v2Pos4D.y);
            BoundingBox box = BoundingBox::GetBoundingBox(v0Pos, v1Pos, v2Pos);
            // 不渲染在屏幕外的点
            box.top = std::max(0, box.top);
            box.bottom = std::min(ScreenHeight - 1, static_cast<float>(box.bottom));
            box.left = std::max(0, box.left);
            box.right = std::min(ScreenWidth - 1, static_cast<float>(box.right));

            for (int i = box.top; i <= box.bottom; ++i)
            {
                for (int j = box.left; j <= box.right; ++j)
                {
                    RenderMath::Vec2D fragmentPos(j + 0.5, i + 0.5);
                    BarycentricCoord tempBcCoor = BarycentricCoord::GetBarycentricOfP(fragmentPos, v0Pos, v1Pos, v2Pos);
                    if (tempBcCoor.a >= 0 && tempBcCoor.b >= 0 && tempBcCoor.c >= 0)
                    {

                        float tempW = tempBcCoor.a * (1.0f / v0->lightProj.w) +
                                      tempBcCoor.b * (1.0f / v1->lightProj.w) +
                                      tempBcCoor.c * (1.0f / v2->lightProj.w);
                        float tempDepth = (v0->lightProj.z / v0->lightProj.w) * tempBcCoor.a +
                                          (v1->lightProj.z / v1->lightProj.w) * tempBcCoor.b +
                                          (v2->lightProj.z / v2->lightProj.w) * tempBcCoor.c;
                        if (tempDepth < shadowDepth[i * ScreenWidth + j])
                        {
                           shadowDepth[i * ScreenWidth + j] = tempDepth;
                        }
                    }
                }
            }
        }
    }
}