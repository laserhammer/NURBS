#pragma once

#include <GLM\gtc\type_ptr.hpp>
#include <GLM\gtc\matrix_transform.hpp>

class CameraManager
{
public:
	static void Init(float aspectRatio, float fov, float near, float far);
	static void Update(float dt);
	static glm::mat4 ViewProjMat();
private:
	static glm::mat4 _proj;
	static glm::mat4 _view;

	static glm::vec2 _position;
};