/*
Bezier Surface
(c) 2015
original authors: Benjamin Robbins
Written under the supervision of David I. Schwartz, Ph.D., and
supported by a professional development seed grant from the B. Thomas
Golisano College of Computing & Information Sciences
(https://www.rit.edu/gccis) at the Rochester Institute of Technology.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or (at
your option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*	An expansion of the Bezier curve example, this example deals with a Bezier surface, now with 16 control points. The 
*	math theory is still the same, just the curve now needs to be solved for more control points. To find the position
*	of a given point on the surface, each row of control points needs to be solved at that column to form 4 meta-control
*	points that can then be used to solve to for the current vertex position.
*	This example also includes expanded camera functionality so you can get a better look at that curve.
*	There are 3 static component classes that make up the base functionality for this program
*
*	1) RenderManager
*	- This class maintains data for everything that needs to be drawn in two display lists, one for non-interactive shapes and
*	one for interactive shapes. It handels the updating and drawing of these shapes.
*
*	2) InputManager
*	- This class handles all user input from the mouse and keyboard.
*
*	3) CameraManager
*	- This class maintains the data for the view and projection matrices used in the rendering pipeline. It also updates the position
*	of the camera based on user input.
*
*	RenderShape
*	- Holds the instance data for a shape that can be rendered to the screen. This includes a transform, a vao, a shader, the drawing
*	mode (eg triangles, lines), it's active state, and its color
*
*	InteractiveShape
*	- Inherits from RenderShape, possessing all the same properties. Additionally, it has a collider and can use it to check collisions against
*	world boundries, other colliders, and the cursor.
*
*	Init_Shader
*	- Contains static functions for loading, compiling and linking shaders.
*
*	Patch
*	- Holds data for the Bezier surface and the helper shapes that go along with it. Generates and dynamically adjusts the vertices of the surface
*	based on the positions of the control points and the mathematical function above.
*/
#include <GLEW\GL\glew.h>
#include <GLFW\glfw3.h>
#include <GLM\gtc\type_ptr.hpp>
#include <GLM\gtc\matrix_transform.hpp>
#include <GLM\gtc\quaternion.hpp>
#include <GLM\gtc\random.hpp>
#include <iostream>
#include <ctime>

#include "RenderShape.h"
#include "InteractiveShape.h"
#include "Init_Shader.h"
#include "RenderManager.h"
#include "InputManager.h"
#include "Patch.h"
#include "CameraManager.h"

GLFWwindow* window;

GLuint vertexShader;
GLuint fragmentShader;
GLuint shaderProgram;

GLuint vbo;
GLuint vao0;
GLuint vao1;
GLuint ebo0;
GLuint ebo1;
GLint posAttrib;
GLint uTransform;
GLint uColor;

GLfloat vertices[] = {
	-1.0f, +1.0f, -1.0f,
	+1.0f, +1.0f, -1.0f,
	-1.0f, -1.0f, -1.0f,
	+1.0f, -1.0f, -1.0f,
	-1.0f, +1.0f, +1.0f,
	+1.0f, +1.0f, +1.0f,
	-1.0f, -1.0f, +1.0f,
	+1.0f, -1.0f, +1.0f
};

GLint elements[] = {
	// Front
	0, 1, 2,
	1, 3, 2,

	// Back
	6, 5, 4,
	6, 7, 5,

	// Top
	4, 5, 0,
	1, 0, 5,

	// Bottom
	7, 2, 3,
	7, 6, 2,

	// Right
	1, 5, 7,
	7, 3, 1,

	// Left
	6, 4, 0,
	0, 2, 6,
};

GLint outlineElements[] = {
	2, 5 
};

Patch* bezierSurface;

void initShaders()
{
	char* shaders[] = { "fshader.glsl", "vshader.glsl" };
	GLenum types[] = { GL_FRAGMENT_SHADER, GL_VERTEX_SHADER };
	int numShaders = 2;
	
	shaderProgram = initShaders(shaders, types, numShaders);
	
	// Bind buffer data to shader values
	posAttrib = glGetAttribLocation(shaderProgram, "position");
	glEnableVertexAttribArray(posAttrib);
	glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 0, 0);

	uTransform = glGetUniformLocation(shaderProgram, "transform");
	uColor = glGetUniformLocation(shaderProgram, "color");
}

void init()
{
	if (!glfwInit()) exit(EXIT_FAILURE);

	//Create window
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	window = glfwCreateWindow(800, 600, "K-D_Tree-GLFW", NULL, NULL); // Windowed

	//Activate window
	glfwMakeContextCurrent(window);

	glewExperimental = true;
	glewInit();

	// Store the data for the triangles in a buffer that the gpu can use to draw
	glGenVertexArrays(1, &vao0);
	glBindVertexArray(vao0);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glGenBuffers(1, &ebo0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo0);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);

	// Compile shaders
	initShaders();

	glGenVertexArrays(1, &vao1);
	glBindVertexArray(vao1);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glGenBuffers(1, &ebo1);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo1);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(outlineElements), outlineElements, GL_STATIC_DRAW);

	initShaders();

	glfwSetTime(0.0);

	Shader shader;
	shader.shaderPointer = shaderProgram;
	shader.uTransform = uTransform;
	shader.uColor = uColor;

	time_t timer;
	time(&timer);
	srand((unsigned int)timer);

	Collider collider;
	collider.height = 0.05f;
	collider.width = 0.05f;
	bezierSurface = new Patch(InteractiveShape(collider, vao0, 36, GL_TRIANGLES, shader, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f)), RenderShape(vao1, 2, GL_LINES, shader, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f)));

	InputManager::Init(window);
	CameraManager::Init(800.0f / 600.0f, 60.0f, 0.1f, 100.0f);

	glEnable(GL_DEPTH_TEST);
}

void step()
{
	// Clear to black
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	InputManager::Update();

	// Get delta time since the last frame
	float dt = glfwGetTime();
	glfwSetTime(0.0);

	CameraManager::Update(dt);

	RenderManager::Update(dt);

	bezierSurface->Update();

	RenderManager::Draw(CameraManager::ViewProjMat());

	// Swap buffers
	glfwSwapBuffers(window);
}

void cleanUp()
{
	glDeleteProgram(shaderProgram);
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	glDeleteBuffers(1, &vbo);
	glDeleteBuffers(1, &ebo0);
	glDeleteBuffers(1, &ebo1);
	glDeleteBuffers(1, &vao0);
	glDeleteBuffers(1, &vao1);

	RenderManager::DumpData();

	delete bezierSurface;
}

int main()
{
	init();

	while (!glfwWindowShouldClose(window))
	{
		step();
		glfwPollEvents();
	}

	cleanUp();

	return 0;
}
