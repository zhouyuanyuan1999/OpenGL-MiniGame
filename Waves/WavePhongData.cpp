// ************************
// WavePhongData.cpp
// ************************

#define GLEW_STATIC
#include <GL/glew.h> 
#include <GLFW/glfw3.h>

#include "WavePhongData.h"
#include "EduPhong.h"
#include "LinearR4.h"
#include "GlGeomSphere.h"

GlGeomSphere mySphere; 
extern int modelviewMatLocation;       // Defined in GlslWaves.cpp

extern unsigned int vertPos_loc;
extern unsigned int vertNormal_loc;

extern phGlobal globalPhongData;

// It is suggested to use
//     myMaterial[0] for the water-like surface.
// myMaterials[1] and myMaterials[2] are unused.
phMaterial myMaterials[5];

// There are (up to) four lights.
// They are enabled/disabled by PhongProj.cpp code (already written)
// myLights[0], myLights[1], myLights[2] are the three lights above the scene.
// myLights[3] is the spotlight.
phLight myLights[4];

phMaterial myEmissiveMaterials;   // Use for small spheres showing the location of the lights.

// Suggested positions for the lights. It is OK to change them if it fits in your scene better.
// Especially, you may need to move them higher or lower!
VectorR3 myLightPositions[3] = {
    VectorR3(-5.0, 7.0, 0.0),
    VectorR3(0.0, 7.0, 0.0),
    VectorR3(5.0, 7.0, 0.0),
};

// You most likely to do not want to change MySetupGlobal
void MySetupGlobalLight()
{
    globalPhongData.NumLights = 4;     // Should be enough lights for the programming project #5

    globalPhongData.GlobalAmbientColor.Set(0.1, 0.1, 0.1);
    globalPhongData.LoadIntoShaders();
}

// Only called during initial set up.
void MySetupLights()
{
    mySphere.InitializeAttribLocations(vertPos_loc, vertNormal_loc);
    // First light (light #0).
    myLights[0].AmbientColor.Set(0.2, 0.2, 0.2);    // Gray color
    myLights[0].DiffuseColor.Set(0.6,0.6,0.6);      // Very light gray
    myLights[0].SpecularColor.Set(0.9, 0.9, 0.9);   // Very light gray
    myLights[0].IsEnabled = true;                   // Enable the light

     // Second light (light #1)
    myLights[1].AmbientColor.Set(0.2, 0.2, 0.2);    // Gray color
    myLights[1].DiffuseColor.Set(0.5, 0.5, 0.5);    // Gray
    myLights[1].SpecularColor.Set(0.7, 0.7, 0.7);   // White
    myLights[1].IsEnabled = true;                   // Enable the light

    // Third light (light #2)
    myLights[2].AmbientColor.Set(0.2, 0.2, 0.2);    // Gray color
    myLights[2].DiffuseColor.Set(0.5, 0.5, 0.5);    // Very light gray
    myLights[2].SpecularColor.Set(0.7, 0.7, 0.7);   // Very light gray
    myLights[2].IsEnabled = true;                   // Enable the light

    // The spotlight. Initially disabled.
    myLights[3].DiffuseColor.Set(0.6, 0.6, 0.6);    // Very light gray
    myLights[3].SpotCosCutoff = 0.95f;
    myLights[3].IsSpotLight = true;
    myLights[3].SpotExponent = 1.0;
    myLights[3].IsEnabled = false;                  // BE SURE TO ENABLE YOUR LIGHTS

}

void LoadAllLights() 
{
    myLights[0].SetPosition(viewMatrix, myLightPositions[0]);
    myLights[0].LoadIntoShaders(0); 

    myLights[1].SetPosition(viewMatrix, myLightPositions[1]);
    myLights[1].LoadIntoShaders(1);  

    myLights[2].SetPosition(viewMatrix, myLightPositions[2]);
    myLights[2].LoadIntoShaders(2);  

    myLights[3].SetPosition(viewMatrix, VectorR3(0.0, 6.0, 4.0));
    myLights[3].SetSpotlightDirection(viewMatrix, VectorR3(0.0, -1.0, -0.5));
    myLights[3].LoadIntoShaders(3);
}

// *******************************************
// In this routine, you must set the material properties for your water-like surface.
// Make the Emissive Color values ALL EQUAL TO ZERO.
// It is suggested that Ambient and Diffuse colors for a material are equal to each other, 
//         or at least are scalar multiples of each other.
// It is suggested that the Specular Color is a white or gray.
// *******************************************

void MySetupMaterials()
{

    // myMaterials[0]: Material for the water
    // ADJUST THIS COLOR TO LOOK BETTER
    myMaterials[0].AmbientColor.Set(0.1f, 0.1f, 0.2f);
    myMaterials[0].DiffuseColor.Set(0.5f, 0.5f, 0.9f);
    myMaterials[0].SpecularColor.Set(0.7f,0.7f,0.7f);
    myMaterials[0].SpecularExponent = 10.0;
    myMaterials[0].UseFresnel = true;
   
    myMaterials[1].AmbientColor.Set(1.0f, 0.0f, 0.0f);
    myMaterials[1].DiffuseColor.Set(0.5f, 0.5f, 0.9f);
    myMaterials[1].SpecularColor.Set(0.7f,0.7f,0.7f);
    myMaterials[1].SpecularExponent = 80.0;
   
    myMaterials[2].AmbientColor.Set(0.8f, 0.8f, 0.1f);
    myMaterials[2].DiffuseColor.Set(0.6f, 0.6f, 0.0f);
    myMaterials[2].SpecularColor.Set(0.7f,0.7f,0.7f);
    myMaterials[2].SpecularExponent = 80.0;
   
    myMaterials[3].AmbientColor.Set(0.8f, 0.2f, 0.0f);
    myMaterials[3].DiffuseColor.Set(0.8f, 0.2f, 0.0f);
    myMaterials[3].SpecularColor.Set(0.7f,0.7f,0.7f);
    myMaterials[3].SpecularExponent = 80.0;
}

// Purely emissive spheres showing placement of the light[0]
// Use the light's diffuse color as the emissive color
// Use the light's position as the sphere's position
void MyRenderSpheresForLights() {
   float matEntries[16];	// Holds 16 floats (since cannot load doubles into a shader that uses floats)
   phMaterial myEmissiveMaterial;

   for (int i = 0; i < 3; i++) {
        if (myLights[i].IsEnabled) {
            LinearMapR4 modelviewMat = viewMatrix;
            modelviewMat.Mult_glTranslate(myLightPositions[i].x, myLightPositions[i].y,myLightPositions[i].z);
            modelviewMat.Mult_glScale(0.2);
            modelviewMat.DumpByColumns(matEntries);
            glUniformMatrix4fv(modelviewMatLocation, 1, false, matEntries);
            myEmissiveMaterial.EmissiveColor = myLights[i].DiffuseColor;
            myEmissiveMaterial.LoadIntoShaders();
            mySphere.Render();
        }
    }
}
