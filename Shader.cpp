#include "Shader.h"
#include <algorithm>

/*Shader需要用到的片元属性如下
*ORM,BaseColor,Normal,WorldPos
*/
std::vector<Color> Shader::Shading(std::vector<Fragment> &fragments){
    int size= fragments.size();
    std::vector<Color> finalColor(size);
#ifndef __NDEBUG__
    diffuseLightFin.resize(size);
    specularLightFin.resize(size);
    directLightFin.resize(size);
    ambLightFin.resize(size);
#endif
    int totalFrag = 0,specularFrag = 0;
    for(int i = 0;i < size;++i){
        Fragment& frag = fragments[i];
        if(frag.trianglePtr == -1){
            finalColor[i] = BackGroundColor;
#ifdef __DEBUG__
            diffuseLightFin[i] = BackGroundColor;
            specularLightFin[i] = BackGroundColor;
            directLightFin[i] = BackGroundColor;
            ambLightFin[i] = BackGroundColor;
#endif
           continue;
        }
        //如果是画出来的线，不计算光照
        if(!frag.standardUV.Length()){
            finalColor[i] = frag.color;        
            continue;
        }
        //finalColor[i] = frag.color;
        ++totalFrag;
        //1. 前置的常数变量计算
        float shininess = (1.f - frag.orm.roughness) *128.f;
        float spFactor = (shininess + 8.f) / (8*RenderMath::PI);
        //高光能量比例因子
        RenderMath::Vec3D F0(0.04f, 0.04f, 0.04f);
        // 根据金属度在 0.04 和物体表面颜色之间进行线性插值
        RenderMath::Vec3D spK = F0 * (1.f - frag.orm.meterillic) + frag.color.ToVec3D() * frag.orm.meterillic ;
        //散射能量比例因子
        RenderMath::Vec3D diffK = (1 - frag.orm.meterillic) * (RenderMath::Vec3D(1.f,1.f,1.f) - spK);
        RenderMath::Vec3D lightD = RenderMath::Normalize(lightSource.worldPos - frag.worldPos);
        RenderMath::Vec3D cameraD = RenderMath::Normalize(camera.worldPos - frag.worldPos);
        //Phong Shading半角向量
        // 确保 lightD 和 cameraD 都是单位向量（Normalize）
        RenderMath::Vec3D halfAngle = RenderMath::Normalize(lightD + cameraD);

        //2. 直射光：漫反射部分
        auto diffuseLight = diffK * (frag.color.ToVec3D()/RenderMath::PI);
        //3. 直射光：高光部分
        float NdotH = std::max(RenderMath::DotProduct(frag.normal, halfAngle), 0.f);
        //float NdotH = std::max(RenderMath::DotProduct(frag.normal, halfAngle), 0.f);
        auto specularLight = spK * spFactor * pow( NdotH, shininess);

        //4. 直射光就是两部分在光照能量和角度下的总和
        auto directLight = lightSource.intensity * std::max(RenderMath::DotProduct(frag.normal,lightD),0.f) * \
                            lightSource.color.ToVec3D() *(diffuseLight + specularLight);
        //5. 环境光
        float safeAO = frag.orm.occlusion <= 0 ? 0.1:frag.orm.occlusion;
        auto ambLightPerFrag = ambLight.color.ToVec3D() * frag.color * ambLight.intensity * safeAO;
        //6. 最终颜色 = 直射光+环境光
        finalColor[i] = ambLightPerFrag + directLight;
#ifdef __DEBUG__
        diffuseLightFin[i] = diffuseLight;
        specularLightFin[i] = specularLight;
        directLightFin[i] = directLight;
        ambLightFin[i] = ambLightPerFrag;
        if(specularLight.Length()) ++specularFrag;
#endif
        
    }
#ifdef __DEBUG__
    std::cout << "Specular Light " << specularFrag << '\n';
#endif
    return std::move(finalColor);
}