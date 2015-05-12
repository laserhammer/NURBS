/*
Bezier Curve
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

*	This example rather than dealing with static meshes, deals with dynamically generated shapes
*	based on Bernstein polynomials. This first on focuses on the the 2-D Bezier curve. The curve
*	generated based on a mathematical formula which is in this case. 
*
*	(1 - t)^3*A + 3t(1 - t)^2*B + 3t^2(1 - t)*C + t^3*D
*
*	This is the third order Bernstein polynomial. Any lower order represents the derivation of the 
*	one abobe and every higher order represents the integration of the one below. Also the order
*	of the polynomial determines how many control points will be required to determine its values.
*	For this formula "t" represents the location along the curve between 0 and 1, 0 being at the first
*	control point and 1 being at the last control point. 
*	A, B, C and D are position vectors representing the locations of each of the four control points.
*	To see some other BernStein polynomials see here: http://en.wikipedia.org/wiki/Bernstein_polynomial
*	There are 2 static component classes that make up the base functionality for this program
*
*	1) RenderManager
*	- This class maintains data for everything that needs to be drawn in two display lists, one for non-interactive shapes and 
*	one for interactive shapes. It handels the updating and drawing of these shapes.
*
*	2) InputManager
*	- This class handles all user input from the mouse and keyboard.
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
*	Bezier_Curve
*	- Holds data for the bezier curve and the helper shapes that go along with it. Generates and dynamically adjusts the vertices of the curve
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
#include "BezierCurve.h"

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
	-1.0f, +1.0f,
	+1.0f, +1.0f,
	-1.0f, -1.0f,
	+1.0f, -1.0f
};

GLint elements[] = {
	0, 1, 2,
	1, 3, 2
};

GLint outlineElements[] = {
	1, 2 
};

BezierCurve* bezierCurve;

void initShaders()
{
	char* shaders[] = { "fshader.glsl", "vshader.glsl" };
	GLenum types[] = { GL_FRAGMENT_SHADER, GL_VERTEX_SHADER };
	int numShaders = 2;
	
	shaderProgram = initShaders(shaders, types, numShaders);

	uTransform = glGetUniformLocation(shaderProgram, "transform");
	uColor = glGetUniformLocation(shaderProgram, "color");
}

void initGeometry()
{
	// Store the data for the triangles in a buffer that the gpu can use to draw
	glGenVertexArrays(1, &vao0);
	glBindVertexArray(vao0);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glGenBuffers(1, &ebo0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo0);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);

	// Bind buffer data to shader values
	posAttrib = glGetAttribLocation(shaderProgram, "position");
	glEnableVertexAttribArray(posAttrib);
	glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);

	glGenVertexArrays(1, &vao1);
	glBindVertexArray(vao1);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glGenBuffers(1, &ebo1);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo1);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(outlineElements), outlineElements, GL_STATIC_DRAW);

	glEnableVertexAttribArray(posAttrib);
	glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);
}

void init()
{
	if (!glfwInit()) exit(EXIT_FAILURE);

	//Create window
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	window = glfwCreateWindow(800, 600, "Bezier_Curve-GLFW", NULL, NULL); // Windowed

	//Activate window
	glfwMakeContextCurrent(window);

	glewExperimental = true;
	glewInit();

	initShaders();
	initGeometry();

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
	bezierCurve = new BezierCurve(InteractiveShape(collider, vao0, 6, GL_TRIANGLES, shader, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f)), RenderShape(vao1, 2, GL_LINES, shader, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f)), 64);

	InputManager::Init(window);
}

void step()
{
	// Clear to black
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	InputManager::Update();

	// Get delta time since the last frame
	float dt = (float)glfwGetTime();
	glfwSetTime(0.0);

	int verts = bezierCurve->numVerts();
	if (InputManager::downKey(true) && !InputManager::downKey()) verts /= 2;
	if (InputManager::upKey(true) && !InputManager::upKey()) verts *= 2;
	bezierCurve->numVerts(verts);

	RenderManager::Update(dt);

	bezierCurve->Update();

	RenderManager::Draw();

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
	glDeleteVertexArrays(1, &vao0);
	glDeleteVertexArrays(1, &vao1);

	RenderManager::DumpData();

	delete bezierCurve;

	glfwTerminate();
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
