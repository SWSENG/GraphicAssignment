#include <Windows.h>
#include <gl/GL.h>
#include <gl/GLU.h>
#include <math.h>

#define WINDOW_TITLE "Robot"
#define PI 3.142

float radius = 1.0f;
float angle = 0.01f;
int slices = 20, stacks = 20;
float boneLength = 1.0f;


LRESULT WINAPI WindowProcedure(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE) PostQuitMessage(0);
	default:
		break;
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}
//--------------------------------------------------------------------

bool initPixelFormat(HDC hdc)
{
	PIXELFORMATDESCRIPTOR pfd;
	ZeroMemory(&pfd, sizeof(PIXELFORMATDESCRIPTOR));

	pfd.cAlphaBits = 8;
	pfd.cColorBits = 32;
	pfd.cDepthBits = 24;
	pfd.cStencilBits = 0;

	pfd.dwFlags = PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW;

	pfd.iLayerType = PFD_MAIN_PLANE;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;

	// choose pixel format returns the number most similar pixel format available
	int n = ChoosePixelFormat(hdc, &pfd);

	// set pixel format returns whether it sucessfully set the pixel format
	if (SetPixelFormat(hdc, n, &pfd))
	{
		return true;
	}
	else
	{
		return false;
	}
}
//--------------------------------------------------------------------

void spinalCord(GLUquadricObj* var);

void drawLinePyramid() {
	glBegin(GL_LINE_LOOP);
	{
		// Bottom Face 
		glColor3f(1.0f, 1.0f, 0.0f);
		glVertex3f(-1.0f, -1.0f, 1.0f);
		glVertex3f(1.0f, -1.0f, 1.0f);
		glVertex3f(1.0f, -1.0f, -1.0f);
		glVertex3f(-1.0f, -1.0f, -1.0f);
		// Front Face 
		glVertex3f(-1.0f, -1.0f, 1.0f);
		glVertex3f(1.0f, -1.0f, 1.0f);
		glVertex3f(0.0f, 1.0f, 0.0f);
		// Left Face
		glVertex3f(-1.0f, -1.0f, -1.0f);
		glVertex3f(0.0f, 1.0f, 0.0f);
		// Right and Back Face
		glVertex3f(1.0f, -1.0f, 1.0f);
		glVertex3f(1.0f, -1.0f, -1.0f);
		glVertex3f(0.0f, 1.0f, 0.0f);
	}
	glEnd();
}

void drawJoint(GLUquadricObj* var) {
	glPushMatrix();
	{
		gluSphere(var, radius / 25.0f, slices, stacks);
	}
	glPopMatrix();
}

void drawBone(GLUquadricObj* var) {
	glPushMatrix();
	{
		gluQuadricDrawStyle(var, GLU_LINE);
		gluCylinder(var, 0.03, 0.03, boneLength, 10, 30);
	}
	glPopMatrix();
}

void spinalCord(GLUquadricObj* var)
{
	glPushMatrix();
	{//headjoint
		glTranslatef(0, 0.72, 0);
		drawJoint(var);
	}
	glPopMatrix();
	glPushMatrix();
	{//leftarmjoint
		glTranslatef(-0.25, 0.5, 0);
		drawJoint(var);
	}
	glPopMatrix();
	glPushMatrix();
	{//shoulderbone
		glTranslatef(-0.25, 0.5, 0);
		glRotatef(90, 0, 1, 0);
		boneLength = 0.5;
		drawBone(var);
	}
	glPopMatrix();
	glPushMatrix();
	{//rightarmjoint
		glTranslatef(0.25, 0.5, 0);
		drawJoint(var);
	}
	glPopMatrix();
	glPushMatrix();
	{//spinalCordbone
		glTranslatef(0, 0.7, 0);
		glRotatef(90, 1, 0, 0);
		boneLength = 0.5;
		drawBone(var);
	}
	glPopMatrix();
	glPushMatrix();
	{//waistjoint
		glTranslatef(0, 0.20, 0);
		drawJoint(var);
	}
	glPopMatrix();
	glPushMatrix();
	{//belowwaistbone
		glTranslatef(0, 0.18, 0);
		glRotatef(90, 1, 0, 0);
		boneLength = 0.25;
		drawBone(var);
	}
	glPopMatrix();
	glPushMatrix();
	{//kkb
		glScalef(0.03, 0.03, 0.03);
		glRotatef(180, 1, 0, 0);
		glTranslatef(0, 3, 0);
		drawLinePyramid();
	}
	glPopMatrix();
	glPushMatrix();
	{//rightlegjoint
		glTranslatef(0.07, 0, 0);
		drawJoint(var);
	}
	glPopMatrix();
	glPushMatrix();
	{//leftlegjoint
		glTranslatef(-0.07, 0, 0);
		drawJoint(var);
	}
	glPopMatrix();
}

void display()
{
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	GLUquadricObj* var = NULL;
	var = gluNewQuadric();

	spinalCord(var);
	gluDeleteQuadric(var);
}
//--------------------------------------------------------------------

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int nCmdShow)
{
	WNDCLASSEX wc;
	ZeroMemory(&wc, sizeof(WNDCLASSEX));

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.hInstance = GetModuleHandle(NULL);
	wc.lpfnWndProc = WindowProcedure;
	wc.lpszClassName = WINDOW_TITLE;
	wc.style = CS_HREDRAW | CS_VREDRAW;

	if (!RegisterClassEx(&wc)) return false;

	HWND hWnd = CreateWindow(WINDOW_TITLE, WINDOW_TITLE, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
		NULL, NULL, wc.hInstance, NULL);

	//--------------------------------
	//	Initialize window for OpenGL
	//--------------------------------

	HDC hdc = GetDC(hWnd);

	//	initialize pixel format for the window
	initPixelFormat(hdc);

	//	get an openGL context
	HGLRC hglrc = wglCreateContext(hdc);

	//	make context current
	if (!wglMakeCurrent(hdc, hglrc)) return false;

	//--------------------------------
	//	End initialization
	//--------------------------------

	ShowWindow(hWnd, nCmdShow);

	MSG msg;
	ZeroMemory(&msg, sizeof(msg));

	while (true)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT) break;

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		display();

		SwapBuffers(hdc);
	}

	UnregisterClass(WINDOW_TITLE, wc.hInstance);

	//hello


	return true;
}
//--------------------------------------------------------------------