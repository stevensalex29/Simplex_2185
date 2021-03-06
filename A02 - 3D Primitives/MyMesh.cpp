#include "MyMesh.h"
void MyMesh::Init(void)
{
	m_bBinded = false;
	m_uVertexCount = 0;

	m_VAO = 0;
	m_VBO = 0;

	m_pShaderMngr = ShaderManager::GetInstance();
}
void MyMesh::Release(void)
{
	m_pShaderMngr = nullptr;

	if (m_VBO > 0)
		glDeleteBuffers(1, &m_VBO);

	if (m_VAO > 0)
		glDeleteVertexArrays(1, &m_VAO);

	m_lVertex.clear();
	m_lVertexPos.clear();
	m_lVertexCol.clear();
}
MyMesh::MyMesh()
{
	Init();
}
MyMesh::~MyMesh() { Release(); }
MyMesh::MyMesh(MyMesh& other)
{
	m_bBinded = other.m_bBinded;

	m_pShaderMngr = other.m_pShaderMngr;

	m_uVertexCount = other.m_uVertexCount;

	m_VAO = other.m_VAO;
	m_VBO = other.m_VBO;
}
MyMesh& MyMesh::operator=(MyMesh& other)
{
	if (this != &other)
	{
		Release();
		Init();
		MyMesh temp(other);
		Swap(temp);
	}
	return *this;
}
void MyMesh::Swap(MyMesh& other)
{
	std::swap(m_bBinded, other.m_bBinded);
	std::swap(m_uVertexCount, other.m_uVertexCount);

	std::swap(m_VAO, other.m_VAO);
	std::swap(m_VBO, other.m_VBO);

	std::swap(m_lVertex, other.m_lVertex);
	std::swap(m_lVertexPos, other.m_lVertexPos);
	std::swap(m_lVertexCol, other.m_lVertexCol);

	std::swap(m_pShaderMngr, other.m_pShaderMngr);
}
void MyMesh::CompleteMesh(vector3 a_v3Color)
{
	uint uColorCount = m_lVertexCol.size();
	for (uint i = uColorCount; i < m_uVertexCount; ++i)
	{
		m_lVertexCol.push_back(a_v3Color);
	}
}
void MyMesh::AddVertexPosition(vector3 a_v3Input)
{
	m_lVertexPos.push_back(a_v3Input);
	m_uVertexCount = m_lVertexPos.size();
}
void MyMesh::AddVertexColor(vector3 a_v3Input)
{
	m_lVertexCol.push_back(a_v3Input);
}
void MyMesh::CompileOpenGL3X(void)
{
	if (m_bBinded)
		return;

	if (m_uVertexCount == 0)
		return;

	CompleteMesh();

	for (uint i = 0; i < m_uVertexCount; i++)
	{
		//Position
		m_lVertex.push_back(m_lVertexPos[i]);
		//Color
		m_lVertex.push_back(m_lVertexCol[i]);
	}
	glGenVertexArrays(1, &m_VAO);//Generate vertex array object
	glGenBuffers(1, &m_VBO);//Generate Vertex Buffered Object

	glBindVertexArray(m_VAO);//Bind the VAO
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);//Bind the VBO
	glBufferData(GL_ARRAY_BUFFER, m_uVertexCount * 2 * sizeof(vector3), &m_lVertex[0], GL_STATIC_DRAW);//Generate space for the VBO

	// Position attribute
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(vector3), (GLvoid*)0);

	// Color attribute
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(vector3), (GLvoid*)(1 * sizeof(vector3)));

	m_bBinded = true;

	glBindVertexArray(0); // Unbind VAO
}
void MyMesh::Render(matrix4 a_mProjection, matrix4 a_mView, matrix4 a_mModel)
{
	// Use the buffer and shader
	GLuint nShader = m_pShaderMngr->GetShaderID("Basic");
	glUseProgram(nShader); 

	//Bind the VAO of this object
	glBindVertexArray(m_VAO);

	// Get the GPU variables by their name and hook them to CPU variables
	GLuint MVP = glGetUniformLocation(nShader, "MVP");
	GLuint wire = glGetUniformLocation(nShader, "wire");

	//Final Projection of the Camera
	matrix4 m4MVP = a_mProjection * a_mView * a_mModel;
	glUniformMatrix4fv(MVP, 1, GL_FALSE, glm::value_ptr(m4MVP));
	
	//Solid
	glUniform3f(wire, -1.0f, -1.0f, -1.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDrawArrays(GL_TRIANGLES, 0, m_uVertexCount);  

	//Wire
	glUniform3f(wire, 1.0f, 0.0f, 1.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glEnable(GL_POLYGON_OFFSET_LINE);
	glPolygonOffset(-1.f, -1.f);
	glDrawArrays(GL_TRIANGLES, 0, m_uVertexCount);
	glDisable(GL_POLYGON_OFFSET_LINE);

	glBindVertexArray(0);// Unbind VAO so it does not get in the way of other objects
}
void MyMesh::AddTri(vector3 a_vBottomLeft, vector3 a_vBottomRight, vector3 a_vTopLeft)
{
	//C
	//| \
	//A--B
	//This will make the triangle A->B->C 
	AddVertexPosition(a_vBottomLeft);
	AddVertexPosition(a_vBottomRight);
	AddVertexPosition(a_vTopLeft);
}
void MyMesh::AddQuad(vector3 a_vBottomLeft, vector3 a_vBottomRight, vector3 a_vTopLeft, vector3 a_vTopRight)
{
	//C--D
	//|  |
	//A--B
	//This will make the triangle A->B->C and then the triangle C->B->D
	AddVertexPosition(a_vBottomLeft);
	AddVertexPosition(a_vBottomRight);
	AddVertexPosition(a_vTopLeft);

	AddVertexPosition(a_vTopLeft);
	AddVertexPosition(a_vBottomRight);
	AddVertexPosition(a_vTopRight);
}
void MyMesh::GenerateCube(float a_fSize, vector3 a_v3Color)
{
	if (a_fSize < 0.01f)
		a_fSize = 0.01f;

	Release();
	Init();

	float fValue = a_fSize * 0.5f;
	//3--2
	//|  |
	//0--1

	vector3 point0(-fValue,-fValue, fValue); //0
	vector3 point1( fValue,-fValue, fValue); //1
	vector3 point2( fValue, fValue, fValue); //2
	vector3 point3(-fValue, fValue, fValue); //3

	vector3 point4(-fValue,-fValue,-fValue); //4
	vector3 point5( fValue,-fValue,-fValue); //5
	vector3 point6( fValue, fValue,-fValue); //6
	vector3 point7(-fValue, fValue,-fValue); //7

	//F
	AddQuad(point0, point1, point3, point2);

	//B
	AddQuad(point5, point4, point6, point7);

	//L
	AddQuad(point4, point0, point7, point3);

	//R
	AddQuad(point1, point5, point2, point6);

	//U
	AddQuad(point3, point2, point7, point6);

	//D
	AddQuad(point4, point5, point0, point1);

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateCuboid(vector3 a_v3Dimensions, vector3 a_v3Color)
{
	Release();
	Init();

	vector3 v3Value = a_v3Dimensions * 0.5f;
	//3--2
	//|  |
	//0--1
	vector3 point0(-v3Value.x, -v3Value.y, v3Value.z); //0
	vector3 point1(v3Value.x, -v3Value.y, v3Value.z); //1
	vector3 point2(v3Value.x, v3Value.y, v3Value.z); //2
	vector3 point3(-v3Value.x, v3Value.y, v3Value.z); //3

	vector3 point4(-v3Value.x, -v3Value.y, -v3Value.z); //4
	vector3 point5(v3Value.x, -v3Value.y, -v3Value.z); //5
	vector3 point6(v3Value.x, v3Value.y, -v3Value.z); //6
	vector3 point7(-v3Value.x, v3Value.y, -v3Value.z); //7

	//F
	AddQuad(point0, point1, point3, point2);

	//B
	AddQuad(point5, point4, point6, point7);

	//L
	AddQuad(point4, point0, point7, point3);

	//R
	AddQuad(point1, point5, point2, point6);

	//U
	AddQuad(point3, point2, point7, point6);

	//D
	AddQuad(point4, point5, point0, point1);

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateCone(float a_fRadius, float a_fHeight, int a_nSubdivisions, vector3 a_v3Color)
{
	if (a_fRadius < 0.01f)
		a_fRadius = 0.01f;

	if (a_fHeight < 0.01f)
		a_fHeight = 0.01f;

	if (a_nSubdivisions < 3)
		a_nSubdivisions = 3;
	if (a_nSubdivisions > 360)
		a_nSubdivisions = 360;

	Release();
	Init();

	// Cone creation code
	double angStep = 2.0 * PI / a_nSubdivisions;
	double centerX = 0.0f;
	double centerY = 0.0f;
	vector3 center = vector3(centerX, centerY, -a_fHeight/2);

	for (int i = 0; i < a_nSubdivisions; i++)
	{
		// Create base triangle
		double angle = angStep * i;
		double x = centerX + a_fRadius * cos(angle);
		double y = centerY + a_fRadius * sin(angle);

		double angle2 = angStep * (i + 1);  // Create vertices for all sides
		double x2 = centerX + a_fRadius * cos(angle2);
		double y2 = centerY + a_fRadius * sin(angle2);

		vector3 C = vector3(x, y, -a_fHeight / 2);
		vector3 B = vector3(x2, y2, -a_fHeight / 2);
		AddTri(center, C, B); // Use the center as the first point and the other new vertices as the other points

		// Create side face triangle
		vector3 D = vector3(centerX, centerY, a_fHeight/2);
		AddTri(C,D,B);
	}

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateCylinder(float a_fRadius, float a_fHeight, int a_nSubdivisions, vector3 a_v3Color)
{
	if (a_fRadius < 0.01f)
		a_fRadius = 0.01f;

	if (a_fHeight < 0.01f)
		a_fHeight = 0.01f;

	if (a_nSubdivisions < 3)
		a_nSubdivisions = 3;
	if (a_nSubdivisions > 360)
		a_nSubdivisions = 360;

	Release();
	Init();

	// Cylinder creation code
	double angStep = 2.0 * PI / a_nSubdivisions;
	double centerX = 0.0f;
	double centerY = 0.0f;
	vector3 bottomCenter = vector3(centerX, centerY, -a_fHeight / 2);
	vector3 topCenter = vector3(centerX, centerY, a_fHeight/2);

	for (int i = 0; i < a_nSubdivisions; i++)
	{
		// Create triangle for bottom base
		double bAngle = angStep * i;
		double bottomX = centerX + a_fRadius * cos(bAngle);
		double bottomY = centerY + a_fRadius * sin(bAngle);

		double bAngle2 = angStep * (i + 1);  // Create vertices for all sides
		double bottomX2 = centerX + a_fRadius * cos(bAngle2);
		double bottomY2 = centerY + a_fRadius * sin(bAngle2);

		vector3 bottomC = vector3(bottomX, bottomY, -a_fHeight / 2);
		vector3 bottomB = vector3(bottomX2, bottomY2, -a_fHeight / 2);
		AddTri(bottomB,bottomC,bottomCenter); // Use bottom center for center point at bottom

		// Create triangle for top base
		double tAngle = angStep * i;
		double topX = centerX + a_fRadius * cos(tAngle);
		double topY = centerY + a_fRadius * sin(tAngle);

		double tAngle2 = angStep * (i + 1);  // Create vertices for all sides
		double topX2 = centerX + a_fRadius * cos(tAngle2);
		double topY2 = centerY + a_fRadius * sin(tAngle2);

		vector3 topC = vector3(topX, topY, a_fHeight/2);
		vector3 topB = vector3(topX2, topY2, a_fHeight/2);
		AddTri(topCenter,topC,topB); // Use top center for center point at top

		// Create side face of cylinder
		AddQuad(bottomC,bottomB,topC,topB);
	}

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateTube(float a_fOuterRadius, float a_fInnerRadius, float a_fHeight, int a_nSubdivisions, vector3 a_v3Color)
{
	if (a_fOuterRadius < 0.01f)
		a_fOuterRadius = 0.01f;

	if (a_fInnerRadius < 0.005f)
		a_fInnerRadius = 0.005f;

	if (a_fInnerRadius > a_fOuterRadius)
		std::swap(a_fInnerRadius, a_fOuterRadius);

	if (a_fHeight < 0.01f)
		a_fHeight = 0.01f;

	if (a_nSubdivisions < 3)
		a_nSubdivisions = 3;
	if (a_nSubdivisions > 360)
		a_nSubdivisions = 360;

	Release();
	Init();

	// Tube creation code
	double angStep = 2.0 * PI / a_nSubdivisions;
	double centerX = 0.0f;
	double centerY = 0.0f;
	vector3 bottomCenter = vector3(centerX, centerY, -a_fHeight / 2);
	vector3 topCenter = vector3(centerX, centerY, a_fHeight/2);

	for (int i = 0; i < a_nSubdivisions; i++)
	{
		// Create quad for bottom base
		double bAngle = angStep * i;
		double bottomX = centerX + a_fOuterRadius * cos(bAngle);
		double bottomY = centerY + a_fOuterRadius * sin(bAngle);

		double bAngle2 = angStep * (i + 1);  // Create vertices for all sides
		double bottomX2 = centerX + a_fOuterRadius * cos(bAngle2);
		double bottomY2 = centerY + a_fOuterRadius * sin(bAngle2);

		double bottomX3 = centerX + a_fInnerRadius * cos(bAngle);
		double bottomY3 = centerY + a_fInnerRadius * sin(bAngle);

		double bottomX4 = centerX + a_fInnerRadius * cos(bAngle2);
		double bottomY4 = centerY + a_fInnerRadius * sin(bAngle2);

		vector3 bottomC = vector3(bottomX, bottomY, -a_fHeight / 2);
		vector3 bottomB = vector3(bottomX2, bottomY2, -a_fHeight / 2);
		vector3 bottomD = vector3(bottomX3, bottomY3, -a_fHeight / 2);
		vector3 bottomA = vector3(bottomX4, bottomY4, -a_fHeight / 2);
		AddQuad(bottomA,bottomB,bottomD,bottomC); // create quad for bottom tube base

		// Create quad for top base
		double tAngle = angStep * i;
		double topX = centerX + a_fOuterRadius * cos(tAngle);
		double topY = centerY + a_fOuterRadius * sin(tAngle);

		double tAngle2 = angStep * (i + 1);  // Create vertices for all sides
		double topX2 = centerX + a_fOuterRadius * cos(tAngle2);
		double topY2 = centerY + a_fOuterRadius * sin(tAngle2);

		double topX3 = centerX + a_fInnerRadius * cos(tAngle);
		double topY3 = centerY + a_fInnerRadius * sin(tAngle);

		double topX4 = centerX + a_fInnerRadius * cos(tAngle2);
		double topY4 = centerY + a_fInnerRadius * sin(tAngle2);

		vector3 topC = vector3(topX, topY, a_fHeight/2);
		vector3 topB = vector3(topX2, topY2, a_fHeight/2);
		vector3 topD = vector3(topX3, topY3, a_fHeight/2);
		vector3 topA = vector3(topX4, topY4, a_fHeight/2);
		AddQuad(topA, topD, topB, topC); // create quad for top tube base

		// Create side face of cylinder
		AddQuad(bottomC, bottomB, topC, topB);
		AddQuad(bottomA, bottomD, topA, topD);
	}

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateTorus(float a_fOuterRadius, float a_fInnerRadius, int a_nSubdivisionsA, int a_nSubdivisionsB, vector3 a_v3Color)
{
	if (a_fOuterRadius < 0.01f)
		a_fOuterRadius = 0.01f;

	if (a_fInnerRadius < 0.005f)
		a_fInnerRadius = 0.005f;

	if (a_fInnerRadius > a_fOuterRadius)
		std::swap(a_fInnerRadius, a_fOuterRadius);

	if (a_nSubdivisionsA < 3)
		a_nSubdivisionsA = 3;
	if (a_nSubdivisionsA > 360)
		a_nSubdivisionsA = 360;

	if (a_nSubdivisionsB < 3)
		a_nSubdivisionsB = 3;
	if (a_nSubdivisionsB > 360)
		a_nSubdivisionsB = 360;

	Release();
	Init();

	// Replace this with your code
	GenerateCube(a_fOuterRadius * 2.0f, a_v3Color);
	// -------------------------------

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateSphere(float a_fRadius, int a_nSubdivisions, vector3 a_v3Color)
{
	if (a_fRadius < 0.01f)
		a_fRadius = 0.01f;

	//Sets minimum and maximum of subdivisions
	if (a_nSubdivisions < 1)
	{
		GenerateCube(a_fRadius * 2.0f, a_v3Color);
		return;
	}
	if (a_nSubdivisions > 12)
		a_nSubdivisions = 12;

	Release();
	Init();

	// Create the sphere
	std::vector<double> ycuts;
	std::vector<std::vector<vector3>> rings;
	double centerX = 0.0f;
	double centerY = 0.0f;
	vector3 bottomCenter = vector3(centerX, centerY, -a_fRadius);
	vector3 topCenter = vector3(centerX, centerY, a_fRadius);
	double angledelta = PI / (a_nSubdivisions + 1);
	double cangle = 0;

	// Get all of the y cuts of the sphere
	for (int i = 0; i < a_nSubdivisions; i++) 
	{
		cangle += angledelta;
		ycuts.push_back(cos(cangle) * a_fRadius);
	}

	// Build the rings of points the circle
	for (int i = 0; i < ycuts.size(); i++) 
	{
		double height = ycuts[i];
		/*double b = a_fRadius * tan(angledelta / 2);
		double z = sqrt(b * b + a_fRadius * a_fRadius);
		double x = 0.5 * (z - a_fRadius);*/
		double cutradius = sqrt((a_fRadius*a_fRadius) - (height*height));
		std::vector<vector3> r;
		rings.push_back(r);
		double ringangledelta = PI * 2 / a_nSubdivisions;
		double ringangle = 0;
		for (int k = 0; k < a_nSubdivisions; k++)  // Put the rings together
		{
			vector3 p = vector3(cos(ringangle)*cutradius, sin(ringangle)*cutradius, height);
			rings[i].push_back(p);
			ringangle += ringangledelta;
		}
	}

	int ringSize = rings[0].size();

	// Build the top of the sphere
	for (int i = 0; i < ringSize; i++) 
	{
		AddTri(rings[0][i], rings[0][(i+1)%ringSize], topCenter);
	}

	// Build the center of the sphere
	for (int i = 0; i < rings.size() - 1; i++) 
	{
		for (int k = 0; k < rings[i].size(); k++) {
			AddQuad(rings[i + 1][k], rings[i + 1][(k + 1)%ringSize], rings[i][k], rings[i][(k + 1)%ringSize]);
		}
	}

	// Build the bottom of the sphere
	for (int i = 0; i < ringSize; i++) 
	{
		AddTri(rings[rings.size() - 1][i], bottomCenter, rings[rings.size() - 1][(i+1)%ringSize]);
	}
	
	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}