#pragma once
#include <GLEW\GL\glew.h>
#include <GLM\gtc\matrix_transform.hpp>
#include <vector>

struct Transform;
struct Collider;
struct Shader;
class RenderShape;
class InteractiveShape;

class RenderManager
{
public:

	static void GenerateShapes(Shader shader, GLuint vao, int numShapes, GLenum type, GLsizei count);

	static void AddShape(Shader shader, GLuint vao, GLenum type, GLsizei count, glm::vec4 color, Transform transform, Collider collider);

	static void AddShape(Shader shader, GLuint vao, GLenum type, GLsizei count, glm::vec4 color, Transform transform);
	
	static void AddShape(RenderShape* shape);

	static void AddShape(InteractiveShape* shape);

	static void Update(float dt);

	static void Draw(glm::mat4& viewProjMat);

	static void DumpData();

	static std::vector<InteractiveShape*>& interactiveShapes();
	static bool shapeMoved();

private:

	static std::vector<RenderShape*> _shapes;
	static std::vector<InteractiveShape*> _interactiveShapes;

	static bool _shapeMoved;
	static int _selectedShape;
};
