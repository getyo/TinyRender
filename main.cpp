#include <iostream>
#include "Projection.h"
#include "FlieManager.h"
#include "Resterization.h"

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
    RenderMath::Vec3D cameraPos(0,0,0);
    RenderMath::Vec3D lightPos(0,0,-100);
    RenderMath::Vec3D objPos(0,0,-200);
    //渲染管线开启
    //1. 投影 
    //输入：顶点，三角形
    //输出：带有投影位置，以及tbn的顶点 
    auto proj = Projection::ProjectionFactory(cameraPos,lightPos,objPos);
    proj->Project(shapes[0].vertices,shapes[0].triangles);
    //2. 光栅化插值
    //输入：上一步输出的顶点，三角形，basecolor，orm，normal三张贴图组成的纹理
    //输出：插值完的片元
    auto rester = Resterization::RasterizationFactory();
    std::vector<Fragment> fragments;
    rester->Rasterize(fragments,shapes[0].vertices,shapes[0].triangles,texture);
    return 0;
}