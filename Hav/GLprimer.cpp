/*
* Author: Erik Olsson (eriol726@student.liu.se) 2017
*/

#define _USE_MATH_DEFINES


#include <windows.h>
#include <GL/glew.h>
#include <iostream>
#include <math.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp> 
#include <glm/gtx/transform.hpp>
#include <glm/gtx/string_cast.hpp>

// GLFW 3.x, to handle the OpenGL window
#include <GL/glfw3.h>
#include "Utilities.hpp"
#include "Shader.hpp"
#include "Texture.hpp"
#include "TriangleSoup.hpp"
#include "controls.hpp"


#define HM_SIZE_X 4 // Dimensions of our heightmap
#define HM_SIZE_Y 4

GLFWwindow *window;    // GLFW struct to hold information about the window



glm::mat4 PVcalc(GLuint location_cameraPosition, Camera *camera) {

	glm::mat4 ProjectionMatrix = camera->getProjectionMatrix();
	glm::mat4 ViewMatrix = camera->getViewMatrix();

	glm::mat4 PV = ProjectionMatrix*ViewMatrix;


	return PV;
}

glm::mat4 CameraReflect(GLuint location_cameraPosition, Camera *camera) {




	glm::vec4 plane = glm::vec4(0.0f, 1.0f, 0.0f, 0.1f);



	float reflect[16] = {
		1 - 2 * plane.x*plane.x, -2 * plane.x*plane.y, -2 * plane.x*plane.z, -2 * plane.x*plane.w,
		-2 * plane.x*plane.y, 1 - 2 * plane.y*plane.y, -2 * plane.y*plane.z, -2 * plane.y*plane.w,
		-2 * plane.x*plane.z, -2 * plane.y*plane.z, 1 - 2 * plane.z*plane.z, -2 * plane.z*plane.w,
		0, 0, 0, 1
	};

	float flip[16] = {
		1, 0, 0, 0,
		0, -1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};

	glm::mat4 reflectMatrix = glm::make_mat4(reflect);
	glm::mat4 flipMatrix = glm::make_mat4(flip);
	glm::mat4 scale = glm::scale(glm::vec3(1.0f, -1.0f, 1.0f));

	glm::mat4 ViewMatrix = camera->getViewMatrix();
	glm::mat4 cameraFlip = ViewMatrix*reflectMatrix;

	glm::mat4 ProjectionMatrix = camera->getProjectionMatrix();
	glm::mat4 CameraReflect = ProjectionMatrix*cameraFlip;


	return CameraReflect;
}

void seaBottomMVP(GLuint location_terrainMVP, glm::mat4 PV, float heigt) {

	glm::mat4 scale2 = glm::scale(glm::vec3(5.0f, 5.0f, 5.0f));
	glm::mat4 trans2 = glm::translate(glm::vec3(0.0f, heigt, 0.0f));
	glm::mat4 rotationX = glm::rotate(90.0f, glm::vec3(1.0f, 0.0f, 0.0f));
	glm::mat4 ModelMatrix = trans2*rotationX*scale2;
	glm::mat4 MVP2 = PV * ModelMatrix;

	glUniformMatrix4fv(location_terrainMVP, 1, GL_FALSE, &MVP2[0][0]);
}

void shipMVP(GLuint location_shipMVP, glm::mat4 PV, float time) {

	glm::mat4 rot = glm::rotate(1.f, glm::vec3(1.0f, 0.0f, 0.0f));

	glm::mat4 trans = glm::translate(glm::vec3(-1.8f, 0.1f, sin(time) * 2));

	glm::mat4 scale = glm::scale(glm::vec3(0.3f, 0.3f, 0.3f));
	glm::mat4 ModelMatrix = trans*scale;
	glm::mat4 MVP = PV * ModelMatrix;

	glUniformMatrix4fv(location_shipMVP, 1, GL_FALSE, &MVP[0][0]);
}

void seaWaterMVP(GLuint location_seaMVP, GLuint location_seaM, Camera camera, glm::mat4 PV) {

	glm::mat4 scale = glm::scale(glm::vec3(1.0f, 1.0f, 1.0f));
	glm::mat4 trans = glm::translate(glm::vec3(0.0f, 0.1f, 0.0f));
	glm::mat4 rotationX = glm::rotate(90.0f, glm::vec3(1.0f, 0.0f, 0.0f));
	glm::mat4 ModelMatrix = trans*rotationX*scale;
	glm::mat4 MVP = PV * ModelMatrix;

	glm::mat4 ViewMatrix = camera.getViewMatrix();
	glUniformMatrix4fv(location_seaM, 1, GL_FALSE, &ModelMatrix[0][0]);
	glUniformMatrix4fv(location_seaMVP, 1, GL_FALSE, &MVP[0][0]);
}

void sphereMVP(GLuint location_reflectionMVP, GLuint location_reflectionM, glm::mat4 PV) {

	glm::mat4 scale = glm::scale(glm::vec3(0.5f, 0.5f, 0.5f));
	glm::mat4 trans = glm::translate(glm::vec3(0.0f, 1.5f, 0.0f));
	glm::mat4 rotate = glm::rotate(-90.0f, glm::vec3(1, 0, 0));
	glm::mat4 ModelMatrix = trans*scale*rotate;

	glm::mat4 MVP = PV * ModelMatrix;

	glUniformMatrix4fv(location_reflectionMVP, 1, GL_FALSE, &MVP[0][0]);
}

void bindTexture(GLenum GLTEXTURE, Texture texture, GLint location, int unit) {

	glActiveTexture(GLTEXTURE);
	glBindTexture(GL_TEXTURE_2D, texture.textureID);
	glUniform1i(location, unit);
	glDisable(GL_TEXTURE_2D);
}


int main(int argc, char *argv[]) {

	Shader waterShader, terrainShader;
	Texture dudvTexture, seaTexture, normalTexture, seaBottomTexture, objectTexture, sphereTexture;
	TriangleSoup seaBottomShape, seaShape, ship, sphere;
	float waveTime;
	float waveSpeed = 1.0;

	GLuint location_waveTime;
	GLuint location_normalMap;
	GLuint location_dudvMap;
	GLuint location_light;
	GLuint location_texID;

	GLuint location_seaMVP;
	GLuint location_seaM;
	GLuint location_reflectionMVP;
	GLuint location_reflectionM;
	GLuint location_cameraPosition;

	GLuint location_terrainMVP;
	GLuint location_seabottomTexture;
	GLuint location_terrainTexture;

	using namespace std;

	int width, height;
	const GLFWvidmode *vidmode;  // GLFW struct to hold information about the display

	glfwInit(); // Initialise GLFW

				// Determine the desktop size
	vidmode = glfwGetVideoMode(glfwGetPrimaryMonitor());

	// Make sure we are getting a GL context of at least version 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	// Exclude old legacy cruft from the context. We don't need it, and we don't want it.
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	// Open a square window (aspect 1:1) to fill half the screen height
	window = glfwCreateWindow(1024, 768, "GLprimer", NULL, NULL);
	if (!window)
	{
		cout << "Unable to open window. Terminating." << endl;
		glfwTerminate(); // No window was opened, so we can't continue in any useful way
		return -1;
	}

	// Make the newly created window the "current context" for OpenGL
	// (This step is strictly required, or things will simply not work)
	glfwMakeContextCurrent(window);


	// Hide the mouse and enable unlimited mouvement
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	// Set the mouse at the center of the screen
	glfwPollEvents();
	glfwSetCursorPos(window, 1024 / 2, 768 / 2);

	// Initialise GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		return -1;
	}

	//shaders----------------
	waterShader.createShader("waterVertex.glsl", "waterFragment.glsl");
	terrainShader.createShader("terrainVextex.glsl", "terrainFragment.glsl");

	Utilities::loadExtensions();

	// Show some useful information on the GL context
	cout << "GL vendor:       " << glGetString(GL_VENDOR) << endl;
	cout << "GL renderer:     " << glGetString(GL_RENDERER) << endl;
	cout << "GL version:      " << glGetString(GL_VERSION) << endl;
	cout << "Desktop size:    " << vidmode->width << "x" << vidmode->height << " pixels" << endl;

	//memeory for shader matrix
	location_seaMVP = glGetUniformLocation(waterShader.programID, "MVP");
	location_seaM = glGetUniformLocation(waterShader.programID, "M");
	location_waveTime = glGetUniformLocation(waterShader.programID, "waveTime");
	location_normalMap = glGetUniformLocation(waterShader.programID, "normalMap");
	location_dudvMap = glGetUniformLocation(waterShader.programID, "dudvMap");
	location_cameraPosition = glGetUniformLocation(waterShader.programID, "cameraPosition");
	location_light = glGetUniformLocation(waterShader.programID, "lightPosition");
	location_texID = glGetUniformLocation(waterShader.programID, "renderedTexture");
	location_seabottomTexture = glGetUniformLocation(waterShader.programID, "refractionTexture");

	location_terrainMVP = glGetUniformLocation(terrainShader.programID, "MVP");
	location_terrainTexture = glGetUniformLocation(terrainShader.programID, "Texture");

	dudvTexture.createTexture("texture/waterDUDV.tga");
	normalTexture.createTexture("texture/normalMap.tga");
	seaBottomTexture.createTexture("texture/sand_pebble2.tga");
	objectTexture.createTexture("texture/piratShip.tga");
	sphereTexture.createTexture("texture/earth.tga");

	ship.readOBJ("meshes/ship.obj");
	sphere.createSphere(1, 25);

	seaBottomShape.createTriangle();
	seaShape.createSea();

	// ---------------------------------------------
	// Render to Texture - specific code begins here
	// For FBOs I used the tutorial in: http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-14-render-to-texture/ 
	// ---------------------------------------------

	// The framebuffer, which regroups 0, 1, or more textures, and 0 or 1 depth buffer.
	GLuint FramebufferName = 0;
	glGenFramebuffers(1, &FramebufferName);
	glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);

	// The texture we're going to render to
	GLuint renderedTexture;
	glGenTextures(1, &renderedTexture);

	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, renderedTexture);

	// Give an empty image to OpenGL ( the last "0" means "empty" )
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1024, 768, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

	// Poor filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// The depth buffer
	GLuint depthrenderbuffer;
	glGenRenderbuffers(1, &depthrenderbuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, depthrenderbuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 1024, 768);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthrenderbuffer);

	// Set "renderedTexture" as our colour attachement #0
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, renderedTexture, 0);

	// Set the list of draw buffers.
	GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers

								   // Always check that our framebuffer is ok
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		return false;

	Camera *camera = new Camera();


	// Main loop
	while (!glfwWindowShouldClose(window))
	{
		// Did not work
		//double plane[] = { 0.0f, 1.0f, 0.0f, 0.1f };
		//glClipPlane(GL_CLIP_PLANE0, plane);
		//glEnable(GL_CLIP_DISTANCE0);

		glEnable(GL_DEPTH_TEST);
		glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);

		glUseProgram(terrainShader.programID);

		Utilities::displayFPS(window);
		// Get window size. It may start out different from the requested
		// size, and will change if the user resizes the window.
		glfwGetWindowSize(window, &width, &height);
		// Set viewport. This is the pixel rectangle we want to draw into.
		glViewport(0, 0, width, height); // The entire window

		glClearColor(0.3f, 0.3f, 0.3f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		float currentTime = (float)glfwGetTime();
		/* ---- Rendering code should go here ---- */

		glm::mat4  PV = PVcalc(location_cameraPosition, camera);

		seaBottomMVP(location_terrainMVP, PV, -2.0f);
		bindTexture(GL_TEXTURE3, seaBottomTexture, location_terrainTexture, 3);
		seaBottomShape.render();


		// ProjectionViewMatrix calculations
		PV = CameraReflect(location_cameraPosition, camera);

		// Draw reflections
		shipMVP(location_terrainMVP, PV, currentTime);
		bindTexture(GL_TEXTURE1, objectTexture, location_terrainTexture, 1);

		//camera movement does not work!
		float distance = 2 * (camera->getCameraPosition().y - 0.1f);// waters height
		camera->computeMatricesFromInputs(true, -distance);
		ship.render();

		sphereMVP(location_terrainMVP, location_terrainMVP, PV);
		bindTexture(GL_TEXTURE2, sphereTexture, location_terrainTexture, 2);
		sphere.render();

		glUseProgram(waterShader.programID);

		//positon for the sun
		glm::vec3 lightPos = glm::vec3(0, 15, -5);
		glUniform3fv(location_light, 1, &lightPos[0]);

		//passing campos till water shader
		glm::vec3 camPosition = camera->getCameraPosition();
		glUniform3fv(location_cameraPosition, 1, &camPosition[0]);

		bindTexture(GL_TEXTURE1, dudvTexture, location_dudvMap, 1);
		bindTexture(GL_TEXTURE2, normalTexture, location_normalMap, 2);
		bindTexture(GL_TEXTURE3, seaBottomTexture, location_seabottomTexture, 3);


		//Bind 0, which means render to back buffer, as a result, fb is unbound
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, renderedTexture);

		// Clear the screen
		glClearColor(0.3f, 0.3f, 0.3f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Render to the screen

		//render sea
		seaWaterMVP(location_seaMVP, location_seaM, *camera, PV);
		seaShape.renderSea();

		glUseProgram(terrainShader.programID);
		PV = PVcalc(location_cameraPosition, camera);

		//render reality ship
		shipMVP(location_terrainMVP, PV, currentTime);
		bindTexture(GL_TEXTURE1, objectTexture, location_terrainTexture, 1);
		ship.render();

		//render reality sphere
		sphereMVP(location_terrainMVP, location_terrainMVP, PV);
		bindTexture(GL_TEXTURE2, sphereTexture, location_terrainTexture, 2);
		sphere.render();

		//render seaBottom
		seaBottomMVP(location_terrainMVP, PV, -2.0f);
		bindTexture(GL_TEXTURE3, seaBottomTexture, location_terrainTexture, 3);
		seaBottomShape.render();

		//glEnable(GL_BLEND);
		//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// Update the uniform waveTime variable.
		if (location_waveTime != -1) {
			glUseProgram(waterShader.programID);
			waveTime = (float)glfwGetTime();
			waveTime *= waveSpeed;
			glUniform1f(location_waveTime, waveTime);

		}

		// Swap buffers, i.e. display the image and prepare for next frame.
		glfwSwapBuffers(window);

		// Poll events (read keyboard and mouse input)
		glfwPollEvents();

		// Exit if the ESC key is pressed (and also if the window is closed).
		if (glfwGetKey(window, GLFW_KEY_ESCAPE)) {
			glfwSetWindowShouldClose(window, GL_TRUE);
		}
	}

	// Cleanup  shader and textures
	glDeleteProgram(waterShader.programID);
	glDeleteProgram(terrainShader.programID);
	glDeleteTextures(3, &location_seabottomTexture);
	glDeleteTextures(1, &location_normalMap);
	glDeleteTextures(5, &location_terrainTexture);
	glDeleteTextures(0, &location_dudvMap);

	//Delete resources
	glDeleteTextures(1, &renderedTexture);
	glDeleteRenderbuffersEXT(1, &depthrenderbuffer);
	glDeleteFramebuffersEXT(1, &FramebufferName);

	// Close the OpenGL window and terminate GLFW.
	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}


