//
//  WaterPlane.cpp
//
// Creates a flat water plane for Math 155B, Project #1
//


// Use the static library (so glew32.dll is not needed):
#define GLEW_STATIC
#include <GL/glew.h> 
#include <GLFW/glfw3.h>

#include "LinearR4.h"		// Adjust path as needed.

#include "WaterPlane.h"
#include "WavePhongData.h"

extern int meshRes;
extern int logs;
extern float water_scale;
extern float log_size;
extern float move_z;
extern float move_x;
extern int modelviewMatLocation;       // Defined in GlslWaves.cpp
extern bool check_for_opengl_errors();
extern bool fired;
extern float currentTime;
extern float height_h;
extern float height_l;

float fire_dir[3] = {-100.0f, 0.0, 0.0};
float fired_time = 0;
float fire_pos[3*3];
float hitted_boat[30] = {
   -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
};
float m_x_z[2];
float hit_dis = 1.5*log_size;
float missle_speed = water_scale/3.0f;
float max_fight_dis = water_scale;


unsigned int vertPos_loc = 0;    // Corresponds to "location = 0" in the verter shader definitions
unsigned int vertNormal_loc = 1; // Corresponds to "location = 1" in the verter shader definitions

// ************************
// General data helping with setting up VAO (Vertex Array Objects)
//    and Vertex Buffer Objects.
// ***********************

float rand_l[10] = {0.3844790298940486,
   0.636374901607918,
   0.8898181249081139,
   0.36248926974630635,
   0.5802744828917876,
   0.9597875533912519,
   0.8135028649952466,
   0.05335048698222111,
   0.20235230848952257,
   0.8662353041154222
};

const int MaxNumObjects = 20;
const int iFloor = 0;

unsigned int myVBO[MaxNumObjects];  // a Vertex Buffer Object holds an array of data
unsigned int myVAO[MaxNumObjects];  // a Vertex Array Object - holds info about an array of vertex data;
unsigned int myEBO[MaxNumObjects];  // a Element Array Buffer Object - holds an array of elements (vertex indices)

// **********************
// This sets up geometries needed for the water plane.
//  It is called only once.
// **********************
void MySetupSurfaces() {
    // Initialize the VAO's, VBO's and EBO's for the ground plane,
    // No data is loaded into the VBO's or EBO's until the "Remesh"
    //   routines are called.

    // For the floor:
    // Allocate the needed Vertex Array Objects (VAO's),
    //      Vertex Buffer Objects (VBO's) and Element Array Buffer Objects (EBO's)
   
    glGenVertexArrays(MaxNumObjects, &myVAO[0]);
    glGenBuffers(MaxNumObjects, &myVBO[0]);
    glGenBuffers(MaxNumObjects, &myEBO[0]);

    glBindVertexArray(myVAO[iFloor]);
    glBindBuffer(GL_ARRAY_BUFFER, myVBO[iFloor]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, myEBO[iFloor]);

    glVertexAttribPointer(vertPos_loc, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);	// Store vertices in the VBO
    glEnableVertexAttribArray(vertPos_loc);									// Enable the stored vertices
 
    // No data has been loaded into the VBO yet.
    // This is done next by the "Remesh" routine.

    RemeshFloor();
   
    for (int i = 0; i < logs; i++){
       if (hitted_boat[i] != 1 ){
          glBindVertexArray(myVAO[i+1]);
          glBindBuffer(GL_ARRAY_BUFFER, myVBO[i+1]);
          glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, myEBO[i+1]);
          
          glVertexAttribPointer(vertPos_loc, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);   // Store vertices in the VBO
          glEnableVertexAttribArray(vertPos_loc);                           // Enable the stored vertices
          
          setupLogs(i+1);
       }
    }
   
    if (fired) {
       glBindVertexArray(myVAO[logs+1]);
       glBindBuffer(GL_ARRAY_BUFFER, myVBO[logs+1]);
       glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, myEBO[logs+1]);
       
       glVertexAttribPointer(vertPos_loc, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);   // Store vertices in the VBO
       glEnableVertexAttribArray(vertPos_loc);                           // Enable the stored vertices
       
       if (fired){
          setupMissle();
       }
    }
    
    check_for_opengl_errors();      // Watch the console window for error messages!
}

void setupMissle() {
   if ( missle_speed*(currentTime-fired_time) > max_fight_dis ){
       printf("Ready to Fired Again!\n");
       fired = false;
       fired_time = 0;
    }
    float logVerts[3*3];
    for (int i=0;i<9;i++){
       logVerts[i] = fire_pos[i];
    }
    for (int i = 0; i < 3; i++){
       logVerts[i*3] += missle_speed*(currentTime-fired_time);
    }
    m_x_z[0] = logVerts[3];
    m_x_z[1] = logVerts[5];
    unsigned int ele[3] = { (unsigned int)0,(unsigned int)1,(unsigned int)2 };
   
    // Load data into the VBO and EBO using glBindBuffer and glBufferData commands
    glBindVertexArray(myVAO[logs+1]);
    glBindBuffer(GL_ARRAY_BUFFER, myVBO[logs+1]);
    glBufferData(GL_ARRAY_BUFFER, 3 * 3 * sizeof(float), &logVerts, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, myEBO[logs+1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3 * sizeof(unsigned int), &ele, GL_STATIC_DRAW);
}

void renderMissle() {
   glBindVertexArray(myVAO[logs+1]);
   
   // Set the uniform values (they are not stored with the VAO and thus must be set again everytime
   glVertexAttrib3f(vertNormal_loc, 0.0, 1.0, 0.0);    // Generic vertex attribute: Normal is (0,1,0) for the floor.
   myMaterials[3].LoadIntoShaders();
   float matEntries[16];
   viewMatrix.DumpByColumns(matEntries);
   glUniformMatrix4fv(modelviewMatLocation, 1, false, matEntries);
   glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, (void*)0);
   
}

void MyRenderSurfaces() {
    RenderFloor();
    for (int i = 0; i < logs; i++){
       if (hitted_boat[i] != 1){
          RenderLogs(i+1);
       }
    }
    if (fired) {
       renderMissle();
    }
    check_for_opengl_errors();      // Watch the console window for error messages!
}

void setupLogs( int idx){
    int numLogVerts = 3;
    int numLogElts = 3;
    //float* logVerts = new float[3 * numLogVerts];
    unsigned int* logElements = new unsigned int[numLogElts];
    float inv_l = (water_scale-2*log_size)*2.0f / (float)logs;
    float inv[2] = { -log_size -water_scale + idx*inv_l, -log_size -water_scale };
    float center[3] = { inv[0] + inv_l* (float)(rand()/RAND_MAX), 0.0f, inv[1] + inv_l*logs*rand_l[ (idx-1)%10 ] };
   
    float logVerts[3 * 3] = { center[0]-log_size, center[1], center[2]+log_size,
       center[0]+log_size, center[1], center[2]+log_size,
       center[0]-log_size, center[1], center[2]-log_size
    };
   
    if ( idx == 1 ){
       for (int i = 0; i < 3; i++){
          logVerts[i*3 + 2] += (-center[2] + move_z);
          logVerts[i*3] += move_x;
       }
       if (fired){
          if (fired_time == 0){
             fired_time = currentTime;
             for (int i = 0; i<9; i++){
                fire_pos[i] = logVerts[i];
             }
          }
       }
    }
    else{
       if (fired && (fired_time != 0)){
          float dx = logVerts[3] - m_x_z[0];
          float dz = logVerts[5] - m_x_z[1];
          if ( (dx*dx + dz*dz) <= hit_dis ){
             printf("Missle Hitted! \n");
             fired = false;
             fired_time = 0;
             m_x_z[0] = -100.0f;
             m_x_z[1] = -100.0f;
             hitted_boat[idx-1] = 1;
             printf("Ready to Fire Again! \n");
          }
       }
    }
   
    for (int i = 0; i < 3; i++){
       logElements[i] = (unsigned int)i;
    }
   
    // Load data into the VBO and EBO using glBindBuffer and glBufferData commands
    glBindVertexArray(myVAO[idx]);
    glBindBuffer(GL_ARRAY_BUFFER, myVBO[idx]);
    glBufferData(GL_ARRAY_BUFFER, 3 * numLogVerts * sizeof(float), &logVerts, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, myEBO[idx]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, numLogElts * sizeof(unsigned int), logElements, GL_STATIC_DRAW);
   
//    // Avoid a memory leak by deleting the arrays obtained with "new" above
//    delete[] logVerts;
    delete[] logElements;
}

void RenderLogs(int idx) {
   
   glBindVertexArray(myVAO[idx]);
   
   // Set the uniform values (they are not stored with the VAO and thus must be set again everytime
   glVertexAttrib3f(vertNormal_loc, 0.0, 1.0, 0.0);    // Generic vertex attribute: Normal is (0,1,0) for the floor.
   if (idx == 1) {
      myMaterials[2].LoadIntoShaders();
   }
   else{
      myMaterials[1].LoadIntoShaders();
   }
   float matEntries[16];
   viewMatrix.DumpByColumns(matEntries);
   glUniformMatrix4fv(modelviewMatLocation, 1, false, matEntries);
   glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, (void*)0);
}

// *********************************************
// A water plane gridded as an array of rectangles (triangulated)
// x and z values are in the range [-5,5].
// All points stored here with y value equal to zero.
// **** Heights are changed by the vertex shader.****
// *********************************************************

void RemeshFloor()
{
    // Floor (water plane) vertices.
    int numFloorVerts = (meshRes + 1)*(meshRes + 1);    
    float* floorVerts = new float[3 * numFloorVerts];
    // Floor elements (indices to vertices in a triangle strip)
    int numFloorElts = meshRes * 2 * (meshRes + 1);     
    unsigned int* floorElements = new unsigned int[numFloorElts];

    for (int i = 0; i <= meshRes; i++) {
        float z = (-water_scale*(float)(meshRes - i) + water_scale*(float)i) / (float)meshRes;
        for (int j = 0; j <= meshRes; j++) {
            float x = (-water_scale*(float)(meshRes - j) + water_scale*(float)j) / (float)meshRes;
            int vrtIdx = 3 * i * (meshRes + 1) + 3 * j;
            floorVerts[vrtIdx] = x;
            floorVerts[vrtIdx + 1] = 0.0f;
            floorVerts[vrtIdx + 2] = z;
        }
    }
    for (int i = 0; i < meshRes; i++) {
        for (int j = 0; j <= meshRes; j++) {
            int elt = 2 * i * (meshRes + 1) + 2 * j;
            floorElements[elt] = i * (meshRes + 1) + j;
            floorElements[elt + 1] = (i + 1) * (meshRes + 1) + j;
        }
    }

    // Load data into the VBO and EBO using glBindBuffer and glBufferData commands
    glBindVertexArray(myVAO[iFloor]);
    glBindBuffer(GL_ARRAY_BUFFER, myVBO[iFloor]);
    glBufferData(GL_ARRAY_BUFFER, 3 * numFloorVerts * sizeof(float), floorVerts, GL_STATIC_DRAW);  
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, myEBO[iFloor]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, numFloorElts * sizeof(unsigned int), floorElements, GL_STATIC_DRAW); 

    // Avoid a memory leak by deleting the arrays obtained with "new" above
    delete[] floorVerts;
    delete[] floorElements;
}

void RenderFloor() {

    glBindVertexArray(myVAO[iFloor]);

    // Set the uniform values (they are not stored with the VAO and thus must be set again everytime
    glVertexAttrib3f(vertNormal_loc, 0.0, 1.0, 0.0);    // Generic vertex attribute: Normal is (0,1,0) for the floor.
    myMaterials[0].LoadIntoShaders();                   // materials[0] defined in DemoPhongData.h
    float matEntries[16];
    viewMatrix.DumpByColumns(matEntries);
    glUniformMatrix4fv(modelviewMatLocation, 1, false, matEntries);

    // Draw the the triangle strips
    for (int i = 0; i < meshRes; i++) {
        glDrawElements(GL_TRIANGLE_STRIP, 2 * (meshRes + 1), GL_UNSIGNED_INT, 
            (void*)(i * 2* (meshRes + 1) * sizeof(unsigned int)));
    }
}
