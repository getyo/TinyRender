#pragma once
#include "RenderMath.h"
#include "Vertex.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <math.h>

class FileManager{
public:
    virtual ~FileManager() = 0;
    static bool ReadMeshData(const std::string filePath,std::vector<MeshData>& meshDatas);
    static bool ReadTexture(const std::string normalPath,const std::string colorPath,const std::string ormPath,Texture& texture);
};


bool FileManager::ReadMeshData(const std::string filePath, std::vector<MeshData>& meshDatas) {
    tinyobj::ObjReaderConfig reader_config;
    reader_config.triangulate = true; // 确保切成三角形

    tinyobj::ObjReader reader;
    if (!reader.ParseFromFile(filePath, reader_config)) {
        if (!reader.Error().empty()) {
            std::cerr << "TinyObjLoader error: " << reader.Error() << std::endl;
        }
        return false;
    }
    
    auto& attrib = reader.GetAttrib();
    auto& shapes = reader.GetShapes();
    int scnt = shapes.size();
    meshDatas.resize(scnt);
    
    // 对每个 Shape (子网格) 进行遍历
    for (int i = 0; i < scnt; ++i) {
        auto& indices = shapes[i].mesh.indices;
        int totalIndices = indices.size();
        
        meshDatas[i].vertices.resize(totalIndices);
        meshDatas[i].triangles.resize(totalIndices / 3);
        
        // 对每个三角形进行组装
        for (int j = 0; j < totalIndices; j += 3) {
            auto& curTriangle = meshDatas[i].triangles[j / 3];
            
            // 防御未指定材质的 -1 边界情况
            int matId = shapes[i].mesh.material_ids[j / 3];
            curTriangle.textureId = (matId < 0) ? 0 : matId;
            
            for (int k = 0; k < 3; ++k) {
                int globalIdx = j + k;
                curTriangle.vertexIndex[k] = globalIdx; // 映射到全新的拓扑索引
                
                auto& curVertex = meshDatas[i].vertices[globalIdx];
                auto& idxData = indices[globalIdx];
                
                // 获取空间几何位置
                long long posIndex = idxData.vertex_index * 3;
                curVertex.pos3D.x = attrib.vertices[posIndex];
                curVertex.pos3D.y = attrib.vertices[posIndex + 1];
                curVertex.pos3D.z = attrib.vertices[posIndex + 2];
                
                // 获取纹理 UV 坐标
                if (idxData.texcoord_index >= 0) {
                    long long uvIndex = idxData.texcoord_index * 2;
                    curVertex.textureUV.x = attrib.texcoords[uvIndex];
                    curVertex.textureUV.y = attrib.texcoords[uvIndex + 1];
                } else {
                    curVertex.textureUV.x = 0.0f;
                    curVertex.textureUV.y = 0.0f;
                }
            }
        }
    }
    return true;
}

bool FileManager::ReadTexture(const std::string normalPath,const std::string colorPath,const std::string ormPath,Texture& texture){
    //加载颜色
    int width, height, original_channels;
    unsigned char* imageData = stbi_load(colorPath.c_str(), &width, &height, &original_channels, 3);
    if (imageData == nullptr) {
        printf("BaseColor load failed: %s\n", stbi_failure_reason());
        return false;
    }

    auto &colorCache = texture.rgb;
    colorCache.resize(height * width);
    texture.rgbH = height;
    texture.rgbW = width;
    for(int i = 0;i < height; ++i){
        for(int j = 0;j < width;++j){
            int itPos = i*width + j ;
            colorCache[itPos].red = static_cast<float>(imageData[itPos * 3]) / 255.f;
            colorCache[itPos].green = static_cast<float>(imageData[itPos * 3 + 1]) / 255.f;
            colorCache[itPos].blue = static_cast<float>(imageData[itPos * 3 + 2]) / 255.f;
            colorCache[itPos].red = pow(colorCache[itPos].red,2.2f);
            colorCache[itPos].green = pow(colorCache[itPos].green,2.2f);
            colorCache[itPos].blue = pow(colorCache[itPos].blue,2.2f);
        }
    }
    stbi_image_free(imageData);

    //加载法线
    imageData = stbi_load(normalPath.c_str(), &width, &height, &original_channels, 3);
    if (imageData == nullptr) {
        printf("Normal load failed: %s\n", stbi_failure_reason());
        return false;
    }

    auto &normalCache = texture.normal;
    normalCache.resize(height * width);
    texture.normalH = height;
    texture.normalW = width;
    for(int i = 0;i < height; ++i){
        for(int j = 0;j < width;++j){
            int itPos = i*width + j ;
            normalCache[itPos].x = (static_cast<float>(imageData[itPos * 3]) / 255.f) * 2.f -1.f;
            normalCache[itPos].y = (static_cast<float>(imageData[itPos * 3 + 1]) / 255.f) * 2.f -1.f;
            normalCache[itPos].z = (static_cast<float>(imageData[itPos * 3 + 2]) / 255.f) * 2.f -1.f;

            normalCache[itPos].y = -normalCache[itPos].y;
            normalCache[itPos] = RenderMath::Normalize(normalCache[itPos]);
        }
    }
    stbi_image_free(imageData);

    //记载环境光遮盖，粗糙度，金属度
    imageData = stbi_load(ormPath.c_str(), &width, &height, &original_channels, 3);
    if (imageData == nullptr) {
        printf("ORM load failed: %s\n", stbi_failure_reason());
        return false;
    }

    auto &ormCache = texture.orm;
    ormCache.resize(height * width);
    texture.ormH = height;
    texture.ormW = width;
    for(int i = 0;i < height; ++i){
        for(int j = 0;j < width;++j){
            int itPos = i*width + j ;
            ormCache[itPos].occlusion = static_cast<float>(imageData[itPos * 3]) / 255.f;
            ormCache[itPos].roughness = static_cast<float>(imageData[itPos * 3 + 1]) / 255.f;
            ormCache[itPos].meterillic = static_cast<float>(imageData[itPos * 3 + 2]) / 255.f;
        }
    }
    stbi_image_free(imageData);

    return true;
}