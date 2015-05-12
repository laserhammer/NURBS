/*
Bezier Spline Wireframe
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

*	An expansion of the Bezier surface example, this example deals with an array of Bezier surfaces joined at the edges to form
*	a Bezier-Spline. A B-Spline is used to make the famous Utah teapot, so that's exactly what this program does.
*	It also includes a wireframe for the teapot for additional clarity.
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
*
*	B_Spline
*	- Generates and holds an array of Patch objects and gives them a single transform. 
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
#include "Init_Shader.h"
#include "RenderManager.h"
#include "InputManager.h"
#include "B-Spline.h"
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

// Source http://www.holmes3d.net/graphics/teapot/teapotCGA.bpt
GLfloat teapotControlPoints[] = {
	1.4, 2.25, 0,		1.3375,	2.38125, 0,			1.4375,	2.38125, 0,			1.5, 2.25, 0,
	1.4, 2.25, .784,	1.3375,	2.38125, .749,		1.4375, 2.38125, .805,		1.5, 2.25, .84,
	.784, 2.25, 1.4,	.749, 2.38125, 1.3375,		.805, 2.38125, 1.4375,		.84, 2.25, 1.5,
	0, 2.25, 1.4,		0, 2.38125, 1.3375,			0, 2.38125, 1.4375,			0, 2.25, 1.5,

	0, 2.25, 1.4,		0, 2.38125, 1.3375,			0, 2.38125, 1.4375,			0, 2.25, 1.5,
	-.784, 2.25, 1.4,	-.749, 2.38125, 1.3375,		-.805, 2.38125,	1.4375,		-.84, 2.25, 1.5,
	-1.4, 2.25, .784,	-1.3375, 2.38125, .749,		-1.4375, 2.38125, .805,		-1.5, 2.25, .84,
	-1.4, 2.25, 0,		-1.3375, 2.38125, 0,		-1.4375, 2.38125, 0,		-1.5, 2.25, 0,

	-1.4, 2.25, 0,		-1.3375, 2.38125, 0,		-1.4375, 2.38125, 0,		-1.5, 2.25, 0,
	-1.4, 2.25, -.784,	-1.3375, 2.38125, -.749,	-1.4375, 2.38125, -.805,	-1.5, 2.25, -.84,
	-.784, 2.25, -1.4,	-.749, 2.38125, -1.3375,	-.805, 2.38125,	-1.4375,	-.84, 2.25, -1.5,
	0, 2.25, -1.4,		0, 2.38125, -1.3375,		0,	2.38125, -1.4375,		0, 2.25, -1.5,

	0, 2.25, -1.4,		0, 2.38125, -1.3375,		0, 2.38125, -1.4375,		0, 2.25, -1.5,
	.784, 2.25, -1.4,	.749, 2.38125, -1.3375,		.805, 2.38125, -1.4375,		.84, 2.25, -1.5,
	1.4, 2.25, -.784,	1.3375,	2.38125, -.749,		1.4375, 2.38125, -.805,		1.5, 2.25, -.84,
	1.4, 2.25, 0,		1.3375,	2.38125, 0,			1.4375, 2.38125, 0,			1.5, 2.25, 0,

	1.5, 2.25, 0,		1.75, 1.725, 0,				2, 1.2,	0,					2, .75, 0,
	1.5, 2.25, .84,		1.75, 1.725, .98,			2, 1.2,	1.12,				2, .75, 1.12,
	.84, 2.25, 1.5,		.98, 1.725, 1.75,			1.12, 1.2, 2,				1.12, .75, 2,
	0, 2.25, 1.5,		0, 1.725, 1.75,				0, 1.2,	2,					0, .75, 2,

	0, 2.25, 1.5,		0, 1.725, 1.75,				0, 1.2,	2,					0, .75, 2,
	-.84, 2.25, 1.5,	-.98, 1.725, 1.75,			-1.12, 1.2, 2,				-1.12, .75, 2,
	-1.5, 2.25, .84,	-1.75, 1.725, .98,			-2, 1.2, 1.12,				-2, .75, 1.12,
	-1.5, 2.25, 0,		-1.75, 1.725, 0,			-2, 1.2, 0,					-2, .75, 0,

	-1.5, 2.25, 0,		-1.75, 1.725, 0,			-2, 1.2, 0,					-2, .75, 0,
	-1.5, 2.25, -.84,	-1.75, 1.725, -.98,			-2, 1.2, -1.12,				-2, .75, -1.12,
	-.84, 2.25, -1.5,	-.98, 1.725, -1.75,			-1.12, 1.2, -2,				-1.12, .75, -2,
	0, 2.25, -1.5,		0, 1.725,-1.75,				0, 1.2, -2,					0, .75, -2,

	0, 2.25, -1.5,		0, 1.725, -1.75,			0, 1.2,	-2,					0, .75, -2,
	.84, 2.25, -1.5,	.98, 1.725, -1.75,			1.12, 1.2, -2,				1.12, .75, -2,
	1.5, 2.25, -.84,	1.75, 1.725, -.98,			2, 1.2, -1.12,				2, .75, -1.12,
	1.5, 2.25, 0,		1.75, 1.725, 0,				2, 1.2, 0,					2, .75, 0,

	2, .75, 0,			2, .3, 0,					1.5, .075, 0,				1.5, 0, 0,
	2, .75, 1.12,		2, 	.3, 1.12,				1.5, .075, .84,				1.5, 0, .84,
	1.12, .75, 2,		1.12, .3, 2,				.84, .075, 1.5,				.84, 0, 1.5,
	0, .75, 2,			0, .3, 2,					0, .075, 1.5,				0, 0, 1.5,

	0, .75, 2,			0, .3, 2,					0, .075, 1.5,				0, 0, 1.5,
	-1.12, .75, 2,		-1.12, .3, 2,				-.84, .075, 1.5,			-.84, 0, 1.5,
	-2, .75, 1.12,		-2, .3, 1.12,				-1.5, .075,	.84,			-1.5, 0, .84,
	-2, .75, 0,			-2, .3, 0,					-1.5, .075,	0,				-1.5, 0, 0,

	-2, .75, 0,			-2,	.3, 0,					-1.5, .075,	0,				-1.5, 0, 0,
	-2, .75, -1.12,		-2,	.3, -1.12,				-1.5, .075,	-.84,			-1.5, 0, -.84,
	-1.12, .75, -2,		-1.12, .3, -2,				-.84, .075,	-1.5,			-.84, 0, -1.5,
	0, .75, -2,			0, .3, -2,					0, .075, -1.5,				0, 0, -1.5,

	0, .75, -2,			0, .3, -2,					0, .075, -1.5,				0, 0, -1.5,
	1.12, .75, -2,		1.12, .3, -2,				.84, .075, -1.5,			.84, 0, -1.5,
	2, .75, -1.12,		2, .3, -1.12,				1.5, .075,	-.84,			1.5, 0, -.84,
	2, .75, 0,			2, .3, 0,					1.5, .075,	0,				1.5, 0, 0,

	-1.6, 1.875, 0,		-2.3, 1.875, 0,				-2.7, 1.875, 0,				-2.7, 1.65, 0,
	-1.6, 1.875, .3,	-2.3, 1.875, .3,			-2.7, 1.875, .3,			-2.7, 1.65, .3,
	-1.5, 2.1, .3,		-2.5, 2.1, .3,				-3, 2.1, .3,				-3, 1.65, .3,
	-1.5, 2.1, 0,		-2.5, 2.1, 0,				-3, 2.1, 0,					-3, 1.65, 0,

	-1.5, 2.1, 0,		-2.5, 2.1, 0,				-3, 2.1, 0,					-3, 1.65, 0,
	-1.5, 2.1, -.3,		-2.5, 2.1, -.3,				-3, 2.1, -.3,				-3, 1.65, -.3,
	-1.6, 1.875, -.3,	-2.3, 1.875, -.3,			-2.7, 1.875, -.3,			-2.7, 1.65, -.3,
	-1.6, 1.875, 0,		-2.3, 1.875, 0,				-2.7, 1.875, 0,				-2.7, 1.65, 0,

	-2.7, 1.65, 0,		-2.7, 1.425, 0,				-2.5, .975, 0,				-2, .75, 0,
	-2.7, 1.65, .3,		-2.7, 1.425, .3,			-2.5, .975,	.3,				-2, .75, .3,
	-3, 1.65, .3,		-3, 1.2, .3,				-2.65, .7875, .3,			-1.9, .45, .3,
	-3, 1.65, 0,		-3,	1.2, 0,					-2.65, .7875, 0,			-1.9, .45, 0,

	-3, 1.65, 0,		-3,	1.2, 0,					-2.65, .7875, 0,			-1.9, .45, 0,
	-3, 1.65, -.3,		-3,	1.2, -.3,				-2.65, .7875, -.3,			-1.9, .45, -.3,
	-2.7, 1.65, -.3,	-2.7, 1.425, -.3,			-2.5, .975, -.3,			-2, .75, -.3,
	-2.7, 1.65, 0,		-2.7, 1.425, 0,				-2.5, .975,	0,				-2, .75, 0,

	1.7, 1.275, 0,		2.6, 1.275, 0,				2.3, 1.95, 0,				2.7, 2.25, 0,
	1.7, 1.275, .66,	2.6, 1.275, .66,			2.3, 1.95,	.25,			2.7, 2.25, .25,
	1.7, .45, .66,		3.1, .675, .66,				2.4, 1.875,	.25,			3.3, 2.25, .25,
	1.7, .45, 0,		3.1, .675, 0,				2.4, 1.875,	0,				3.3, 2.25, 0,

	1.7, .45, 0,		3.1, .675, 0,				2.4, 1.875,	0,				3.3, 2.25, 0,
	1.7, .45, -.66,		3.1, .675, -.66,			2.4, 1.875,	-.25,			3.3, 2.25, -.25,
	1.7, 1.275, -.66,	2.6, 1.275, -.66,			2.3, 1.95, -.25,			2.7, 2.25, -.25,
	1.7, 1.275, 0,		2.6, 1.275, 0,				2.3, 1.95, 0,				2.7, 2.25, 0,

	2.7, 2.25, 0,		2.8, 2.325, 0,				2.9, 2.325, 0,				 2.8, 2.25, 0,
	2.7, 2.25, .25,		2.8, 2.325, .25,			2.9, 2.325,	.15,			2.8, 2.25, .15,
	3.3, 2.25, .25,		3.525, 2.34375, .25,		3.45, 2.3625, .15,			3.2, 2.25, .15,
	3.3, 2.25, 0,		3.525, 2.34375, 0,			3.45, 2.3625, 0,			3.2, 2.25, 0,

	3.3, 2.25, 0,		3.525, 2.34375, 0,			3.45, 2.3625, 0,			3.2, 2.25, 0,
	3.3, 2.25, -.25,	3.525, 2.34375, -.25,		3.45, 2.3625, -.15,			3.2, 2.25, -.15,
	2.7, 2.25, -.25,	2.8, 2.325, -.25,			2.9, 2.325, -.15,			2.8, 2.25, -.15,
	2.7, 2.25, 0,		2.8, 2.325, 0,				2.9, 2.325,	0,				2.8, 2.25, 0,

	0, 3, 0,			.8, 3, 0,					0, 2.7, 0,					.2, 2.55, 0,
	0, 3, .002,			.8,	3, .45,					0, 2.7,	0,					.2, 2.55, .112,
	.002, 3, 0,			.45, 3, .8,					0, 2.7,	0,					.112, 2.55, .2,
	0, 3, 0,			0, 3, .8,					0, 2.7,	0,					0, 2.55, .2,

	0, 3, 0,			0, 3, .8,					0, 2.7,	0,					0, 2.55, .2,
	-.002, 3, 0,		-.45, 3, .8,				0, 2.7,	0,					-.112, 2.55, .2,
	0, 3, .002,			-.8, 3, .45,				0, 2.7,	0,					-.2, 2.55, .112,
	0, 3, 0,			-.8, 3, 0,					0, 2.7,	0,					-.2, 2.55, 0,

	0, 3, 0,			-.8, 3, 0,					0, 2.7,	0,					-.2, 2.55, 0,
	0, 3, -.002,		-.8, 3, -.45,				0, 2.7,	0,					-.2, 2.55, -.112,
	-.002, 3, 0,		-.45, 3, -.8,				0, 2.7,	0,					-.112, 2.55, -.2,
	0, 3, 0,			0, 3, -.8,					0, 2.7,	0,					0, 2.55, -.2,

	0, 3, 0,			0, 3, -.8,					0, 2.7,	0,					0, 2.55, -.2,
	.002, 3, 0,			.45, 3, -.8,				0, 2.7,	0,					.112, 2.55, -.2,
	0, 3, -.002,		.8,	3, -.45,				0, 2.7, 0,					.2, 2.55, -.112,
	0, 3, 0,			.8, 3, 0,					0, 2.7,	0,					.2, 2.55, 0,

	.2, 2.55, 0,		.4, 2.4, 0,					1.3, 2.4, 0,				1.3, 2.25, 0,
	.2, 2.55, .112,		.4, 2.4, .224,				1.3, 2.4, .728,				1.3, 2.25, .728,
	.112, 2.55, .2,		.224, 2.4, .4,				.728, 2.4, 1.3,				.728, 2.25, 1.3,
	0, 2.55, .2,		0, 2.4, .4,					0, 2.4, 1.3,				0, 2.25, 1.3,

	0, 2.55, .2,		0, 2.4, .4,					0, 2.4,	1.3,				0, 2.25, 1.3,
	-.112, 2.55, .2,	-.224, 2.4, .4,				-.728, 2.4, 1.3,			-.728, 2.25, 1.3,
	-.2, 2.55, .112,	-.4, 2.4, .224,				-1.3, 2.4, .728,			-1.3, 2.25, .728,
	-.2, 2.55, 0,		-.4, 2.4, 0,				-1.3, 2.4, 0,				-1.3, 2.25, 0,

	-.2, 2.55, 0,		-.4, 2.4, 0,				-1.3, 2.4, 0,				-1.3, 2.25, 0,
	-.2, 2.55, -.112,	-.4, 2.4, -.224,			-1.3, 2.4, -.728,			-1.3, 2.25, -.728,
	-.112, 2.55, -.2,	-.224, 2.4, -.4,			-.728, 2.4,	-1.3,			-.728, 2.25, -1.3,
	0, 2.55, -.2,		0, 2.4, -.4,				0, 2.4, -1.3,				0, 2.25, -1.3,

	0, 2.55, -.2,		0, 2.4, -.4,				0, 2.4, -1.3,				0, 2.25, -1.3,
	.112, 2.55, -.2,	.224, 2.4, -.4,				.728, 2.4, -1.3,			.728, 2.25, -1.3,
	.2, 2.55, -.112,	.4, 2.4, -.224,				1.3, 2.4, -.728,			1.3, 2.25, -.728,
	.2, 2.55, 0,		.4, 2.4, 0,					1.3, 2.4, 0,				1.3, 2.25, 0
};

B_Spline* teapot;

void generateTeapot()
{
	Shader shader;
	shader.shaderPointer = shaderProgram;
	shader.uTransform = uTransform;
	shader.uColor = uColor;

	teapot = new B_Spline(RenderShape(vao0, 36, GL_TRIANGLES, shader, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), false), RenderShape(vao1, 2, GL_LINES, shader, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), false), 28);

	for (int i = 0; i < 28; ++i)
	{
		int k = i * 48;

		teapot->SetControlPoints(i,
			glm::vec3(teapotControlPoints[k++], teapotControlPoints[k++], teapotControlPoints[k++]),
			glm::vec3(teapotControlPoints[k++], teapotControlPoints[k++], teapotControlPoints[k++]),
			glm::vec3(teapotControlPoints[k++], teapotControlPoints[k++], teapotControlPoints[k++]),
			glm::vec3(teapotControlPoints[k++], teapotControlPoints[k++], teapotControlPoints[k++]),

			glm::vec3(teapotControlPoints[k++], teapotControlPoints[k++], teapotControlPoints[k++]),
			glm::vec3(teapotControlPoints[k++], teapotControlPoints[k++], teapotControlPoints[k++]),
			glm::vec3(teapotControlPoints[k++], teapotControlPoints[k++], teapotControlPoints[k++]),
			glm::vec3(teapotControlPoints[k++], teapotControlPoints[k++], teapotControlPoints[k++]),

			glm::vec3(teapotControlPoints[k++], teapotControlPoints[k++], teapotControlPoints[k++]),
			glm::vec3(teapotControlPoints[k++], teapotControlPoints[k++], teapotControlPoints[k++]),
			glm::vec3(teapotControlPoints[k++], teapotControlPoints[k++], teapotControlPoints[k++]),
			glm::vec3(teapotControlPoints[k++], teapotControlPoints[k++], teapotControlPoints[k++]),

			glm::vec3(teapotControlPoints[k++], teapotControlPoints[k++], teapotControlPoints[k++]),
			glm::vec3(teapotControlPoints[k++], teapotControlPoints[k++], teapotControlPoints[k++]),
			glm::vec3(teapotControlPoints[k++], teapotControlPoints[k++], teapotControlPoints[k++]),
			glm::vec3(teapotControlPoints[k++], teapotControlPoints[k++], teapotControlPoints[k++]));
	}

	teapot->transform().position = glm::vec3(0.0f, -1.5f, 0.0f);
}

void initShaders()
{
	char* shaders[] = { "fshader.glsl", "vshader.glsl" };
	GLenum types[] = { GL_FRAGMENT_SHADER, GL_VERTEX_SHADER };
	int numShaders = 2;
	
	shaderProgram = initShaders(shaders, types, numShaders);
	
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

	window = glfwCreateWindow(800, 600, "Bezier_Spline_Wireframe-GLFW", NULL, NULL); // Windowed

	//Activate window
	glfwMakeContextCurrent(window);

	glewExperimental = true;
	glewInit();

	initShaders();

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
	glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glGenVertexArrays(1, &vao1);
	glBindVertexArray(vao1);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glGenBuffers(1, &ebo1);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo1);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(outlineElements), outlineElements, GL_STATIC_DRAW);

	// Bind buffer data to shader values
	posAttrib = glGetAttribLocation(shaderProgram, "position");
	glEnableVertexAttribArray(posAttrib);
	glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glfwSetTime(0.0);

	time_t timer;
	time(&timer);
	srand((unsigned int)timer);

	generateTeapot();

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

	teapot->Update(dt);

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

	delete teapot;
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
