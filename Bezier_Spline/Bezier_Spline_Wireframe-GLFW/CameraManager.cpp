#include "CameraManager.h"
#include "InputManager.h"

glm::mat4 CameraManager::_proj;
glm::mat4 CameraManager::_view;
glm::vec2 CameraManager::_position;

void CameraManager::Init(float aspectRatio, float fov, float near, float far)
{
	_proj = glm::perspectiveFov(fov, aspectRatio, 1.0f / aspectRatio, near, far);
	_position = glm::vec2(0.0f, 0.0f);
}

void CameraManager::Update(float dt)
{
	float rotRate = 180.0f;
	if (InputManager::cursorLocked())
	{
		glm::vec2 mouseCoords = InputManager::GetMouseCoords();

		_position.x += mouseCoords.x * rotRate;
		_position.x -= _position.x > 360.0f ? 360.0f : 0.0f;

		_position.y += mouseCoords.y * rotRate;
	}

	_position.y = _position.y > 45.0f ? 45.0f : _position.y;
	_position.y = _position.y < -45.0f ? -45.0f : _position.y;

	glm::mat4 bearingMat = glm::mat4_cast(glm::angleAxis(_position.x, glm::vec3(0.0f, 1.0f, 0.0f)));
	glm::mat4 eleMat = glm::mat4_cast(glm::angleAxis(_position.y, glm::vec3(1.0f, 0.0f, 0.0f)));
	glm::mat4 rotMat = eleMat * bearingMat;
	glm::vec3 camPos = glm::vec3(glm::vec4(0.0f, 0.0f, -5.0f, 1.0f) * rotMat);

	_view = glm::lookAt(camPos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
}

glm::mat4 CameraManager::ViewProjMat()
{
	return _proj *_view;
}