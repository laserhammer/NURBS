#include "Patch.h"
#include "RenderManager.h"
#include "RenderShape.h"
#include "Init_Shader.h"
#include "InputManager.h"

#include <vector>

Patch::Patch(RenderShape& markerTemplate, RenderShape& slopeLineTemplate)
{
	_transform = Transform();
	_transform.position = glm::vec3();
	_transform.rotation = glm::quat();
	_transform.scale = glm::vec3(1.0f, 1.0f, 1.0f);

	_transform.angularVelocity = glm::quat();
	_transform.linearVelocity = glm::vec3();

	_transform.rotationOrigin = glm::vec3();
	_transform.scaleOrigin = glm::vec3();

	GLfloat data = 0.0f;
	GLint elements = 0;
	
	glGenVertexArrays(1, &_vaoTris);
	glBindVertexArray(_vaoTris);

	glGenBuffers(1, &_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, _vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat), &data, GL_DYNAMIC_DRAW);

	glGenBuffers(1, &_eboTris);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _eboTris);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLint), &elements, GL_DYNAMIC_DRAW);

	// Bind buffer data to shader values
	GLint posAttrib = glGetAttribLocation(slopeLineTemplate.shader().shaderPointer, "position");
	glEnableVertexAttribArray(posAttrib);
	glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 0, 0);

	_curve = new RenderShape(_vaoTris, NUM_ELEMENTS, GL_TRIANGLES, slopeLineTemplate.shader(), glm::vec4(0.6f, 0.6f, 0.6f, 1.0f));

	_curve->transform().parent = &_transform;
	
	RenderManager::AddShape(_curve);
	
	// Add the wireframe for the patch as its own render shape
	glGenVertexArrays(1, &_vaoLines);
	glBindVertexArray(_vaoLines);

	//glGenBuffers(1, &_vboLines);
	glBindBuffer(GL_ARRAY_BUFFER, _vbo);

	glGenBuffers(1, &_eboLines);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _eboLines);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLint), &elements, GL_DYNAMIC_DRAW);
	
	// Bind buffer data to shader values
	GLint posAttribLines = glGetAttribLocation(slopeLineTemplate.shader().shaderPointer, "position");
	glEnableVertexAttribArray(posAttribLines);
	glVertexAttribPointer(posAttribLines, 3, GL_FLOAT, GL_FALSE, 0, 0);
	
	_curveLines = new RenderShape(_vaoLines, NUM_LINE_ELEMENTS, GL_LINES, slopeLineTemplate.shader(), glm::vec4(0.0f, 0.8f, 0.0f, 1.0f), false);

	_curveLines->transform().parent = &_transform;

	RenderManager::AddShape(_curveLines);

	for (int i = 0; i < 16; ++i)
	{
		_controlPointMarkers[i] = new RenderShape(markerTemplate);
		_controlPointMarkers[i]->transform().scale = glm::vec3(0.01f, 0.01f, 0.01f);
		_controlPointMarkers[i]->transform().position = _controlPoints[i];

		_controlPointMarkers[i]->transform().parent = &_curve->transform();

		RenderManager::AddShape(_controlPointMarkers[i]);
	}

	for (int i = 0; i < 8; ++i)
	{
		_slopeLines[i] = new RenderShape(slopeLineTemplate);

		_slopeLines[i]->transform().parent = &_curve->transform();

		RenderManager::AddShape(_slopeLines[i]);
	}

	_controlPointsActive = true;
	_wireframeActive = true;
	_baseActive = true;

	GeneratePlane();
}
Patch::~Patch()
{
	glDeleteBuffers(1, &_vbo);
	glDeleteBuffers(1, &_vaoTris);
	glDeleteBuffers(1, &_vaoLines);
	glDeleteBuffers(1, &_eboTris);
	glDeleteBuffers(1, &_eboLines);
}

void Patch::Update(float dt)
{
		UpdateShapes();

		_transform.position += _transform.linearVelocity * dt;
		_transform.rotation = glm::slerp(_transform.rotation, _transform.rotation * _transform.angularVelocity, dt);

		glm::mat4 translateMat = glm::translate(glm::mat4(), _transform.position);

		glm::mat4 rotateOriginMat = glm::translate(glm::mat4(), _transform.rotationOrigin);
		glm::mat4 rotateMat = rotateOriginMat * glm::mat4_cast(_transform.rotation) * glm::inverse(rotateOriginMat);

		glm::mat4 scaleOriginMat = glm::translate(glm::mat4(), _transform.scaleOrigin);
		glm::mat4 scaleMat = scaleOriginMat * glm::scale(glm::mat4(), _transform.scale) * glm::inverse(scaleOriginMat);

		glm::mat4 *parentModelMat = _transform.parent ? &_transform.parent->modelMat : &glm::mat4();

		_transform.modelMat = (*parentModelMat) * (translateMat * scaleMat* rotateMat);
}

void Patch::SetControlPoint(int controlPointIndex, glm::vec3 newPos)
{
	_controlPointMarkers[controlPointIndex]->transform().position = newPos;
}

Transform& Patch::transform() { return _transform; }

void  Patch::UpdateShapes()
{
	if (InputManager::spaceKey(true) && !InputManager::spaceKey())
	{
		if (_controlPointsActive && _wireframeActive && _baseActive)
			_controlPointsActive = false;
		else if (!_controlPointsActive && _wireframeActive && _baseActive)
			_wireframeActive = false;
		else if (!_controlPointsActive && !_wireframeActive && _baseActive)
		{
			_baseActive = false;
			_wireframeActive = true;
		}
		else
			_baseActive = _wireframeActive = _controlPointsActive = true;
	}

	_curveLines->active() = _wireframeActive;
	_curve->active() = _baseActive;

	for (int i = 0; i < 16; ++i)
	{
		_controlPoints[i] = _controlPointMarkers[i]->transform().position;
		_controlPointMarkers[i]->active() = _controlPointsActive;
	}

	// Update Lines

	std::vector<int> startVec = std::vector<int>();
	startVec.reserve(8);
	std::vector<int> endVec = std::vector<int>();
	endVec.reserve(8);

	// Front
	startVec.push_back(0);
	endVec.push_back(1);

	startVec.push_back(2);
	endVec.push_back(3);

	// Back
	startVec.push_back(14);
	endVec.push_back(15);

	startVec.push_back(12);
	endVec.push_back(13);

	// Left
	startVec.push_back(3);
	endVec.push_back(7);

	startVec.push_back(11);
	endVec.push_back(15);

	// Right
	startVec.push_back(0);
	endVec.push_back(4);

	startVec.push_back(8);
	endVec.push_back(12);

	glm::vec3 startingVec = glm::vec3(0.7071f, 0.7071f, 1.0f);

	int itr = 0;

	while (!startVec.empty())
	{
		glm::vec3 start = _controlPoints[startVec.back()];
		startVec.pop_back();
		glm::vec3 end = _controlPoints[endVec.back()];
		endVec.pop_back();

		glm::vec3 line = end - start;

		_slopeLines[itr]->transform().position = (start + end) / 2.0f;

		float angle = acosf(glm::dot(line, startingVec) / glm::length(line) / 1.414213f);
		glm::vec3 axis = glm::cross(line, startingVec);

		_slopeLines[itr]->transform().rotation = glm::angleAxis(angle, axis);

		_slopeLines[itr]->transform().scale = line / 2.0f;

		_slopeLines[itr]->active() = _controlPointsActive;

		++itr;
	}
	// Update the curve
	UpdateSurface();
}

void Patch::UpdateSurface()
{
	GLfloat  inc = 1.0f / ((float)NUM_VERTS - 1.0f);
	GLfloat t = 0.0f;

	GLfloat factors[NUM_VERTS][4];

	for (int i = 0; i < NUM_VERTS; ++i, t += inc)
	{
		GLfloat t_sqr = t * t;
		GLfloat t_inv = (1 - t);
		GLfloat t_inv_sqr = t_inv * t_inv;

		factors[i][0] = t_inv * t_inv_sqr;
		factors[i][1] = 3 * t * t_inv_sqr;
		factors[i][2] = 3 * t_sqr * t_inv;
		factors[i][3] = t * t_sqr;
	}
	
	glm::vec3 newControlPoints[4];
	for (int i = 0; i < NUM_VERTS; ++i)
	{
		newControlPoints[0] = factors[i][0] * _controlPoints[0] + factors[i][1] * _controlPoints[1] + factors[i][2] * _controlPoints[2] + factors[i][3] * _controlPoints[3];
		newControlPoints[1] = factors[i][0] * _controlPoints[4] + factors[i][1] * _controlPoints[5] + factors[i][2] * _controlPoints[6] + factors[i][3] * _controlPoints[7];
		newControlPoints[2] = factors[i][0] * _controlPoints[8] + factors[i][1] * _controlPoints[9] + factors[i][2] * _controlPoints[10] + factors[i][3] * _controlPoints[11];
		newControlPoints[3] = factors[i][0] * _controlPoints[12] + factors[i][1] * _controlPoints[13] + factors[i][2] * _controlPoints[14] + factors[i][3] * _controlPoints[15];
		for (int j = 0; j < NUM_VERTS; ++j)
		{
			glm::vec3 newPoint = factors[j][0] * newControlPoints[0] + factors[j][1] * newControlPoints[1] + factors[j][2] * newControlPoints[2] + factors[j][3] * newControlPoints[3];
			_verts[(j + (i * NUM_VERTS)) * 3] = newPoint.x;
			_verts[(j + (i * NUM_VERTS)) * 3 + 1] = newPoint.y;
			_verts[(j + (i * NUM_VERTS)) * 3 + 2] = newPoint.z;
		}
	}
	glBindBuffer(GL_VERTEX_ARRAY, _vaoTris);
	glBindBuffer(GL_ARRAY_BUFFER, _vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(_verts), (void*)&_verts, GL_DYNAMIC_DRAW);
}

void Patch::GeneratePlane()
{
	// Allocate vertices for the plane
	int vertNum = 0;
	GLfloat numVertsf = (GLfloat)NUM_VERTS;
	for (GLfloat i = 0.0f; i < numVertsf; i += 1.0f)
	{
		for (GLfloat j = 0.0f; j < numVertsf; j += 1.0f)
		{
			AddVert(0.0f, 0.0f, 0.0f, i / (numVertsf - 1.0f), j / (numVertsf - 1.0f), vertNum++);
		}
	}

	int cp = 0;
	float zOffset = 1.0f / 3.0f;
	float xOffset = 1.0f / 3.0f;
	glm::vec3 baseVec = glm::vec3(-0.5f, 0.0f, -0.5f);
	for (int row = 0; row < 4; ++row)
	{
		_controlPoints[cp++] = glm::vec3(baseVec.x, baseVec.y, baseVec.z + zOffset * row);
		_controlPoints[cp++] = glm::vec3(baseVec.x + xOffset, baseVec.y, baseVec.z + zOffset * row);
		_controlPoints[cp++] = glm::vec3(baseVec.x + xOffset * 2, baseVec.y, baseVec.z + zOffset * row);
		_controlPoints[cp++] = glm::vec3(baseVec.x + xOffset * 3, baseVec.y, baseVec.z + zOffset * row);
	}

	// Add elements for faces
	int faceNum = 0;
	int quadsPerRow = NUM_VERTS * (NUM_VERTS - 1);
	for (int i = 0; i < quadsPerRow; i += NUM_VERTS)
	{
		for (int j = 0; j < NUM_VERTS - 1; ++j)
		{
			AddFace(i + j, i + j + 1, i + NUM_VERTS + j, faceNum++);
			AddFace(i + j + 1, i + NUM_VERTS + j + 1, i + NUM_VERTS + j, faceNum++);
		}
	}
	glBindBuffer(GL_VERTEX_ARRAY, _vaoTris);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _eboTris);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(_elements), (void*)&_elements, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_VERTEX_ARRAY, _vaoLines);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _eboLines);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(_lineElements), (void*)&_lineElements, GL_DYNAMIC_DRAW);

	UpdateSurface();
}

void Patch::AddVert(GLfloat x, GLfloat y, GLfloat z, GLfloat u, GLfloat v, int vertNum)
{
	_verts[vertNum * 3] = x;
	_verts[vertNum * 3 + 1] = y;
	_verts[vertNum * 3 + 2] = z;
}

void Patch::AddFace(GLint a, GLint b, GLint c, int faceNum)
{
	_elements[faceNum * 3] = a;
	_elements[faceNum * 3 + 1] = b;
	_elements[faceNum * 3 + 2] = c;

	_lineElements[faceNum * 6] = a;
	_lineElements[faceNum * 6 + 1] = b;

	_lineElements[faceNum * 6 + 2] = b;
	_lineElements[faceNum * 6 + 3] = c;

	_lineElements[faceNum * 6 + 4] = c;
	_lineElements[faceNum * 6 + 5] = a;
}
