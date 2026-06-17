#pragma once
#include "RenderMath.h"
#include "Vertex.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"
#include <math.h>
#include <algorithm>

class FileManager{
public:
    virtual ~FileManager() = 0;
    static bool ReadMeshData(const std::string filePath,MeshData& meshData);
    static bool ReadTexture(const std::string normalPath,const std::string colorPath,const std::string ormPath,Texture& texture);
    static void WritePNG(const std::string filePath,std::vector<Color>& color);
    static void WritePNG(const std::string filePath,std::vector<RenderMath::Vec3D>& color);
    static std::vector<unsigned char> CastToPNGColor(std::vector<Color>& color);
    static std::vector<unsigned char> CastToPNGColor(std::vector<RenderMath::Vec3D>& color);
    static void WritePNGLinear(const std::string& filePath, std::vector<RenderMath::Vec3D>& data) {
        int width = ScreenWidth, height = ScreenHeight;
        std::vector<unsigned char> pixels(width * height * 3);
        for (int i = 0; i < width * height; ++i) {
            pixels[i*3+0] = (unsigned char)std::clamp((data[i].x +1.f) / 2* 255.f, 0.f, 255.f);
            pixels[i*3+1] = (unsigned char)std::clamp((data[i].y +1.f) / 2 * 255.f, 0.f, 255.f);
            pixels[i*3+2] = (unsigned char)std::clamp((data[i].z +1.f) / 2 * 255.f, 0.f, 255.f);
        }
        stbi_write_png(filePath.c_str(), width, height, 3, pixels.data(), width * 3);
    }
};

bool FileManager::ReadMeshData(const std::string filePath, MeshData& meshData) {
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
    if(shapes.size() > 1) 
        std::cerr << "OBJ has more than one mesh \n";
    
    
    auto& indices = shapes[0].mesh.indices;
    int totalIndices = indices.size();    
    meshData.vertices.resize(totalIndices);
    meshData.triangles.resize(totalIndices / 3);
        
    // 对每个三角形进行组装
    for (int j = 0; j < totalIndices; j += 3) {
        auto& curTriangle = meshData.triangles[j / 3];
            
        // 防御未指定材质的 -1 边界情况
        int matId = shapes[0].mesh.material_ids[j / 3];
            curTriangle.textureId = (matId < 0) ? 0 : matId;
            
        for (int k = 0; k < 3; ++k) {
            int globalIdx = j + k;
            curTriangle.vertexIndex[k] = globalIdx; // 映射到全新的拓扑索引
                
            auto& curVertex = meshData.vertices[globalIdx];
            auto& idxData = indices[globalIdx];
                
            // 获取空间几何位置 (UE 左手系 -> 右手系)
            long long posIndex = idxData.vertex_index * 3;
            curVertex.pos3D.x = attrib.vertices[posIndex]; 
            curVertex.pos3D.y = attrib.vertices[posIndex + 1]; 
            curVertex.pos3D.z = attrib.vertices[posIndex +2 ];     
                
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

std::vector<unsigned char> FileManager::CastToPNGColor(std::vector<Color>& color){
    int width = ScreenWidth;
    int height = ScreenHeight;
    for(auto &c:color){
        c.red = pow(c.red,1/2.2f) * 255.f;
        c.green = pow(c.green,1/2.2f) * 255.f;
        c.blue = pow(c.blue,1/2.2f) * 255.f;
    }
    // 准备一个 buffer 来存储 byte 数据
    std::vector<unsigned char> pixels(width * height * 3);

    for (int i = 0; i < width * height; ++i) {
        pixels[i * 3 + 0] = static_cast<unsigned char>(std::clamp(color[i].red, 0.0f, 255.0f));
        pixels[i * 3 + 1] = static_cast<unsigned char>(std::clamp(color[i].green, 0.0f, 255.0f));
        pixels[i * 3 + 2] = static_cast<unsigned char>(std::clamp(color[i].blue, 0.0f, 255.0f));
    }
    return pixels;
}

std::vector<unsigned char> FileManager::CastToPNGColor(std::vector<RenderMath::Vec3D>& color){
    int width = ScreenWidth;
    int height = ScreenHeight;
    for(auto &c:color){
        c.x = pow(c.x,1/2.2f) * 255.f;
        c.y = pow(c.y,1/2.2f) * 255.f;
        c.z = pow(c.z,1/2.2f) * 255.f;
    }
    // 准备一个 buffer 来存储 byte 数据
    std::vector<unsigned char> pixels(width * height * 3);

    for (int i = 0; i < width * height; ++i) {
        pixels[i * 3 + 0] = static_cast<unsigned char>(std::clamp(color[i].x, 0.0f, 255.0f));
        pixels[i * 3 + 1] = static_cast<unsigned char>(std::clamp(color[i].y, 0.0f, 255.0f));
        pixels[i * 3 + 2] = static_cast<unsigned char>(std::clamp(color[i].z, 0.0f, 255.0f));
    }
    return pixels;
}

void FileManager::WritePNG(const std::string filePath,std::vector<Color>& color){
    auto pixels = FileManager::CastToPNGColor(color);
    stbi_write_png(filePath.c_str(), ScreenWidth, ScreenHeight, 3, pixels.data(), ScreenWidth * 3);
}

void FileManager::WritePNG(const std::string filePath,std::vector<RenderMath::Vec3D>& color){
    auto pixels = FileManager::CastToPNGColor(color);
    stbi_write_png(filePath.c_str(), ScreenWidth, ScreenHeight, 3, pixels.data(), ScreenWidth * 3);
}
