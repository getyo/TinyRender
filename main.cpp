#include <iostream>
#include "Projection.h"
#include "FlieManager.h"
#include "Resterization.h"

int main(){
    int widget = 1024,height = 1024;
    std::vector<MeshData> shapes;
    FileManager::ReadMeshData("input/SM_Weapon.OBJ",shapes);
    Texture t;
    FileManager::ReadTexture("Input\\T_IronWeapon_2_Normal.PNG",\
        "Input\\T_IronWeapon_2_BaseColor.PNG",\
        "Input\\T_IronWeapon_2_OcclusionRoughnessMetallic.PNG",
        t
    );
    RenderMath::Vec3D cameraPos(0,0,0);
    RenderMath::Vec3D lightPos(0,0,-100);
    RenderMath::Vec3D objPos(0,0,-200);
    auto proj = Projection::ProjectionFactory(cameraPos,lightPos,objPos);
    proj->Project(shapes[0].vertices,shapes[0].triangles);
    auto rester = Resterization::RasterizationFactory();
    rester->Rasterize(shapes[0].vertices,shapes[0].triangles,t);
    return 0;
}