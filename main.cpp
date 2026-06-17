#include <iostream>
#include "Projection.h"
#include "FlieManager.h"
#include "Rasterization.h"
#include "Shader.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

void FocusOnShield(Camera& camera,RenderMath::Vec3D& cameraLookAt){
    camera.worldPos = RenderMath::Vec3D(-150,100,-150);
    cameraLookAt = RenderMath::Vec3D(-100,0,-100);
}

void FocusOnSword(Camera& camera,RenderMath::Vec3D& cameraLookAt){
    camera.worldPos = RenderMath::Vec3D(0,50,100);
    cameraLookAt = RenderMath::Vec3D(0,70,0);
}

int main(){
    //设置相机，点光源，物体位置
    Camera camera({50,50,50},{0,0,1});
    PointLight light({150,150,150},{Colors::White},8.f);
    AmbientLight ambLight(Colors::White,0.15f);
    std::vector<WorldObject> objs;
    objs.push_back(WorldObject::MakeGround());
    
    //读取模型和贴图
    std::string swordObjFile = "input/SM_Weapon.OBJ";
    std::string swordNormal = "Input\\T_IronWeapon_2_Normal.PNG";
    std::string swordBaseColor = "Input\\T_IronWeapon_2_BaseColor.PNG";
    std::string swordORM = "Input\\T_IronWeapon_2_OcclusionRoughnessMetallic.PNG";
    objs.push_back(WorldObject("Sword",{0,0,0},{0,20,0}));
    FileManager::LoadObject(objs[objs.size()-1],swordObjFile,swordNormal,swordBaseColor,swordORM);
    
    std::string shieldObjFile = "Input\\SM_Medieval_Shield.OBJ";
    std::string shieldNormal = "Input\\Medieval_Shield_low_M_Medieval_Shield_Norm.PNG";
    std::string shieldBaseColor = "Input\\Medieval_Shield_low_M_Medieval_Shield_Base.PNG";
    std::string shieldAO = "Input\\M_Medieval_Shield_ambient_occlusion_mix_tg.PNG";
    std::string shieldRough = "Input\\Medieval_Shield_low_M_Medieval_Shield_Roug.PNG";
    std::string shieldMeta = "Input\\Medieval_Shield_low_M_Medieval_Shield_Meta.PNG";
    objs.push_back(WorldObject("Shield",{0,0,0},{-100,20,-100}));
    FileManager::LoadObject(objs[objs.size()-1],shieldObjFile,shieldNormal,shieldBaseColor,shieldMeta);
   
    RenderMath::Vec3D cameraLookAt;
    FocusOnSword(camera,cameraLookAt);
    //FocusOnShield(camera,cameraLookAt);

    //渲染管线开启
    //1. 投影 
    //输入：顶点，三角形
    //输出：带有投影位置，以及tbn的顶点 
    auto proj = Projection::ProjectionFactory(camera.worldPos,light.worldPos,cameraLookAt);
    int objCnt = objs.size();
    for(int i = 0;i < objCnt;++i)
    {
        if(objs[i].isDrawn) {
            proj->ShadowProj(objs[i],false);
            proj->Project(objs[i],false);
        }
        else {
            proj->ShadowProj(objs[i],true);
            proj->Project(objs[i],true);
        }
    }
    //2. 光栅化插值
    //输入：上一步输出的顶点，三角形，basecolor，orm，normal三张贴图组成的纹理
    //输出：插值完的片元
    auto raster = Rasterization::RasterizationFactory();
    std::vector<Fragment> fragments;
    raster->MakeShadow(objs);
    raster->Rasterize(fragments,objs);
    //3. shader光照处理
    //输入：上一步输出的片元，以及全局摄像机，物体，环境光
    //输出：归一化的颜色
    auto shader = Shader::ShaderFactory(camera,light,ambLight);
    auto finalColor = shader->Shading(fragments);
    
    //4. 输出图片
    FileManager::WritePNG("Output/Output.png",finalColor);
#ifdef __DEBUG__
    FileManager::WritePNGLinear("Output/Normal.png",raster->normalFin);
    FileManager::WritePNGLinear("Output/BaseColor.png",raster->baseColorFin);
    FileManager::WritePNG("Output/ORM.png",raster->ormFin);
    FileManager::WritePNG("Output/Shadow.png",shader->shadowFin);
    FileManager::WritePNG("Output/DirectLight.png",shader->directLightFin);
    FileManager::WritePNG("Output/DiffuseLight.png",shader->diffuseLightFin);
    FileManager::WritePNG("Output/SpecularLight.png",shader->specularLightFin);
    FileManager::WritePNG("Output/ambientLight.png",shader->ambLightFin);
#endif
    return 0;
}