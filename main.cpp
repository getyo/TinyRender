#include <iostream>
#include "Projection.h"
#include "FlieManager.h"
#include "Resterization.h"
#include "Shader.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

int main(){
    //读取模型和贴图
    std::vector<MeshData> shapes;
    FileManager::ReadMeshData("input/SM_Weapon.OBJ",shapes);
    Texture texture;
    FileManager::ReadTexture("Input\\T_IronWeapon_2_Normal.PNG",\
        "Input\\T_IronWeapon_2_BaseColor.PNG",\
        "Input\\T_IronWeapon_2_OcclusionRoughnessMetallic.PNG",
        texture
    );
    //设置相机，点光源，物体位置
    Camera camera({0,0,0},{0,0,1});
    PointLight light({0,-100,50},{Colors::White},10.f);
    WorldObject worldObj({0,50,100},{0,-30,0});
    AmbientLight ambLight(Colors::White,0.1f);
    
    //渲染管线开启
    //1. 投影 
    //输入：顶点，三角形
    //输出：带有投影位置，以及tbn的顶点 
    auto proj = Projection::ProjectionFactory(camera.worldPos,light.worldPos,worldObj.worldPos,worldObj.RelatetiveOffset);
    proj->Project(shapes[0].vertices,shapes[0].triangles);
    //2. 光栅化插值
    //输入：上一步输出的顶点，三角形，basecolor，orm，normal三张贴图组成的纹理
    //输出：插值完的片元
    auto rester = Resterization::RasterizationFactory();
    std::vector<Fragment> fragments;
    rester->Rasterize(fragments,shapes[0].vertices,shapes[0].triangles,texture);
    //3. shader光照处理
    //输入：上一步输出的片元，以及全局摄像机，物体，环境光
    //输出：归一化的颜色
    auto shader = Shader::ShaderFactory(camera,light,ambLight);
    auto finalColor = shader->Shading(fragments);
    
    //4. 输出图片
    FileManager::WritePNG("Output/Output.png",finalColor);
#ifdef __DEBUG__
    FileManager::WritePNGLinear("Output/Normal.png",rester->normalFin);
    FileManager::WritePNGLinear("Output/BaseColor.png",rester->baseColorFin);
    FileManager::WritePNG("Output/ORM.png",rester->ormFin);
    FileManager::WritePNG("Output/DirectLight.png",shader->directLightFin);
    FileManager::WritePNG("Output/DiffuseLight.png",shader->diffuseLightFin);
    FileManager::WritePNG("Output/SpecularLight.png",shader->specularLightFin);
    FileManager::WritePNG("Output/ambientLight.png",shader->ambLightFin);
#endif
    return 0;
}