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

Vec3D Vec3D::operator=(const ORM& orm){
    x = orm.occlusion;
    y = orm.roughness;
    z = orm.meterillic;
    return *this;
}

Vec3D Vec3D::operator=(const Color& c){
    this->x = c.red;
    this->y = c.green;
    this->z = c.blue;
    return *this;
}

Mat4D Mat3D::ToMat4D(){
    return Mat4D(*this);
}