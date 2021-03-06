// Adapted from the online tuturials by Joey de Vries found at 'http://learnopengl.com/'
// Also from Anton's OpenGL 4 Tutorials https://capnramses.itch.io/antons-opengl-4-tutorials


/*
     *
         *
	              *
    Cloth Animation

	*                     *
	        *
*/


#pragma region "includes"

#include <iostream> // Std. Includes
#include <string>
#include <algorithm>
#include <vector>
using namespace std;
#define GLEW_STATIC // GLEW
#include <GL/glew.h> 
#include <GLFW/glfw3.h> // GLFW
#include <glm/glm.hpp> // GLM Mathematics
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/orthonormalize.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <SOIL/SOIL.h> // SOIL

//Rendering
#include "Shader.h" 
#include "Camera.h"
#include "Model.h"

//Physics
#include "ClothSystem.h"
#include "RigidBody.h"
#include "BookKeeping.h"

#include "Cloth.h"


#define PI 3.141592653589793238462643383279502
#define timestep 0.01

#pragma endregion




#pragma region "Function Prototypes and Variables"

//Function prototypes
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void Do_Movement();

//Texture stuff
GLuint loadCubemap(vector<const GLchar*> faces);
GLuint loadTexture(GLchar* path);

//Drawing
void RenderQuad();










// Camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
bool keys[1024];
GLfloat lastX = 400, lastY = 300;
bool firstMouse = true;

bool playSimulation = false;
bool reverseGrav = false;
bool applyRotationMatrix= false;
bool applyRotationMatrixY = false;
bool applyRotationMatrixZ = false;
bool drawBody = false;

bool checkSomething = false;
bool movePos = false;
bool playForces = false;
bool rotateRT = true;
bool boundBox = false;


bool applyVec = false;
//bool applyVec2 = false;
bool applyForce = false;
bool reset = false;


GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;
GLuint screenWidth = 1600, screenHeight = 900; // Properties


//float timestep = 0.01f;
float epsilon = 0.000001f;
float ninety = PI / 2;
float ninetyfive = PI / 1.9f;


int i, j;
int clock;
int place = 0;

GLfloat narrowPhasePlaneThreshold = 0.000000001f; //0.01f;
GLfloat coeffRest = 0.45;

glm::vec3 lightPos(5.0f, 0.0f, 10.0f);


glm::vec3 planeNormal(0.0f, 1.0f, 0.0f);
glm::vec3 plane2Normal = glm::rotate(planeNormal, ninetyfive, glm::vec3(1.0f, 0.0f, 0.0f));

glm::vec3 planePos(0.0f, -10.0f, 0.0f);
GLfloat planeThreshold = 0.0f;


glm::vec3 spherePos(3.0f, -5.0f, -5.0f);

glm::vec3 posVec(0.0f, 0.0f, 0.0f);
glm::vec3 velVecX(0.0f, 0.0f, 0.0f);
glm::vec3 velVecY(0.0f, -0.01f, 0.0f);
glm::vec3 velVecZ(0.0f, 0.0f, 0.0f);

glm::vec3 angVec(0.5f, 0.0f, 0.0f);

glm::vec3 forceVec(0.0f, 0.0f, 0.0f);
glm::vec3 torqueVec(0.0f, 0.0f, 0.0f);



glm::vec3 Jforce(0.0f, 0.0f, 0.0f);




#pragma endregion



// Start our application and run our Game loop
int main()
{

#pragma region "Initialisation"

	// Init GLFW
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "Real-Time Physics", nullptr, nullptr); // Windowed
	glfwMakeContextCurrent(window);

	// Set the required callback functions
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// Input Options
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Initialize GLEW to setup the OpenGL Function pointers
	glewExperimental = GL_TRUE;
	glewInit();

	// Define the viewport dimensions, as above
	glViewport(0, 0, screenWidth, screenHeight);

#pragma endregion

	// Setup some OpenGL options
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);



#pragma region "Shaders"



	// Setup and compile our shaders
	Shader skyboxShader("U:/Physics/FinalProject/Stuff/Shaders/skybox/skybox.vert", "U:/Physics/FinalProject/Stuff/Shaders/skybox/skybox.frag");

	Shader giveColourShader("U:/Physics/FinalProject/Stuff/Shaders/givecolour.vert", "U:/Physics/FinalProject/Stuff/Shaders/givecolour.frag");
	
	Shader whiteShader("U:/Physics/FinalProject/Stuff/Shaders/plaincolour/white.vert", "U:/Physics/FinalProject/Stuff/Shaders/plaincolour/white.frag");

	Shader drawShader("U:/Physics/Phys4/Stuff/Shaders/plaincolour/blue.vert", "U:/Physics/Phys4/Stuff/Shaders/plaincolour/blue.frag");
	
	Shader boundboxShader("U:/Physics/FinalProject/Stuff/Shaders/plaincolour/boundbox.vert", "U:/Physics/FinalProject/Stuff/Shaders/plaincolour/boundbox.frag");

	Shader phongShader("U:/Physics/FinalProject/Stuff/Shaders/Lighting/phong.vert", "U:/Physics/FinalProject/Stuff/Shaders/Lighting/phong.frag");

#pragma endregion



#pragma region "Models and Textures"

	// Load models
	Model cube("U:/Physics/FinalProject/Stuff/Models/cube.obj");
	Model clothM("U:/Physics/FinalProject/Stuff/Models/clothModel.obj");

	Model sphere("U:/Physics/FinalProject/Stuff/Models/sphere/sphere.obj");

	Model largeSphere("U:/Physics/FinalProject/Stuff/Models/sphere/largesphere.obj");

	Model quadModel("U:/Physics/FinalProject/Stuff/Models/quadModel.obj");

	// Load textures
	//GLuint painting = loadTexture("U:/Rendering/RTRAssignment_2/Textures/desk.jpg");

#pragma endregion	



#pragma region "object_initialization, skybox"
	// Set the object data (buffers, vertex attributes)
	GLfloat cubeVertices[] = {
		// Positions          // Normals
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
		0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,

		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

		0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
		0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
		0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
		0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
		0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
		0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
		0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
	};
	GLfloat skyboxVertices[] = {
		// Positions          
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		1.0f,  1.0f, -1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		1.0f, -1.0f,  1.0f
	};

	// Setup cube VAO
	GLuint cubeVAO, cubeVBO;
	glGenVertexArrays(1, &cubeVAO);
	glGenBuffers(1, &cubeVBO);
	glBindVertexArray(cubeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glBindVertexArray(0);
	// Setup skybox VAO
	GLuint skyboxVAO, skyboxVBO;
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glBindVertexArray(0);


	


	// Cubemap (Skybox)
	//loads the cubemap 'faces' into a vector and loads them into a cubemap format in skyboxTexture
	vector<const GLchar*> faces;
	faces.push_back("U:/Physics/VanyaOpenGL/Stuff/skybox/clouds/stormydays_ft.png");
	faces.push_back("U:/Physics/VanyaOpenGL/Stuff/skybox/clouds/stormydays_bk.png");
	faces.push_back("U:/Physics/VanyaOpenGL/Stuff/skybox/clouds/stormydays_up.png"); 
	faces.push_back("U:/Physics/VanyaOpenGL/Stuff/skybox/clouds/stormydays_dn.png"); 
	faces.push_back("U:/Physics/VanyaOpenGL/Stuff/skybox/clouds/stormydays_rt.png");
	faces.push_back("U:/Physics/VanyaOpenGL/Stuff/skybox/clouds/stormydays_lf.png");
	GLuint skyboxTexture = loadCubemap(faces);
	
#pragma endregion



	bool structuralSprings = 1;
	bool shearSprings = 1;
	bool flexionSprings = 1;

	//Cloth implementation
	Cloth cloth(10, 10, 0.01f, 14, 14, structuralSprings, shearSprings, flexionSprings, 2);
	cloth.BuildAABBVH(&cloth.rootNode, cloth.clothTriangles, 2);


	// Rigid Body
	BroadPhaseCollisionDetection bpcd;
	bpcd.initialise();

	bpcd.bodies[0].position = glm::vec3(0.0f, -9.5f, 8.0f); //Far
	bpcd.bodies[1].position = glm::vec3(6.0f, -9.0f, 8.0f);
	bpcd.bodies[2].position = glm::vec3(3.0f, -8.0f, 13.0f);

	bpcd.bodies[0].position = glm::vec3(0.0f, -9.5f, 4.0f); //Close
	bpcd.bodies[1].position = glm::vec3(6.0f, -9.0f, 4.0f);
	bpcd.bodies[2].position = glm::vec3(3.0f, -8.0f, 6.5f);


	for (int i = 0; i < bpcd.bodies.size(); i++)
	{
		bpcd.bodies[i].Update(timestep);
	}

	// A test particle
	Particle parti(glm::vec3(4.0f, -5.0f, 5.0f), 1.0f);
	
	glm::vec3 moveVec(0.0f, 0.2f, 0.0f);

	//Particle parti(bpcd.bodies[0].modelVertices[4], 1.0f);
	Particle parti1(bpcd.bodies[0].modelVertices[5], 1.0f);
	Particle parti2(bpcd.bodies[0].modelVertices[6], 1.0f);
	Particle parti3(bpcd.bodies[0].modelVertices[7], 1.0f);

	Particle parti10(bpcd.bodies[1].modelVertices[4], 1.0f);
	Particle parti11(bpcd.bodies[1].modelVertices[5], 1.0f);
	Particle parti12(bpcd.bodies[1].modelVertices[6], 1.0f);
	Particle parti13(bpcd.bodies[1].modelVertices[7], 1.0f);


	Particle parti20(bpcd.bodies[2].modelVertices[4], 1.0f);
	Particle parti21(bpcd.bodies[2].modelVertices[5], 1.0f);
	Particle parti22(bpcd.bodies[2].modelVertices[6], 1.0f);
	Particle parti23(bpcd.bodies[2].modelVertices[7], 1.0f);

	//parti.applyForce(glm::vec3(0.0f, 0.0f, -700.0f), timestep);

	std::vector<Particle> partis;
	partis.push_back(parti);// partis.push_back(parti1), partis.push_back(parti2), partis.push_back(parti3);
	//partis.push_back(parti10), partis.push_back(parti11), partis.push_back(parti12), partis.push_back(parti13);
	//partis.push_back(parti20), partis.push_back(parti21), partis.push_back(parti22), partis.push_back(parti23);

	for (int i = 0; i < partis.size(); i++)
		partis[i].isPinned = true;





	// Game loop
	while (!glfwWindowShouldClose(window))
	{
		// Set frame time
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		// Check and call events
		glfwPollEvents();
		Do_Movement();
		// Clear buffers
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		clock++;
		

		// Draw scene, create transformations
		glm::mat4 model, outerModel, boundBoxModel;
		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 projection = glm::perspective(camera.Zoom, (float)screenWidth / (float)screenHeight, 0.1f, 100.0f);
		
		
		whiteShader.Use();
		//glUniform3f(glGetUniformLocation(whiteShader.Program, "cameraPos"), camera.Position.x, camera.Position.y, camera.Position.z);
		glUniformMatrix4fv(glGetUniformLocation(whiteShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(whiteShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));






		float dampingConstant = 0.01f;
		int constraintIterations = 2;
		int springIterations = 1;

		if (playSimulation)
		{
			cloth.Update(dampingConstant, constraintIterations, springIterations, timestep);

			// Particle
			parti.verletIntegration(dampingConstant, timestep);
		}
		

#pragma region DRAWING OBJECTS

		std::vector<Particle>::iterator particle;
		for (particle = cloth.particles.begin(); particle != cloth.particles.end(); particle++)
		{
			float partX = (*particle).position[0];
			float partY = (*particle).position[1];
			float partZ = (*particle).position[2];


			glUniform3f(glGetUniformLocation(whiteShader.Program, "passedColour"), (*particle).colour[0], (*particle).colour[1], (*particle).colour[2]);

			// DRAW the particle
			model = glm::mat4();
			//Translate to the particles position
			model = glm::translate(model, glm::vec3(partX, partY, partZ));
			glUniformMatrix4fv(glGetUniformLocation(whiteShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
			//quadModel.Draw(whiteShader);
			//sphere.Draw(whiteShader);
		}


		//Collision particle(s)
		for (int i = 0; i < partis.size(); i++)
		{
			// DRAW Sphere for the collision particle
			model = glm::mat4();
			//Translate to the particles position
			model = glm::translate(model, partis[i].position);
			glUniform3f(glGetUniformLocation(whiteShader.Program, "passedColour"), partis[i].colour[0], partis[i].colour[1], partis[i].colour[2]);
			glUniformMatrix4fv(glGetUniformLocation(whiteShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
			sphere.Draw(whiteShader);
		}
		
		// DRAW Sphere for the collision particle
		model = glm::mat4();
		//Translate to the particles position
		model = glm::translate(model, parti.position);
		glUniform3f(glGetUniformLocation(whiteShader.Program, "passedColour"), parti.colour[0], parti.colour[1], parti.colour[2]);
		glUniformMatrix4fv(glGetUniformLocation(whiteShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
		//sphere.Draw(whiteShader);



		//if (boundBox) // Press B
		//{
		//	for (int i = 0; i < cloth.clothTriangles.size(); i++)
		//	{
		//		cloth.clothTriangles[i].GetBoundingBox();

		//		//Draw bounding box
		//		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		//		boundboxShader.Use();
		//		glUniform3f(glGetUniformLocation(boundboxShader.Program, "cameraPos"), camera.Position.x, camera.Position.y, camera.Position.z);
		//		glUniformMatrix4fv(glGetUniformLocation(boundboxShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
		//		glUniformMatrix4fv(glGetUniformLocation(boundboxShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		//		model = glm::mat4();
		//		model = glm::translate(model, cloth.clothTriangles[i].getTriangleUpperCenter());
		//		model = glm::scale(model, glm::vec3(cloth.clothTriangles[i].xDist, cloth.clothTriangles[i].yDist, cloth.clothTriangles[i].zDist));
		//		glUniformMatrix4fv(glGetUniformLocation(boundboxShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
		//		glUniform4f(glGetUniformLocation(boundboxShader.Program, "boundBoxColour"), 1.0f, 0.0f, 0.0f, 1.0f);
		//		cube.Draw(boundboxShader);
		//	}
		//}
		

		
		//For drawing the box of a given node
		/*glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		boundboxShader.Use();
		glUniform3f(glGetUniformLocation(boundboxShader.Program, "cameraPos"), camera.Position.x, camera.Position.y, camera.Position.z);
		glUniformMatrix4fv(glGetUniformLocation(boundboxShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(boundboxShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		model = glm::mat4();
		model = glm::translate(model, cloth.ClothNodes[place]->boundingVolume.centerPos);
		model = glm::scale(model, glm::vec3(cloth.ClothNodes[place]->boundingVolume.xDist, cloth.ClothNodes[place]->boundingVolume.yDist, cloth.ClothNodes[place]->boundingVolume.zDist));
		glUniformMatrix4fv(glGetUniformLocation(boundboxShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glUniform4f(glGetUniformLocation(boundboxShader.Program, "boundBoxColour"), 1.0f, 0.0f, 0.0f, 1.0f);
		cube.Draw(boundboxShader);*/



		////Draw the bodies
		//for (int i = 0; i < bpcd.bodies.size(); i++)
		//{
		//	//if (drawBody)
		//	//{
		//		//Draw inner box
		//		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		//		drawShader.Use();
		//		model = glm::mat4();
		//		glUniform3f(glGetUniformLocation(drawShader.Program, "cameraPos"), camera.Position.x, camera.Position.y, camera.Position.z);
		//		glUniformMatrix4fv(glGetUniformLocation(drawShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
		//		glUniformMatrix4fv(glGetUniformLocation(drawShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		//		glUniformMatrix4fv(glGetUniformLocation(drawShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
		//		bpcd.bodies[i].model.Draw(drawShader);


		//		//Draw line box
		//		whiteShader.Use();
		//		glUniform3f(glGetUniformLocation(whiteShader.Program, "cameraPos"), camera.Position.x, camera.Position.y, camera.Position.z);
		//		glUniformMatrix4fv(glGetUniformLocation(whiteShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
		//		glUniformMatrix4fv(glGetUniformLocation(whiteShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		//		model = glm::mat4();
		//		model = glm::scale(model, glm::vec3(1.001f, 1.001f, 1.001f));
		//		glUniformMatrix4fv(glGetUniformLocation(whiteShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
		//		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		//		bpcd.bodies[i].model.Draw(whiteShader);
		//	//}
		//}

		//Draw a big collision sphere
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		phongShader.Use();
		glUniform3f(glGetUniformLocation(phongShader.Program, "objectColor"), 1.0f, 1.0f, 1.0f);
		glUniform3f(glGetUniformLocation(phongShader.Program, "lightColor"), 1.0f, 1.0f, 1.0f);
		glUniform3f(glGetUniformLocation(phongShader.Program, "lightPos"), lightPos.x, lightPos.y, lightPos.z);
		glUniform3f(glGetUniformLocation(phongShader.Program, "viewPos"), camera.Position.x, camera.Position.y, camera.Position.z);
		glUniformMatrix4fv(glGetUniformLocation(phongShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(phongShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		model = glm::mat4();
		model = glm::translate(model, spherePos);
		glUniformMatrix4fv(glGetUniformLocation(phongShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
		//largeSphere.Draw(phongShader);



#pragma endregion


		if (1 == 1)
		{
		//Prob a bit expensive, but seems to work ok!
#pragma region DRAW CLOTH WELL

		phongShader.Use();
		glUniform3f(glGetUniformLocation(phongShader.Program, "objectColor"), 1.0f, 0.5f, 0.31f);
		glUniform3f(glGetUniformLocation(phongShader.Program, "lightColor"), 1.0f, 1.0f, 1.0f);
		glUniform3f(glGetUniformLocation(phongShader.Program, "lightPos"), lightPos.x, lightPos.y, lightPos.z);
		glUniform3f(glGetUniformLocation(phongShader.Program, "viewPos"), camera.Position.x, camera.Position.y, camera.Position.z);
		glUniformMatrix4fv(glGetUniformLocation(phongShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(phongShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

		for (int i = 0; i < cloth.clothTriangles.size(); i++)
		{
			

			GLfloat x1, y1, z1, x2, y2, z2, x3, y3, z3;

			x1 = cloth.clothTriangles[i].p1->position[0];
			y1 = cloth.clothTriangles[i].p1->position[1];
			z1 = cloth.clothTriangles[i].p1->position[2];
			x2 = cloth.clothTriangles[i].p2->position[0];
			y2 = cloth.clothTriangles[i].p2->position[1];
			z2 = cloth.clothTriangles[i].p2->position[2];
			x3 = cloth.clothTriangles[i].p3->position[0];
			y3 = cloth.clothTriangles[i].p3->position[1];
			z3 = cloth.clothTriangles[i].p3->position[2];


			// Set up vertex data (and buffer(s)) and attribute pointers
			GLfloat vertices[] = {
				x1, y1, z1, // Left  
				x2, y2, z2, // Right 
				x3, y3, z3  // Top   
			};

			GLuint VBO, VAO;
			glGenVertexArrays(1, &VAO);
			glGenBuffers(1, &VBO);
			// Bind the Vertex Array Object first, then bind and set vertex buffer(s) and attribute pointer(s).
			glBindVertexArray(VAO);
			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
			glEnableVertexAttribArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, 0); // Note that this is allowed, the call to glVertexAttribPointer registered VBO as the currently bound vertex buffer object so afterwards we can safely unbind

			model = glm::mat4();
			glUniformMatrix4fv(glGetUniformLocation(phongShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
			glm::vec3 col = cloth.clothTriangles[i].p1->colour;
			glUniform3f(glGetUniformLocation(phongShader.Program, "objectColor"), col[0], col[1], col[2]);

			glDrawArrays(GL_TRIANGLES, 0, 3);
			glBindVertexArray(0); // Unbind VAO (it's always a good thing to unbind any buffer/array to prevent strange bugs)
		}

#pragma endregion
		}

#pragma region DRAW PARTICLE LINKS

		if (1 == 2)
		{


		giveColourShader.Use();
		glUniformMatrix4fv(glGetUniformLocation(giveColourShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(giveColourShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));



		for (int i = 0; i < cloth.particleConstraints.size(); i++)
		{
			GLfloat x1, y1, z1, x2, y2, z2;

			x1 = cloth.particleConstraints[i].p1->position[0];
			y1 = cloth.particleConstraints[i].p1->position[1];
			z1 = cloth.particleConstraints[i].p1->position[2];
			x2 = cloth.particleConstraints[i].p2->position[0];
			y2 = cloth.particleConstraints[i].p2->position[1];
			z2 = cloth.particleConstraints[i].p2->position[2];


			GLfloat lineVertices[] =
			{
				x1, y1, z1,
				x2, y2, z2
			};

			GLuint lVBO, lVAO;
			glGenVertexArrays(1, &lVAO);
			glGenBuffers(1, &lVBO);
			glBindVertexArray(lVAO);
			glBindBuffer(GL_ARRAY_BUFFER, lVBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(lineVertices), lineVertices, GL_STATIC_DRAW);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
			glEnableVertexAttribArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindVertexArray(0);

			model = glm::mat4();
			glUniformMatrix4fv(glGetUniformLocation(giveColourShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));

			/*if (cloth.particleConstraints[i].constraintType == 0)
			{
				glUniform4f(glGetUniformLocation(giveColourShader.Program, "ourColor"), 1.0f, 0.0f, 0.0f, 1.0f);
			}*/
			/*if (cloth.particleConstraints[i].constraintType == 1)
			{
			glUniform4f(glGetUniformLocation(giveColourShader.Program, "ourcolor"), 0.0f, 1.0f, 0.0f, 1.0f);
			}*/
			/*if (cloth.particleConstraints[i].constraintType == 2)
			{
			glUniform4f(glGetUniformLocation(giveColourShader.Program, "ourColor"), 0.0f, 0.0f, 1.0f, 1.0f);
			}*/
			/*else
			glUniform4f(glGetUniformLocation(giveColourShader.Program, "ourColor"), 1.0f, 1.0f, 1.0f, 1.0f);*/


			glBindVertexArray(lVAO);
			glDrawArrays(GL_LINES, 0, 2);
			glBindVertexArray(0);
		}

#pragma endregion
		}


#pragma region BOUNDING VOLUME HIERARCHY

		//Currently rebuilding each frame, not too slow but not ideal @TODO
		cloth.ClearNodes();
		//cloth.BuildAABBVH(&cloth.rootNode, cloth.clothTriangles, 2);

#pragma endregion	
		
		


#pragma region CLOTH FORCES + COLLISION



		glm::vec3 grav(0.0f, -09.811f, 0.0f);
		cloth.addForce(grav, timestep);

		float wind1 = 1.0f * sin(glfwGetTime()); // 0.5f;
		glm::vec3 wind(wind1, 0, 0.2);
		cloth.applyWindForce(wind, timestep);


		// go through the particles, test against the root node BB of the cloth mesh
		for (int i = 0; i < partis.size(); i++)
		{
			//cloth.CheckBVH(&cloth.rootNode, &partis[i], timestep);
		}



		//Brute force plane check
     	cloth.bruteForceParticlePlaneCollisionCheck(planeNormal, planePos);

		//Sphere collision check
		//cloth.CheckCollisionWithSphere(spherePos, 2.3f);

#pragma endregion



		
		






		
		




	

		//
		//Renders a quad 'infinite plane'
		//
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		phongShader.Use();
		glUniform3f(glGetUniformLocation(phongShader.Program, "objectColor"), 0.4f, 0.6f, 0.8f);
		glUniform3f(glGetUniformLocation(phongShader.Program, "lightColor"), 1.0f, 1.0f, 1.0f);
		glUniform3f(glGetUniformLocation(phongShader.Program, "lightPos"), lightPos.x, lightPos.y, lightPos.z);
		glUniform3f(glGetUniformLocation(phongShader.Program, "viewPos"), camera.Position.x, camera.Position.y, camera.Position.z);
		glUniformMatrix4fv(glGetUniformLocation(phongShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(phongShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		model = glm::mat4();
		model = glm::translate(model, planePos - 0.2f);
		model = glm::scale(model, glm::vec3(30.0f, 30.0f, 30.0f));
		model = glm::rotate(model, ninety, glm::vec3(1.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(glGetUniformLocation(phongShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
		RenderQuad();






		/*
		/   Draw the SkyBox
		*/
		//Removes wireframe for skybox
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        // DRAW SKYBOX
		glDepthFunc(GL_LEQUAL);  // Change depth function so depth test passes when values are equal to depth buffer's content
		skyboxShader.Use(); //skybox shader
		view = glm::mat4(glm::mat3(camera.GetViewMatrix()));	// Remove any translation component of the view matrix, movement doesn't affect skybox position vectors
		glUniformMatrix4fv(glGetUniformLocation(skyboxShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(skyboxShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		// skybox cube
		glBindVertexArray(skyboxVAO);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
		glDepthFunc(GL_LESS); // Set depth function back to default
		
		

		// Swap the buffers
		glfwSwapBuffers(window);
	}

	glfwTerminate();
	return 0;
}










#pragma region "Makes a quad"

// RenderQuad() Renders a 1x1 quad in NDC
// Used for tangent calculations in normal mapping

GLuint quadVAO = 0;
GLuint quadVBO;
void RenderQuad()
{
	if (quadVAO == 0)
	{
		// normal vector
		glm::vec3 nm(0.0, 0.0, 1.0);


		// positions
		glm::vec3 pos1(-1.0, 1.0, 0.0);
		glm::vec3 pos2(-1.0, -1.0, 0.0);
		glm::vec3 pos3(1.0, -1.0, 0.0);
		glm::vec3 pos4(1.0, 1.0, 0.0);


		// texture coordinates
		glm::vec2 uv1(0.0, 1.0);
		glm::vec2 uv2(0.0, 0.0);
		glm::vec2 uv3(1.0, 0.0);
		glm::vec2 uv4(1.0, 1.0);




		// calculate tangent/bitangent vectors of both triangles
		glm::vec3 tangent1, bitangent1;
		glm::vec3 tangent2, bitangent2;

		// TRIANGLE 1
		glm::vec3 edge1 = pos2 - pos1;
		glm::vec3 edge2 = pos3 - pos1;
		glm::vec2 deltaUV1 = uv2 - uv1;
		glm::vec2 deltaUV2 = uv3 - uv1;

		GLfloat f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

		tangent1.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
		tangent1.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
		tangent1.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
		tangent1 = glm::normalize(tangent1);

		bitangent1.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
		bitangent1.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
		bitangent1.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
		bitangent1 = glm::normalize(bitangent1);

		// TRIANGLE 2
		edge1 = pos3 - pos1;
		edge2 = pos4 - pos1;
		deltaUV1 = uv3 - uv1;
		deltaUV2 = uv4 - uv1;

		f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

		tangent2.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
		tangent2.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
		tangent2.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
		tangent2 = glm::normalize(tangent2);


		bitangent2.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
		bitangent2.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
		bitangent2.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
		bitangent2 = glm::normalize(bitangent2);

		//Vertex data for rendering quads, used in walls

		GLfloat quadVertices[] = {
			// Positions            // normal         // TexCoords  // Tangent                          // Bitangent
			pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
			pos2.x, pos2.y, pos2.z, nm.x, nm.y, nm.z, uv2.x, uv2.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
			pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,

			pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
			pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
			pos4.x, pos4.y, pos4.z, nm.x, nm.y, nm.z, uv4.x, uv4.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z
		};

		// Plane VAO
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(GLfloat), (GLvoid*)(8 * sizeof(GLfloat)));
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(GLfloat), (GLvoid*)(11 * sizeof(GLfloat)));
	}
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}
#pragma endregion



// Loads a cubemap texture from 6 individual texture faces loaded in the 'faces' vector
GLuint loadCubemap(vector<const GLchar*> faces)
{
	GLuint textureID;
	glGenTextures(1, &textureID);
	glActiveTexture(GL_TEXTURE0);

	int width, height;
	unsigned char* image;

	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
	for (GLuint i = 0; i < faces.size(); i++)
	{
		image = SOIL_load_image(faces[i], &width, &height, 0, SOIL_LOAD_RGB);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	return textureID;
}


// General texture loading 
#pragma region "Texture Loading"
// This function loads a texture from file. 
GLuint loadTexture(GLchar* path)
{
	//Generate texture ID and load texture data 
	GLuint textureID;
	glGenTextures(1, &textureID);
	int width, height;
	unsigned char* image = SOIL_load_image(path, &width, &height, 0, SOIL_LOAD_RGB);
	// Assign texture to ID
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D);

	// Parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
	SOIL_free_image_data(image);
	return textureID;
}

#pragma endregion


// User input for camera-related transformations
#pragma region "User input"

// Moves/alters the camera positions based on user input
void Do_Movement()
{
	// Camera controls
	if (keys[GLFW_KEY_W])
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (keys[GLFW_KEY_S])
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (keys[GLFW_KEY_A])
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (keys[GLFW_KEY_D])
		camera.ProcessKeyboard(RIGHT, deltaTime);
}


// Is called whenever a key is pressed/released via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);


	if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
		playSimulation = !playSimulation;


	if (key == GLFW_KEY_B && action == GLFW_PRESS)
		boundBox = !boundBox;
	


	if (key == GLFW_KEY_O && action == GLFW_PRESS)
	{
		movePos = !movePos;
		posVec += glm::vec3(0.0f, 1.0f, 0.0f);
	}
	if (key == GLFW_KEY_P && action == GLFW_PRESS)
	{
		posVec = glm::vec3(0.0f, 0.0f, 0.0f);
		movePos = !movePos;
	}
	

	if (key == GLFW_KEY_I && action == GLFW_PRESS)
		spherePos.x += 0.3f;
	if (key == GLFW_KEY_K && action == GLFW_PRESS)
		spherePos.x -= 0.3f;
	if (key == GLFW_KEY_L && action == GLFW_PRESS)
		spherePos.z += 0.3f;
	if (key == GLFW_KEY_J && action == GLFW_PRESS)
		spherePos.z -= 0.3f;
	

	if (key == GLFW_KEY_N && action == GLFW_PRESS)
		checkSomething = !checkSomething;



	if (key == GLFW_KEY_X && action == GLFW_PRESS)
		place++;

	if (key == GLFW_KEY_C && action == GLFW_PRESS)
		applyForce = !applyForce;


	if (key == GLFW_KEY_Z && action == GLFW_PRESS)
		torqueVec += glm::vec3(0.5f, 1.0f, 0.0f);
	if (key == GLFW_KEY_X && action == GLFW_PRESS)
		torqueVec -= glm::vec3(0.5f, 1.0f, 0.0f);




	if (action == GLFW_PRESS)
		keys[key] = true;
	else if (action == GLFW_RELEASE)
		keys[key] = false;

	
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	GLfloat xoffset = xpos - lastX;
	GLfloat yoffset = lastY - ypos;

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}

#pragma endregion






