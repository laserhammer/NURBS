#include "RenderManager.h"
#include "RenderShape.h"
#include "InteractiveShape.h"
#include "Init_Shader.h"
#include "InputManager.h"
#include <GLM\gtc\random.hpp>

std::vector<RenderShape*> RenderManager::_shapes = std::vector<RenderShape*>();
std::vector<InteractiveShape*> RenderManager::_interactiveShapes = std::vector<InteractiveShape*>();

glm::mat4 RenderManager::_projMat = glm::ortho(-1.337f, 1.337f, -1.0f, 1.0f);

bool RenderManager::_shapeMoved = false;

int RenderManager::_selectedShape = 0;

void RenderManager::GenerateShapes(Shader shader, GLuint vao, int numShapes, GLenum type, GLsizei count)
{
	Collider collider;
	collider.height = 0.035f;
	collider.width = 0.035f;
	for (int i = 0; i < numShapes; ++i)
	{
		Transform transform = Transform();
		transform.position = glm::vec3(glm::linearRand(-1.337f, 1.337f), glm::linearRand(-1.0f, 1.0f), 0.0f);
		glm::vec4 color = glm::vec4(glm::linearRand(0.25f, 0.75f), glm::linearRand(0.25f, 0.75f), glm::linearRand(0.25f, 0.75f), 1.0f);
		transform.scale = glm::vec3(0.025f, 0.025f, 0.01f);
		AddShape(shader, vao, type, count, color, transform, collider);
	}
}

void RenderManager::AddShape(Shader shader, GLuint vao, GLenum type, GLsizei count, glm::vec4 color, Transform transform, Collider collider)
{
	_interactiveShapes.push_back(new InteractiveShape(collider, vao, count, type, shader, color));
	_interactiveShapes[_interactiveShapes.size() - 1]->transform() = transform;
}

void RenderManager::AddShape(Shader shader, GLuint vao, GLenum type, GLsizei count, glm::vec4 color, Transform transform)
{
	_shapes.push_back(new RenderShape(vao, count, type, shader, color));
	_shapes[_shapes.size() - 1]->transform() = transform;
}

void RenderManager::AddShape(RenderShape* shape)
{
	_shapes.push_back(shape);
	_shapes[_shapes.size() - 1]->transform() = shape->transform();
}

void RenderManager::AddShape(InteractiveShape* shape)
{
	_interactiveShapes.push_back(shape);
	_interactiveShapes[_interactiveShapes.size() - 1]->transform() = shape->transform();
}

void RenderManager::Update(float dt)
{
	// Select a shape
	unsigned int size = _interactiveShapes.size();
	if (size)
	{
		int dSelected = 0;
		dSelected += (InputManager::rightKey(true) && !InputManager::rightKey());
		dSelected -= (InputManager::leftKey(true) && !InputManager::leftKey());
		_selectedShape += dSelected;
		_selectedShape %= _interactiveShapes.size();
		_interactiveShapes[_selectedShape]->selected(true);
	}
	std::vector<InteractiveShape*>& moused = std::vector<InteractiveShape*>();
	unsigned int numShapes = _shapes.size();
	for (unsigned int i = 0; i < numShapes; ++i)
	{
		_shapes[i]->Update(dt);
	}
	numShapes = _interactiveShapes.size();
	for (unsigned int i = 0; i < numShapes; ++i)
	{
		_interactiveShapes[i]->Update(dt);
	}
}

void RenderManager::Draw(glm::mat4& viewProjMat)
{
	unsigned int numShapes = _shapes.size();
	for (unsigned int i = 0; i < numShapes; ++i)
	{
		_shapes[i]->Draw(viewProjMat);
	}
	numShapes = _interactiveShapes.size();
	for (unsigned int i = 0; i < numShapes; ++i)
	{
		_interactiveShapes[i]->Draw(viewProjMat);
	}
}

void RenderManager::DumpData()
{
	unsigned int i;
	while (i = _shapes.size())
	{
		delete _shapes[i - 1];
		_shapes.pop_back();
	}
	while (i = _interactiveShapes.size())
	{
		delete _interactiveShapes[i - 1];
		_interactiveShapes.pop_back();
	}
}

std::vector<InteractiveShape*>& RenderManager::interactiveShapes()
{
	return _interactiveShapes;
}

bool RenderManager::shapeMoved() { return _shapeMoved; }
