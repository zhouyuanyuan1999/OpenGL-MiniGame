#pragma once
// ************************
// WavePhongData.h
// ************************


#include "EduPhong.h"
#include "LinearR4.h"

extern LinearMapR4 viewMatrix;          // Defined in GlslWaves.cpp

// It is suggested to use
//     myMaterial[0] for the water-like surface.
// myMaterials[1] and myMaterials[2] are unused.
extern phMaterial myMaterials[5];

// There are (up to) four lights.
// They are enabled/disabled by GlslWaves.cpp code (already written)
// myLights[0], myLights[1], myLights[2] are the three lights above the scene.
// myLights[3] is the spotlight.
extern phLight myLights[4];

void MySetupGlobalLight();
void MySetupLights();
void LoadAllLights();
void MySetupMaterials();
void MyRenderSpheresForLights();
