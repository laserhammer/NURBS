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
	
	// Bind buffer data to shader values
	posAttrib = glGetAttribLocation(shaderProgram, "position");
	glEnableVertexAttribArray(posAttrib);
	glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);

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
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glGenVertexArrays(1, &vao0);
	glBindVertexArray(vao0);

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
	float dt = glfwGetTime();
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
	glDeleteBuffers(1, &vao0);
	glDeleteBuffers(1, &vao1);

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
