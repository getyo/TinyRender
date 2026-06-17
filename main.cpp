#include <iostream>
#include "Projection.h"
#include "FlieManager.h"
#include "Rasterization.h"
#include "Shader.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

int main(){
    //设置相机，点光源，物体位置
    Camera camera({100,30.f,0},{0,0,1});
    PointLight light({0,100,50},{Colors::White},20.f);
    AmbientLight ambLight(Colors::White,0.1f);
    std::vector<WorldObject> objs;
    //读取模型和贴图
    /*
    objs.push_back(WorldObject("Sword",{0,50,100},{0,-30,0}));
    FileManager::ReadMeshData("input/SM_Weapon.OBJ",objs[0].meshData);
    FileManager::ReadTexture("Input\\T_IronWeapon_2_Normal.PNG",\
        "Input\\T_IronWeapon_2_BaseColor.PNG",\
        "Input\\T_IronWeapon_2_OcclusionRoughnessMetallic.PNG",
        objs[0].texture
    );
    */
    objs.push_back(WorldObject::MakeGround());
    
    //渲染管线开启
    //1. 投影 
    //输入：顶点，三角形
    //输出：带有投影位置，以及tbn的顶点 
    auto proj = Projection::ProjectionFactory(camera.worldPos,light.worldPos);
    for(auto &obj:objs)
        proj->Project(obj,false);
    //2. 光栅化插值
    //输入：上一步输出的顶点，三角形，basecolor，orm，normal三张贴图组成的纹理
    //输出：插值完的片元
    auto rester = Rasterization::RasterizationFactory();
    std::vector<Fragment> fragments;
    rester->Rasterize(fragments,objs);
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