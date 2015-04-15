#pragma once
#include <GLEW\GL\glew.h>
#include <GLM\gtc\matrix_transform.hpp>

class InteractiveShape;
class RenderShape;

class BezierCurve
{
public:
	BezierCurve(InteractiveShape& markerTemplate, RenderShape& slopeLineTemplate, int maxNumVerts);
	~BezierCurve();

	void Update();

	void numVerts(int newNumVerts);
	int numVerts();

private:
	void UpdateShapes();
	void UpdateCurve();
private:
	glm::vec3 _controlPoints[4];
	InteractiveShape* _controlPointerMarkers[4];
	RenderShape* _slopeLines[2];
	RenderShape* _curve;
	GLuint _vao;
	GLuint _vbo;
	GLuint _ebo;
	int _numVerts;
	int _maxNumVerts;
};