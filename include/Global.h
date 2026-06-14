#pragma once
#include "RenderMath.h"
// 基本全局参数
//长度单位 cm
constexpr float NearClip = 30.f;
constexpr float FarClip = 10000.0f;
constexpr float SightConeRad = RenderMath::PI / 3.0f; // 60度 FOV
constexpr float ScreenHeight = 1080.0f;
constexpr float ScreenWidth = 1920.0f;
constexpr float Aspect = ScreenWidth / ScreenHeight;