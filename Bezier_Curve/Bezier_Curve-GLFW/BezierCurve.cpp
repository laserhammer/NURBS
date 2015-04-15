#include "BezierCurve.h"
#include "RenderManager.h"
#include "RenderShape.h"
#include "InteractiveShape.h"
#include "Init_Shader.h"

#include <vector>

BezierCurve::BezierCurve(InteractiveShape& markerTemplate, RenderShape& slopeLineTemplate, int maxNumVerts)
{
	_controlPoints[0] = glm::vec3(-0.75f, 0.0f, 0.0f);
	_controlPoints[1] = glm::vec3(-0.25f, 0.0f, 0.0f);
	_controlPoints[2] = glm::vec3(+0.25f, 0.0f, 0.0f);
	_controlPoints[3] = glm::vec3(+0.75f, 0.0f, 0.0f);

	_controlPointerMarkers[0] = new InteractiveShape(markerTemplate);
	_controlPointerMarkers[0]->transform().scale = glm::vec3(0.025f, 0.025f, 0.025f);
	_controlPointerMarkers[0]->transform().position = _controlPoints[0];

	_controlPointerMarkers[1] = new InteractiveShape(markerTemplate);
	_controlPointerMarkers[1]->transform().scale = glm::vec3(0.025f, 0.025f, 0.025f);
	_controlPointerMarkers[1]->transform().position = _controlPoints[1];

	_controlPointerMarkers[2] = new InteractiveShape(markerTemplate);
	_controlPointerMarkers[2]->transform().scale = glm::vec3(0.025f, 0.025f, 0.025f);
	_controlPointerMarkers[2]->transform().position = _controlPoints[2];

	_controlPointerMarkers[3] = new InteractiveShape(markerTemplate);
	_controlPointerMarkers[3]->transform().scale = glm::vec3(0.025f, 0.025f, 0.025f);
	_controlPointerMarkers[3]->transform().position = _controlPoints[3];

	RenderManager::AddShape(_controlPointerMarkers[0]);
	RenderManager::AddShape(_controlPointerMarkers[1]);
	RenderManager::AddShape(_controlPointerMarkers[2]);
	RenderManager::AddShape(_controlPointerMarkers[3]);

	_slopeLines[0] = new RenderShape(slopeLineTemplate);
	_slopeLines[1] = new RenderShape(slopeLineTemplate);

	RenderManager::AddShape(_slopeLines[0]);
	RenderManager::AddShape(_slopeLines[1]);
	
	_numVerts = maxNumVerts;
	_maxNumVerts = maxNumVerts;

	GLfloat data = 0.0f;
	GLint elements = 0;

	glGenVertexArrays(1, &_vao);
	glBindVertexArray(_vao);

	glGenBuffers(1, &_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, _vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat), &data, GL_DYNAMIC_DRAW);

	glGenBuffers(1, &_ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLint), &elements, GL_DYNAMIC_DRAW);

	// Bind buffer data to shader values
	GLint posAttrib = glGetAttribLocation(slopeLineTemplate.shader().shaderPointer, "position");
	glEnableVertexAttribArray(posAttrib);
	glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 0, 0);

	GLint uTransform = glGetUniformLocation(slopeLineTemplate.shader().shaderPointer, "transform");
	GLint uColor = glGetUniformLocation(slopeLineTemplate.shader().shaderPointer, "color");
	
	_curve = new RenderShape(_vao, 0, GL_LINE_STRIP, slopeLineTemplate.shader());

	RenderManager::AddShape(_curve);
	UpdateShapes();
}
BezierCurve::~BezierCurve()
{
	glDeleteBuffers(1, &_vbo);
	glDeleteBuffers(1, &_vao);
}

void BezierCurve::Update()
{
	bool markerMoved[4];
	markerMoved[0] = _controlPoints[0] != _controlPointerMarkers[0]->transform().position;
	markerMoved[1] = _controlPoints[1] != _controlPointerMarkers[1]->transform().position;
	markerMoved[2] = _controlPoints[2] != _controlPointerMarkers[2]->transform().position;
	markerMoved[3] = _controlPoints[3] != _controlPointerMarkers[3]->transform().position;

	if (markerMoved[0] || markerMoved[1] || markerMoved[2] || markerMoved[3])
	{
		UpdateShapes();
	}
}

void  BezierCurve::UpdateShapes()
{
	// Update the control points
	_controlPoints[0] = _controlPointerMarkers[0]->transform().position;
	_controlPoints[1] = _controlPointerMarkers[1]->transform().position;
	_controlPoints[2] = _controlPointerMarkers[2]->transform().position;
	_controlPoints[3] = _controlPointerMarkers[3]->transform().position;

	// Update the slope lines
	glm::vec3 start0 = _controlPoints[0];
	glm::vec3 end0 = _controlPoints[1];
	glm::vec3 line0 = end0 - start0;

	glm::vec3 start1 = _controlPoints[2];
	glm::vec3 end1 = _controlPoints[3];
	glm::vec3 line1 = end1 - start1;

	glm::vec3 startingVec = glm::vec3(0.7071f, 0.7071f, 0.0f);

	_slopeLines[0]->transform().position = (start0 + end0) / 2.0f;
	_slopeLines[1]->transform().position = (start1 + end1) / 2.0f;

	float angle0 = acosf(glm::dot(line0, startingVec) / glm::length(line0));
	float angle1 = acosf(glm::dot(line1, startingVec) / glm::length(line1));

	glm::vec3 axis0 = glm::cross(line0, startingVec);
	glm::vec3 axis1 = glm::cross(line1, startingVec);

	_slopeLines[0]->transform().rotation = glm::angleAxis(angle0, axis0);
	_slopeLines[1]->transform().rotation = glm::angleAxis(angle1, axis1);

	_slopeLines[0]->transform().scale = line0 / 2.0f;
	_slopeLines[1]->transform().scale = line1 / 2.0f;

	// Update the curve
	UpdateCurve();
}

void BezierCurve::UpdateCurve()
{
	int numDataPoints = _numVerts * 3;
	std::vector<GLfloat> data = std::vector<GLfloat>();
	data.resize(numDataPoints);
	std::vector<GLint> elements = std::vector<GLint>();
	elements.resize(_numVerts);
	for (int i = 0; i < _numVerts; ++i)
	{
		float t = (float)i / ((float)_numVerts - 1.0f);

		//Blending factors
		float factor0 = (1 - t) * (1 - t) * (1 - t);	// 1-u^3
		float factor1 = 3 * t * ((1 - t) * (1 - t));	// 3u(1-u)^2
		float factor2 = 3 * (t * t) * (1 - t);			// 3u^2(1-u)
		float factor3 = t * t * t;						// u^3

		data[i * 3] = factor0 * _controlPoints[0].x + factor1 * _controlPoints[1].x + factor2 * _controlPoints[2].x + factor3 * _controlPoints[3].x;
		data[i * 3 + 1] = factor0 * _controlPoints[0].y + factor1 * _controlPoints[1].y + factor2 * _controlPoints[2].y + factor3 * _controlPoints[3].y;
		data[i * 3 + 2] = factor0 * _controlPoints[0].z + factor1 * _controlPoints[1].z + factor2 * _controlPoints[2].z + factor3 * _controlPoints[3].z;
		elements[i] = i;
	}
	glBindVertexArray(_vao);

	glBindBuffer(GL_ARRAY_BUFFER, _vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * numDataPoints, (void*)&data[0], GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLint) * _numVerts, (void*)&elements[0], GL_DYNAMIC_DRAW);
	_curve->count(_numVerts);
}

void BezierCurve::numVerts(int newNumVerts)
{
	if (newNumVerts != _numVerts && newNumVerts <= _maxNumVerts && newNumVerts >= 1)
	{
		_numVerts = newNumVerts;
		UpdateCurve();
	}
}
int BezierCurve::numVerts() { return _numVerts; }
