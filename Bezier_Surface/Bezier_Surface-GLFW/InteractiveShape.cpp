#include "InteractiveShape.h"
#include "InputManager.h"

InteractiveShape::InteractiveShape(Collider collider, GLint vao, GLsizei count, GLenum mode, Shader shader, glm::vec4 color)
{
	this->RenderShape::RenderShape(vao, count, mode, shader, color);
	_collider = collider;
	_mouseOver = false;
	_selected = false;
	_moved = false;
}

InteractiveShape::~InteractiveShape(){}

void InteractiveShape::Update(float dt)
{
	float moveRate = 1.0f;
	RenderShape::Update(dt);

	_currentColor = _color;

	if (_selected)
	{
		_selected = false;
		float dx = 0.0f;
		dx += (InputManager::aKey()) * moveRate * dt;
		dx -= (InputManager::dKey()) * moveRate * dt;

		float dy = 0.0f;
		dy -= (InputManager::ctrlKey()) * moveRate * dt;
		dy += (InputManager::shiftKey()) * moveRate * dt;

		float dz = 0.0f;
		dz -= (InputManager::sKey()) * moveRate * dt;
		dz += (InputManager::wKey()) * moveRate * dt;

		_transform.position += glm::vec3(dx, dy, dz);

		_moved = (dx != 0.0f && dy != 0.0f && dz != 0.0f);

		_currentColor =  _color + glm::vec4(0.7f, 0.7f, 0.7f, 1.0f);
	}

	// Constrain position to the world
	_transform.position.x = _transform.position.x > 1.0f ? 1.0f : _transform.position.x;
	_transform.position.x = _transform.position.x < -1.0f ? -1.0f : _transform.position.x;
	_transform.position.y = _transform.position.y > 1.0f ? 1.0f : _transform.position.y;
	_transform.position.y = _transform.position.y < -1.0f ? -1.0f : _transform.position.y;
	_transform.position.z = _transform.position.z > 1.0f ? 1.0f : _transform.position.z;
	_transform.position.z = _transform.position.z < -1.0f ? -1.0f : _transform.position.z;
}

void InteractiveShape::Draw(const glm::mat4& viewProjMat)
{
	RenderShape::Draw(viewProjMat);
}

const Collider& InteractiveShape::collider()
{
	Collider ret = _collider;
	ret.x += _transform.position.x;
	ret.y += _transform.position.y;
	return ret;
}

bool InteractiveShape::mouseOver() { return _mouseOver; }
bool InteractiveShape::mouseOut() { return _mouseOut; }
bool InteractiveShape::moved() { return _moved; }
bool InteractiveShape::selected() { return _selected; }
void InteractiveShape::selected(bool setSelected) { _selected = setSelected; }