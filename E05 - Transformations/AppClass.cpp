#include "AppClass.h"
void Application::InitVariables(void)
{
	//init the mesh
	m_pMesh = new MyMesh();
	m_pMesh->GenerateCube(1.0f, C_BLACK);
}
void Application::Update(void)
{
	//Update the system so it knows how much time has passed since the last call
	m_pSystem->Update();

	//Is the arcball active?
	ArcBall();

	//Is the first person camera active?
	CameraRotation();
}
void Application::Display(void)
{
	// Clear the screen
	ClearScreen();

	matrix4 m4View = m_pCameraMngr->GetViewMatrix();
	matrix4 m4Projection = m_pCameraMngr->GetProjectionMatrix();
	
	
	static float value = 0.0f;
	matrix4 main = glm::translate(vector3(0.0f, 1.0f, 0.0f));

	//render all cubes
	///*for (int i = 0; i < 46; i++) {
	//	matrix4 m4Scale = glm::scale(main, vector3(.5f, .5f, .5f));
	//	matrix4 m4Translate = glm::translate(main, vector3(value, 0.0f, 0.0f));
	//	matrix4 m4Model = m4Scale * m4Translate;
	//	m_pMesh->Render(m4Projection, m4View, m4Model);
	//}*/

	
	// draw a skybox
	m_pMeshMngr->AddSkyboxToRenderList();
	
	//render list call
	m_uRenderCallCount = m_pMeshMngr->Render();

	//clear the render list
	m_pMeshMngr->ClearRenderList();
	
	//draw gui
	DrawGUI();
	
	//end the current frame (internally swaps the front and back buffers)
	m_pWindow->display();
	value += 0.01f;
}
void Application::Release(void)
{
	SafeDelete(m_pMesh);
	

	//release GUI
	ShutdownGUI();
}