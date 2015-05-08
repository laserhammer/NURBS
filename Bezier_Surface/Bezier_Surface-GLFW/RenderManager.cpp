#include "RenderManager.h"
#include "RenderShape.h"
#include "InteractiveShape.h"
#include "Init_Shader.h"
#include "InputManager.h"
#include <GLM\gtc\random.hpp>

std::vector<RenderShape*> RenderManager::_shapes = std::vector<RenderShape*>();
std::vector<InteractiveShape*> RenderManager::_interactiveShapes = std::vector<InteractiveShape*>();

bool RenderManager::_shapeMoved = false;

int RenderManager::_selectedShape = 0;

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
