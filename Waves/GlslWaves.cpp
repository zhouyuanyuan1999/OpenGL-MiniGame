/*
 * LetterProj.cpp - Version 0.3 - April 13, 2019
 *
 * Starting code for Math 155B, 2019 Project -- GLSL Waves,
 * 
 * Author: Sam Buss
 *
 * Software accompanying POSSIBLE SECOND EDITION TO the book
 *		3D Computer Graphics: A Mathematical Introduction with OpenGL,
 *		by S. Buss, Cambridge University Press, 2003.
 *
 * Software is "as-is" and carries no warranty.  It may be used without
 *   restriction, but if you modify it, please change the filenames to
 *   prevent confusion between different versions.
 * Bug reports: Sam Buss, sbuss@ucsd.edu.
 * Web page: http://math.ucsd.edu/~sbuss/MathCG2
 */

// These libraries are needed to link the program.
// First five are usually provided by the system.
#pragma comment(lib,"opengl32.lib")
#pragma comment(lib,"glu32.lib")
#pragma comment(lib,"glfw3.lib")
#pragma comment(lib,"glew32s.lib")
#pragma comment(lib,"glew32.lib")


// Use the static library (so glew32.dll is not needed):
#define GLEW_STATIC

#include <GL/glew.h> 
#include <GLFW/glfw3.h>

#include "LinearR3.h"		// Adjust path as needed.
#include "LinearR4.h"		// Adjust path as needed.
#include "EduPhong.h"
#include "WavePhongData.h"
#include "GlGeomSphere.h"
#include "GlShaderMgr.h"
#include "GlslWaves.h"
#include "WaterPlane.h"

// Enable standard input and output via printf(), etc.
// Put this include *after* the includes for glew and GLFW!
#include <stdio.h>




// ********************
// Animation controls and state infornation
// ********************

// These variables control the view direction.
//    The arrow keys are used to change these values.
double viewAzimuth = 0.25;	// Angle of view up/down (in radians)
double viewDirection = 0.0; // Rotation of view around y-axis (in radians)
double deltaAngle = 0.01;	// Change in view angle for each up/down/left/right arrow key press
LinearMapR4 viewMatrix;		// The current view matrix, based on viewAzimuth and viewDirection.

// Control Phong lighting modes
phGlobal globalPhongData;

// These two variables control how triangles are rendered.
bool wireframeMode = false;	// Equals true for polygon GL_FILL mode. False for polygon GL_LINE mode.
bool cullBackFaces = true;

// The next variable controls the resoluton of the meshes for the water plane.
int meshRes=60;
int logs = 5;
float water_scale = 10.0f;
float log_size = 0.5f;
float height_h = 0.6f;
float height_l = -1.0f;
float move_z = 0.0f;
float move_x = 0.0f;
bool fired = false;
float currentTime;

// Mode==0 for one wave. Mode==1 for one wave and smaller ripple.
int waveMode = 0;                               

// ************************
// General data helping with setting up VAO (Vertex Array Objects)
//    and Vertex Buffer Objects.
// ***********************

unsigned int myWaveShader;                  // The shader program!
int projMatLocation;						// Location of the projectionMatrix in the currently active shader program
int modelviewMatLocation;					// Location of the modelviewMatrix in the currently active shader program


// ATTENTION : THESE ARE USED FOR PROJECT #1 (GLSL WAVES)// These two locations are not in the supplied shader code.
// You will add them to your shader (GLSL) code as uniform inputs
int timeLocation;
int waveModeLocation;
int height_hLocation;
int height_lLocation;

//  The Projection matrix: Controls the "camera view/field-of-view" transformation
//     Generally is the same for all objects in the scene.
LinearMapR4 theProjectionMatrix;		//  The Projection matrix: Controls the "camera/view" transformation

// A ModelView matrix controls the placement of a particular object in 3-space.
//     It is generally different for each object.
// The array matEntries holds the matrix values as floats to be loaded into the shader program. 
float matEntries[16];		// Holds 16 floats (since cannot load doubles into a shader that uses floats)

// *****************************
// These variables set the dimensions of the perspective region we wish to view.
// They are used to help form the projection matrix and the view matrix
// All rendered objects lie in the rectangular prism centered on the z-axis
//     equal to (-Xmax,Xmax) x (-Ymax,Ymax) x (Zmin,Zmax)
// Be sure to leave some slack in these values, to allow for rotations, etc.
// The model/view matrix can be used to move objects to this position
// THESE VALUES MAY NEED AD-HOC ADJUSTMENT TO GET THE SCENE TO BE VISIBLE.
const double Xmax = 6.0;                // Control x dimensions of viewable scene
const double Ymax = 4.0;                // Control y dimensions of viewable scene
const double Zmin = -8.0, Zmax = 8.0;   // Control z dimensions of the viewable scene
// zNear equals the distance from the camera to the z = Zmax plane
const double zNear = 15.0;              // Make this value larger or smaller to affect field of view.


// *************************
// mySetupGeometries defines the scene data, especially vertex  positions and colors.
//    - It also loads all the data into the VAO's (Vertex Array Objects) and
//      into the VBO's (Vertex Buffer Objects).
// This routine is only called once to initialize the data.
// *************************
void mySetupGeometries() {
 
    MySetupSurfaces();

     mySetViewMatrix();

    check_for_opengl_errors();   // Really a great idea to check for errors -- esp. good for debugging!
}

void mySetViewMatrix() {
    // Set the view matrix. Sets view distance, and view direction.
    // The final translation is done because the ground plane lies in the xz-plane,
    // YOU MAY NEED TO ADJUST THE FINAL TRANSLATION.
    viewMatrix.Set_glTranslate(0.0, 0.0, -(Zmax + zNear));      // Translate to be in front of the camera
    viewMatrix.Mult_glRotate(viewAzimuth, 1.0, 0.0, 0.0);	    // Rotate viewAzimuth radians around x-axis
    viewMatrix.Mult_glRotate(-viewDirection, 0.0, 1.0, 0.0);    // Rotate -viewDirection radians around y-axis
    viewMatrix.Mult_glTranslate(0.0, -2.5, 0.0);                // Translate the view is above the water plane
}

// *************************************
// Main routine for rendering the scene
// myRenderScene() is called every time the scene needs to be redrawn.
// mySetupGeometries() has already created the vertex and buffer objects
//    and the model view matrices.
// The EduPhong shaders should already be setup.
// *************************************
void myRenderScene() {
    currentTime = (float)glfwGetTime();

    // Clear the rendering window
    static const float black[] = { 0.0f, 0.0f, 0.0f, 0.0f };
    const float clearDepth = 1.0f;
    glClearBufferfv(GL_COLOR, 0, black);
    glClearBufferfv(GL_DEPTH, 0, &clearDepth);	// Must pass in a *pointer* to the depth

    glUseProgram(myWaveShader);

    // Should really check if timeLocation and waveModeLocation are -1 before calling these,
    //    but it seems to be OK to just call them anyway.
    glUniform1f(timeLocation, currentTime);     // Set the current time uniform variable in the shader
    glUniform1i(waveModeLocation, waveMode);    // Set the uniform variable in the shader
    glUniform1f(height_hLocation, height_h);
    glUniform1f(height_lLocation, height_l);

    MyRenderSurfaces();
    MyRenderSpheresForLights();

    check_for_opengl_errors();   // Really a great idea to check for errors -- esp. good for debugging!
}

void my_setup_SceneData() {
	mySetupGeometries();


    // ATTENTION: You should add "LoadShaderSource" commands if you make more GLSL files. 
    // GlShaderMgr::LoadShaderSource("EduPhong.glsl");
//    GlShaderMgr::LoadShaderSource("MyWaves.glsl");
    GlShaderMgr::LoadShaderSource("MyWaves.glsl");

//    unsigned int shader_vert = GlShaderMgr::CompileShader("vertexShader_Waves");
//    unsigned int shader_frag = GlShaderMgr::CompileShader("fragmentShader_Waves", "calcPhongLighting", "applyTextureMap");
    
    unsigned int shader_vert = GlShaderMgr::CompileShader("vertexShader_Waves");
    unsigned int shader_frag = GlShaderMgr::CompileShader("fragmentShader_Waves", "calcPhongLighting", "applyTextureMap");
    unsigned int shader_geom = GlShaderMgr::CompileShader("GeometryShader_Waves");
   
    unsigned int shaders_progs[3] = { shader_vert, shader_geom, shader_frag };
    myWaveShader = GlShaderMgr::LinkShaderProgram(3, shaders_progs);
    phRegisterShaderProgram(myWaveShader);
    check_for_opengl_errors();

    projMatLocation = glGetUniformLocation(myWaveShader, phProjMatName);
    modelviewMatLocation = glGetUniformLocation(myWaveShader, phModelviewMatName);
    // Similar commands to the next line are used to get the locations 
    //    for Uniform variables that you add to the shader programs.
    // glGetUniformLocation returns "-1" if the variable is not in the shader program.
    timeLocation = glGetUniformLocation(myWaveShader, "curTime");
    waveModeLocation = glGetUniformLocation(myWaveShader, "waveMode");
    height_hLocation = glGetUniformLocation(myWaveShader, "height_h");
    height_lLocation = glGetUniformLocation(myWaveShader, "height_l");
    check_for_opengl_errors();

    MySetupGlobalLight();
    MySetupLights();
    LoadAllLights();
    MySetupMaterials();

    glfwSetTime(0.0);

	check_for_opengl_errors();   // Really a great idea to check for errors -- esp. good for debugging!
}

// *******************************************************
// Process all key press events.
// This routine is called each time a key is pressed or released.
// *******************************************************
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    static const double Pi = 3.1415926535f;
    if (action == GLFW_RELEASE) {
        return;			// Ignore key up (key release) events
    }
    bool viewChanged = false;
    switch (key) {
    case GLFW_KEY_ESCAPE:
        glfwSetWindowShouldClose(window, true);
        return;
    case '4':
    case '1':
    case '2':
    case '3':
    {
        phLight& theLight = myLights[key - '1'];
        theLight.IsEnabled = !theLight.IsEnabled;   // Toggle whether the light is enabled.
        LoadAllLights();
        return;
    }
    case 'B':
        waveMode = (waveMode + 1) % 2;
        return;
    case 'K':
         if (mods & GLFW_MOD_SHIFT) {
            logs = logs < 14 ? logs + 1 : 15;  // Uppercase 'H'
         }
         else {
            logs = logs > 4 ? logs - 1 : 3;    // Lowercase 'h'
         }
         MySetupSurfaces();
         return;
    case 'L':
         if (mods & GLFW_MOD_SHIFT) {
            height_l += 0.1f;
         }
         else{
            height_l -= 0.1f;
         }
         return;
    case 'H':
          if (mods & GLFW_MOD_SHIFT) {
            height_h += 0.1f;
          }
          else{
            height_h -= 0.1f;
          }
          return;
    case 'Q':		// Toggle wireframe mode
        if (wireframeMode) {
            wireframeMode = false;
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
        else {
            wireframeMode = true;
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
        return;
    case 'C':		// Toggle backface culling
        cullBackFaces = !cullBackFaces;     // Negate truth value of cullBackFaces
        if (cullBackFaces) {
            glEnable(GL_CULL_FACE);
        }
        else {
            glDisable(GL_CULL_FACE);
        }
        return;
    case 'M':
        if (mods & GLFW_MOD_SHIFT) {
            meshRes = meshRes < 100 ? meshRes + 1 : 101;  // Uppercase 'M'
        }
        else {
            meshRes = meshRes > 4 ? meshRes - 1 : 3;    // Lowercase 'm'
        }
        RemeshFloor();
        return;
    case GLFW_KEY_UP:
        viewAzimuth = Min(viewAzimuth + 0.01, PIfourths - 0.05);
        viewChanged = true;
        break;
    case GLFW_KEY_DOWN:
        viewAzimuth = Max(viewAzimuth - 0.01, -PIfourths + 0.05);
        viewChanged = true;
        break;
    case GLFW_KEY_RIGHT:
        viewDirection += 0.01;
        if (viewDirection > PI) {
            viewDirection -= PI2;
        }
        viewChanged = true;
        break;
    case GLFW_KEY_LEFT:
        viewDirection -= 0.01;
        if (viewDirection < -PI) {
            viewDirection += PI2;
        }
        viewChanged = true;
        break;
    case GLFW_KEY_D:
        move_z -= 0.2f;
        MySetupSurfaces();
        break;
    case GLFW_KEY_A:
        move_z += 0.2f;
        MySetupSurfaces();
        break;
    case GLFW_KEY_S:
        move_x -= 0.2f;
        MySetupSurfaces();
        break;
    case GLFW_KEY_W:
        move_x += 0.2f;
        MySetupSurfaces();
        break;
    case GLFW_KEY_E:
        globalPhongData.EnableEmissive = !globalPhongData.EnableEmissive;
        break;
    case GLFW_KEY_O:
        globalPhongData.EnableDiffuse = !globalPhongData.EnableDiffuse;
        break;
    case GLFW_KEY_U:
        globalPhongData.EnableSpecular = !globalPhongData.EnableSpecular;
        break;
    case GLFW_KEY_V:
        globalPhongData.LocalViewer = !globalPhongData.LocalViewer;
        break;
    case GLFW_KEY_F:
        if ( !fired ){
             fired = true;
        }
        MySetupSurfaces();
        break;
    }
    // Might have updated the global phong data above: upload it to the shader program.
    globalPhongData.LoadIntoShaders();

    if (viewChanged) {
        mySetViewMatrix();
        LoadAllLights();        // Have to call this since it affects the position of the lights!
    }
    else {
        // Updated the global phong data above: upload it to the shader program.
        globalPhongData.LoadIntoShaders();
    }
}


// *************************************************
// This function is called with the graphics window is first created,
//    and again whenever it is resized.
// The Projection View Matrix is typically set here.
//    But this program does not use any transformations or matrices.
// *************************************************
void window_size_callback(GLFWwindow* window, int width, int height) {
	// Define the portion of the window used for OpenGL rendering.
	glViewport(0, 0, width, height);

	// Setup the projection matrix as a perspective view.
	// The complication is that the aspect ratio of the window may not match the
	//		aspect ratio of the scene we want to view.
	double w = (width == 0) ? 1.0 : (double)width;
	double h = (height == 0) ? 1.0 : (double)height;
	double windowXmax, windowYmax;
    double aspectFactor = w * Ymax / (h * Xmax);   // == (w/h)/(Xmax/Ymax), ratio of aspect ratios
	if (aspectFactor>1) {
		windowXmax = Xmax * aspectFactor;
		windowYmax = Ymax;
	}
	else {
		windowYmax = Ymax / aspectFactor;
		windowXmax = Xmax;
	}

	// Using the max & min values for x & y & z that should be visible in the window,
	//		we set up the orthographic projection.
    double zFar = zNear + Zmax - Zmin;
	theProjectionMatrix.Set_glFrustum(-windowXmax, windowXmax, -windowYmax, windowYmax, zNear, zFar);

    if (glIsProgram(myWaveShader)) {
        glUseProgram(myWaveShader);
        theProjectionMatrix.DumpByColumns(matEntries);
        glUniformMatrix4fv(projMatLocation, 1, false, matEntries);
    }
    check_for_opengl_errors();   // Really a great idea to check for errors -- esp. good for debugging!
}

void my_setup_OpenGL() {
	
	glEnable(GL_DEPTH_TEST);	// Enable depth buffering
	glDepthFunc(GL_LEQUAL);		// Useful for multipass shaders

	// Set polygon drawing mode for front and back of each polygon
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    glEnable(GL_CULL_FACE);

	check_for_opengl_errors();   // Really a great idea to check for errors -- esp. good for debugging!
}

void error_callback(int error, const char* description)
{
	// Print error
	fputs(description, stderr);
}

void setup_callbacks(GLFWwindow* window) {
	// Set callback function for resizing the window
	glfwSetFramebufferSizeCallback(window, window_size_callback);

	// Set callback for key up/down/repeat events
	glfwSetKeyCallback(window, key_callback);

	// Set callbacks for mouse movement (cursor position) and mouse botton up/down events.
	// glfwSetCursorPosCallback(window, cursor_pos_callback);
	// glfwSetMouseButtonCallback(window, mouse_button_callback);
}

int main() {
	glfwSetErrorCallback(error_callback);	// Supposed to be called in event of errors. (doesn't work?)
	glfwInit();
    
#if defined(__APPLE__) || defined(__linux__)
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    //glfwWindowHint(GLFW_SAMPLES, 4);
    //glEnable(GL_MULTISAMPLE);

	const int initWidth = 800;
	const int initHeight = 600;
	GLFWwindow* window = glfwCreateWindow(initWidth, initHeight, "Phong Demo", NULL, NULL);
	if (window == NULL) {
		printf("Failed to create GLFW window!\n");
		return -1;
	}
	glfwMakeContextCurrent(window);

	if (GLEW_OK != glewInit()) {
		printf("Failed to initialize GLEW!.\n");
		return -1;
	}

	// Print info of GPU and supported OpenGL version
	printf("Renderer: %s\n", glGetString(GL_RENDERER));
	printf("OpenGL version supported %s\n", glGetString(GL_VERSION));
#ifdef GL_SHADING_LANGUAGE_VERSION
	printf("Supported GLSL version is %s.\n", (char *)glGetString(GL_SHADING_LANGUAGE_VERSION));
#endif
    printf("Using GLEW version %s.\n", glewGetString(GLEW_VERSION));

	printf("------------------------------\n");
    printf("Press arrow keys to adjust the view direction.\n   ");
    printf("Press 'B' (both) to toggle whether tide or no tide are generated.\n");
    printf("Press 'Q' (wireframe) to toggle whether wireframe or fill mode.\n");
    printf("Press 'M' (mesh) to increase the mesh resolution.\n");
    printf("Press 'm' (mesh) to decrease the mesh resolution.\n");
    printf("Press 'F'(faster) or 'f' (slower) to speed up or slow down the animation.\n");
    printf("Press '1', '2', '3' to toggle the three lights.\n");
    printf("Press '4' to toggle the spotlight.\n");
    printf("Press ESCAPE or 'X' or 'x' to exit.\n");
    printf("Press 'E' key (Emissive) to toggle rendering Emissive light.\n");
    printf("Press 'U' key (Specular) to toggle rendering Specular light.\n");
    printf("Press 'V' key (Viewer) to toggle using a local viewer.\n");
   
    printf("Wave Controls:");
    printf("Press 'h' and 'H' key to reduce or increase MAX wave heights.\n");
    printf("Press 'l' and 'L' key to reduce or increase MIN wave heights.\n");
   
    printf("Ship Controls:");
    printf("Press 'k' and 'K' key to reduce or add enemy ships.\n");
    printf("Press 'A' key to move ship farther(leftward).\n");
    printf("Press 'D' key to move ship nearer(rightward).\n");
    printf("Press 'W' key to move ship forward.\n");
    printf("Press 'S' key to move ship backward.\n");
    printf("Press 'F' key to fire a missile.\n");
   
    printf("Press ESCAPE to exit.\n");
	
    setup_callbacks(window);
   
	// Initialize OpenGL, the scene and the shaders
    my_setup_OpenGL();
	my_setup_SceneData();
 	window_size_callback(window, initWidth, initHeight);

    // Loop while program is not terminated.
	while (!glfwWindowShouldClose(window)) {
	
      MySetupSurfaces();
		myRenderScene();				// Render into the current buffer
		glfwSwapBuffers(window);		// Displays what was just rendered (using double buffering).

		// Poll events (key presses, mouse events)
		glfwWaitEventsTimeout(1.0/30.0);	    // Use this to animate at 60 frames/sec (timing is NOT reliable)
		// glfwWaitEvents();					// Or, Use this instead if no animation.
		// glfwPollEvents();					// Use this version when animating as fast as possible
	}

	glfwTerminate();
	return 0;
}

// If an error is found, it could have been caused by any command since the
//   previous call to check_for_opengl_errors()
// To find what generated the error, you can try adding more calls to
//   check_for_opengl_errors().
char errNames[8][36] = {
	"Unknown OpenGL error",
	"GL_INVALID_ENUM", "GL_INVALID_VALUE", "GL_INVALID_OPERATION",
	"GL_INVALID_FRAMEBUFFER_OPERATION", "GL_OUT_OF_MEMORY",
	"GL_STACK_UNDERFLOW", "GL_STACK_OVERFLOW" };
bool check_for_opengl_errors() {
	int numErrors = 0;
	GLenum err;
	while ((err = glGetError()) != GL_NO_ERROR) {
		numErrors++;
		int errNum = 0;
		switch (err) {
		case GL_INVALID_ENUM:
			errNum = 1;
			break;
		case GL_INVALID_VALUE:
			errNum = 2;
			break;
		case GL_INVALID_OPERATION:
			errNum = 3;
			break;
		case GL_INVALID_FRAMEBUFFER_OPERATION:
			errNum = 4;
			break;
		case GL_OUT_OF_MEMORY:
			errNum = 5;
			break;
		case GL_STACK_UNDERFLOW:
			errNum = 6;
			break;
		case GL_STACK_OVERFLOW:
			errNum = 7;
			break;
		}
		printf("OpenGL ERROR: %s.\n", errNames[errNum]);
	}
	return (numErrors != 0);
}
