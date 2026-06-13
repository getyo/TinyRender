#pragma once
#include "RenderMath.h"
// 基本全局参数
//长度单位 cm
constexpr float nearClip = 30.f;
constexpr float farClip = 10000.0f;
constexpr float sightConeRad = RenderMath::PI / 3.0f; // 60度 FOV
constexpr float screenHeight = 1080.0f;
constexpr float screenWidth = 1920.0f;
constexpr float aspect = screenWidth / screenHeight;