#pragma once
#include <GLEW\GL\glew.h>
#include <GLM\gtc\matrix_transform.hpp>
#include <vector>

class InteractiveShape;
class RenderShape;

class Patch
{
public:
	Patch(InteractiveShape& markerTemplate, RenderShape& slopeLineTemplate);
	~Patch();

	void Update();

private:
	void UpdateShapes();
	void UpdateSurface();
	void GeneratePlane();
	void AddVert(GLfloat x, GLfloat y, GLfloat z, GLfloat u, GLfloat v, int vertNum);
	void AddFace(GLint a, GLint b, GLint c, int faceNum);
private:
	glm::vec3 _controlPoints[16];
	InteractiveShape* _controlPointMarkers[16];
	RenderShape* _slopeLines[8];
	RenderShape* _curve;
	GLuint _vao;
	GLuint _vbo;
	GLuint _ebo;

	static const int NUM_VERTS = 10;
	static const int NUM_VERTS_STORED = NUM_VERTS * NUM_VERTS * 3;
	static const int NUM_ELEMENTS = (NUM_VERTS - 1) * (NUM_VERTS - 1) * 6;

	GLfloat _verts[NUM_VERTS_STORED];
	GLuint _elements[NUM_ELEMENTS];

	bool _controlPointsActive;
};