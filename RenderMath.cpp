#include "RenderMath.h"
#include "Vertex.h"
using namespace RenderMath;

Vec3D::Vec3D(const Color& c){
    x = c.red;
    y = c.green;
    z = c.blue;
}

Vec3D::Vec3D(const ORM& orm){
    x = orm.occlusion;
    y = orm.roughness;
    z = orm.meterillic;
}