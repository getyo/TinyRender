#pragma once
#include "RenderMath.h"
#include "Vertex.h"
#include "Resterization.h"
#include "Global.h"
#include <memory>

class Shader
{
private:
    inline static bool isInit = false;
    Camera camera;
    PointLight lightSource;
    AmbientLight ambLight;
    Shader(Camera camera,PointLight lightSource,AmbientLight ambLight):camera(camera),lightSource(lightSource),
    ambLight(ambLight){}
public:
#ifdef __DEBUG__
    std::vector<Color> directLightFin;
    std::vector<Color> ambLightFin;
    std::vector<Color> diffuseLightFin;
    std::vector<Color> specularLightFin;
#endif

    static std::shared_ptr<Shader> ShaderFactory(Camera camera,PointLight lightSource,AmbientLight ambLight){
        if(isInit) return nullptr;
        isInit = true;
        return std::shared_ptr<Shader>(new Shader(camera,lightSource,ambLight));
    }
    std::vector<Color> Shading(std::vector<Fragment>&);
};

