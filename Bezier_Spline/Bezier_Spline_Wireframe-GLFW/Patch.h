#pragma once
#include "RenderShape.h"

#include <GLEW\GL\glew.h>
#include <GLM\gtc\matrix_transform.hpp>
#include <vector>

class RenderShape;

class Patch
{
public:
	Patch(RenderShape& markerTemplate, RenderShape& slopeLineTemplate);
	~Patch();

	void Update(float dt);

	void SetControlPoint(int controlPointIndex, glm::vec3 newPos);
	Transform& transform();
private:
	void UpdateShapes();
	void UpdateSurface();
	void GeneratePlane();
	void AddVert(GLfloat x, GLfloat y, GLfloat z, GLfloat u, GLfloat v, int vertNum);
	void AddFace(GLint a, GLint b, GLint c, int faceNum);
private:
	glm::vec3 _controlPoints[16];
	RenderShape* _controlPointMarkers[16];
	RenderShape* _slopeLines[8];
	RenderShape* _curve;
	RenderShape* _curveLines;
	GLuint _vaoTris;
	GLuint _vaoLines;
	GLuint _vbo;
	GLuint _eboTris;
	GLuint _eboLines;

	Transform _transform;

	static const int NUM_VERTS = 10;
	static const int NUM_VERTS_STORED = NUM_VERTS * NUM_VERTS * 3;
	static const int NUM_ELEMENTS = (NUM_VERTS - 1) * (NUM_VERTS - 1) * 6;
	static const int NUM_LINE_ELEMENTS = NUM_ELEMENTS * 2;

	GLfloat _verts[NUM_VERTS_STORED];
	GLuint _elements[NUM_ELEMENTS];
	GLuint _lineElements[NUM_LINE_ELEMENTS];

	bool _controlPointsActive;
	bool _wireframeActive;
	bool _baseActive;
};