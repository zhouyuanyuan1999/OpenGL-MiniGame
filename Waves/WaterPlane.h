#pragma once

// 
// WaterPlane.h   ---  Header file for WaterPlane.cpp.
// 
//   Sets up and renders 
//     - the ground plane, and
//   for the Math 155B project #2.
//
//

//
// Function Prototypes
//
void MySetupSurfaces();             // Called once, before rendering begins.
void RemeshFloor();                 // Remeshing the plane
void MyRenderSurfaces();            // Called to render the two surfaces
void RenderFloor();             // Rendering the plane
void setupLogs(int idx);
void RenderLogs(int idx);
void setupMissle();
void renderMissle();


