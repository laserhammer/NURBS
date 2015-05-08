#include "RenderManager.h"
#include "RenderShape.h"
#include "Init_Shader.h"
#include "InputManager.h"
#include <GLM\gtc\random.hpp>

std::vector<RenderShape*> RenderManager::_shapes = std::vector<RenderShape*>();
std::vector<RenderShape*> RenderManager::_noDepthShapes = std::vector<RenderShape*>();

void RenderManager::AddShape(Shader shader, GLuint vao, GLenum type, GLsizei count, glm::vec4 color, Transform transform)
{
	_shapes.push_back(new RenderShape(vao, count, type, shader, color));
	_shapes[_shapes.size() - 1]->transform() = transform;
}

void RenderManager::AddShape(RenderShape* shape)
{
	if (shape->useDepthTest())
	{
		_shapes.push_back(shape);
		_shapes[_shapes.size() - 1]->transform() = shape->transform();
	}
	else
	{
		_noDepthShapes.push_back(shape);
		_noDepthShapes[_noDepthShapes.size() - 1]->transform() = shape->transform();
	}
}

void RenderManager::Update(float dt)
{
	unsigned int numShapes = _shapes.size();
	for (unsigned int i = 0; i < numShapes; ++i)
	{
		_shapes[i]->Update(dt);
	}
}

void RenderManager::Draw(glm::mat4& viewProjMat)
{
	unsigned int numShapes = _shapes.size();
	for (unsigned int i = 0; i < numShapes; ++i)
	{
		_shapes[i]->Draw(viewProjMat);
	}
	unsigned int numNoDepthShapes = _noDepthShapes.size();
	for (unsigned int i = 0; i < numNoDepthShapes; ++i)
	{
		_noDepthShapes[i]->Draw(viewProjMat);
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
}

