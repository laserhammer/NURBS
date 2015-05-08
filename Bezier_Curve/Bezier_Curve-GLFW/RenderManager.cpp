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
	_shapeMoved = false;
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
		if (_interactiveShapes[i]->moved()) _shapeMoved = true;
	}
	unsigned int size = moused.size();
	for (unsigned int i = 0; i < size; ++i)
	{
		moused[i]->currentColor() = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	}
}

void RenderManager::Draw()
{
	unsigned int numShapes = _shapes.size();
	for (unsigned int i = 0; i < numShapes; ++i)
	{
		_shapes[i]->Draw(_projMat);
	}
	numShapes = _interactiveShapes.size();
	for (unsigned int i = 0; i < numShapes; ++i)
	{
		_interactiveShapes[i]->Draw(_projMat);
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
