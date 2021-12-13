#include <Windows.h>
#include <gl/GL.h>
#include <gl/GLU.h>
#include <math.h>

#define WINDOW_TITLE "Robot"
#define PI 3.142

//GL_POINT
//GL_LINES
//GL_LINE_STRIP
//GL_LINE_LOOP
//GL_POLYGON
//GL_QUADS
//GL_QUADS_STRIP
//GL_TRIANGLES
//GL_TRIANGLE_STRIP
//GL_TRIANGLE_FAN

//clyinder/cone
float radius = 1.0f;
float angle = 0.01f;
int slices = 20, stacks = 20;
float boneLength = 1.0f;


float rx = 1, ry = 0, rz = 0, rAngle = 0;
float tx = 0, ty = 0, tz = 0, tSpeed = 0.1;

float dif[] = { 1.0, 0.0, 0.0 };
float pos[] = { 0.0, 0.0, 0.0 };
int lightSwitch = 0;
int objShape = 0;
float rotate = 0.0;

float pry = 0, prSpeed = 0.3; //rotate y for the projection
float rotateAngle = 0;
float rotateLowerFinger = 0;
float rotateUpperFinger = 0;

/* Arm Up And Down Speed */
float leftArmUpSpeed = 0.0;
float rightArmUpSpeed = 0.0;

/* Projection Method */
boolean goPerspective = false;
boolean goOrtho = false;
boolean goBackOrigin = false;
int inWhatProjectionMode = 0; /* [0 - None] [1 - Ortho] [2 - Perspective] */

//texture
GLuint texture[10];
BITMAP BMP;
HBITMAP hBMP = NULL;

LRESULT WINAPI WindowProcedure(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE) PostQuitMessage(0);
		else if (wParam == VK_LEFT)
			glRotatef(1, 0, 1, 0);
		else if (wParam == VK_RIGHT)
			glRotatef(-1, 0, 1, 0);
		else if (wParam == VK_UP)
			glRotatef(-1, 1, 0, 0);
		else if (wParam == VK_DOWN)
			glRotatef(1, 1, 0, 0);
		else if (wParam == VK_SPACE)
		{
			if (lightSwitch == 0)
				lightSwitch = 1;
			else
				lightSwitch = 0;
		}
		else if (wParam == 'W')
			pos[1] += 0.1;
		else if (wParam == 'S')
			pos[1] -= 0.1;
		else if (wParam == 'A')
			pos[0] -= 0.1;
		else if (wParam == 'D')
			pos[0] += 0.1;
		else if (wParam == 'E')
			pos[2] -= 0.1;
		else if (wParam == 'Q')
			pos[2] += 0.1;

		/*else if (wParam == 0x31)
		{
			rotateLowerFinger -= 1;
		}
		else if (wParam == 0x32)
		{
			rotateUpperFinger -= 1;
		}*/
		else if (wParam == 'P')
			goPerspective = true;
		else if (wParam == 'R')
			goOrtho = true;
		else if (wParam == 'O')
			goBackOrigin = true;
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


void goPerspectiveView()
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glScalef(0.4, 0.4, 0.4);
	gluPerspective(30.0, 1.0, 0.5, 20);
	glFrustum(-2, 2, -2, 2, 1, 20);

}
void goOrthoView()
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	//glScalef(0.4, 0.4, 0.4);
	glOrtho(-2, 2, -0.8, 3.8, 1, -10);
}
void goBackOriginView()
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
}

void drawBody();
void hand();
void weapon();
void rightArm();
void leftArm();
void rightLeg();
void leftLeg();
void feet();

GLuint LoadTexture(LPCSTR filename)
{
	GLuint texture = 0;        //texture name
	//Step3
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	HBITMAP hBMP = (HBITMAP)LoadImage(GetModuleHandle(NULL), filename,
		IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADFROMFILE);
	GetObject(hBMP, sizeof(BMP), &BMP);

	//Step4
	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, BMP.bmWidth,
		BMP.bmHeight, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, BMP.bmBits);
	DeleteObject(hBMP);
	return texture;
}

void drawSphere(int drawNum, float sRad, float sSlice, float sStack) {
	GLUquadricObj* sphere = NULL;
	sphere = gluNewQuadric();

	if (drawNum == 1) GLU_LINE;
	if (drawNum == 2) GLU_FILL;

	gluQuadricDrawStyle(sphere, drawNum);
	gluSphere(sphere, sRad, sSlice, sStack);
	gluDeleteQuadric(sphere);
}
void drawCylinder(int drawNum, float cyBR, float cyTR, float cyH, float cySlice, float cyStack) {
	GLUquadricObj* cylinder = NULL;
	cylinder = gluNewQuadric();

	if (drawNum == 1) GLU_LINE;
	if (drawNum == 2) GLU_FILL;

	gluQuadricDrawStyle(cylinder, drawNum);
	gluCylinder(cylinder, cyBR, cyTR, cyH, cySlice, cyStack);
	gluDeleteQuadric(cylinder);
}
void drawCone(int drawNum, float cBR, float cH, float cSlice, float cStack) {
	GLUquadricObj* cylinder = NULL;
	cylinder = gluNewQuadric();

	if (drawNum == 1) GLU_LINE;
	if (drawNum == 2) GLU_FILL;

	gluQuadricDrawStyle(cylinder, drawNum);
	gluCylinder(cylinder, cBR, 0, cH, cSlice, cStack);
	gluDeleteQuadric(cylinder);
}

void drawBody()
{
	// upper body
	glPushMatrix();
	{
		// fill
		glPushMatrix();
		{
			texture[3] = LoadTexture("greyMetal.bmp");
			// front
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.025, 0.25, 0.0);
			glTexCoord2f(1, 1); glVertex3f(0.025, 0.25, 0.0);
			glTexCoord2f(1, 0); glVertex3f(0.025, 0.4, 0.05);
			glTexCoord2f(0, 0); glVertex3f(-0.025, 0.4, 0.05);
			glEnd();

			// left
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.025, 0.25, 0.0);
			glTexCoord2f(1, 1); glVertex3f(-0.025, 0.4, 0.05);
			glTexCoord2f(1, 0); glVertex3f(-0.145, 0.4, 0.15);
			glTexCoord2f(0, 0); glVertex3f(-0.175, 0.25, 0.15);
			glEnd();

			// back left
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.175, 0.25, 0.15);
			glTexCoord2f(1, 1); glVertex3f(-0.145, 0.4, 0.15);
			glTexCoord2f(1, 0); glVertex3f(-0.145, 0.4, 0.45);
			glTexCoord2f(0, 0); glVertex3f(-0.175, 0.25, 0.35);
			glEnd();

			// right
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(0.025, 0.25, 0.0);
			glTexCoord2f(1, 1); glVertex3f(0.025, 0.4, 0.05);
			glTexCoord2f(1, 0); glVertex3f(0.145, 0.4, 0.15);
			glTexCoord2f(0, 0); glVertex3f(0.175, 0.25, 0.15);
			glEnd();

			// back right
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(0.175, 0.25, 0.15);
			glTexCoord2f(1, 1); glVertex3f(0.145, 0.4, 0.15);
			glTexCoord2f(1, 0); glVertex3f(0.145, 0.4, 0.45);
			glTexCoord2f(0, 0); glVertex3f(0.175, 0.25, 0.35);
			glEnd();

			// behind
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.145, 0.4, 0.45);
			glTexCoord2f(1, 1); glVertex3f(0.145, 0.4, 0.45);
			glTexCoord2f(1, 0); glVertex3f(0.175, 0.25, 0.35);
			glTexCoord2f(0, 0); glVertex3f(-0.175, 0.25, 0.35);
			glEnd();

			// top
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(0.145, 0.4, 0.15);
			glTexCoord2f(1, 1); glVertex3f(0.025, 0.4, 0.05);
			glTexCoord2f(1, 0); glVertex3f(-0.025, 0.4, 0.05);
			glTexCoord2f(0, 0); glVertex3f(-0.145, 0.4, 0.15);
			glTexCoord2f(1, 1); glVertex3f(-0.145, 0.4, 0.45);
			glTexCoord2f(1, 0); glVertex3f(0.145, 0.4, 0.45);
			glEnd();

			// bottom
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(0.175, 0.25, 0.15);
			glTexCoord2f(1, 1); glVertex3f(0.025, 0.25, 0.0);
			glTexCoord2f(1, 0); glVertex3f(-0.025, 0.25, 0.);
			glTexCoord2f(0, 0); glVertex3f(-0.175, 0.25, 0.15);
			glTexCoord2f(1, 1); glVertex3f(-0.175, 0.25, 0.35);
			glTexCoord2f(1, 0); glVertex3f(0.175, 0.25, 0.35);
			glEnd();

			glDeleteTextures(1, &texture[3]);
			// left hand joint
			glPushMatrix();
			{
				glPushMatrix();
				{
					glTranslatef(0.15, 0.325, 0.25);
					glRotatef(90, 0, 1, 0);
					drawCone(2, 0.05, 0.15, 20, 20);
				}
				glPopMatrix();

				glPushMatrix();
				{
					glTranslatef(0.3, 0.325, 0.25);
					drawSphere(2, 0.06, 20, 20);
				}
				glPopMatrix();
			}
			glPopMatrix();

			// right hand joint
			glPushMatrix();
			{
				glColor3f(0.6, 0.6, 0.6);

				glPushMatrix();
				{
					glTranslatef(-0.15, 0.325, 0.25);
					glRotatef(270, 0, 1, 0);
					drawCone(2, 0.05, 0.15, 20, 20);
				}
				glPopMatrix();

				glPushMatrix();
				{
					glTranslatef(-0.3, 0.325, 0.25);
					drawSphere(2, 0.06, 20, 20);
				}
				glPopMatrix();
			}
			glPopMatrix();
		}
		glPopMatrix();

		// line
		glPushMatrix();
		{
			glLineWidth(2);

			// front
			glBegin(GL_LINE_LOOP);
			glVertex3f(-0.025, 0.25, 0.0);
			glVertex3f(0.025, 0.25, 0.0);
			glVertex3f(0.025, 0.4, 0.05);
			glVertex3f(-0.025, 0.4, 0.05);
			glEnd();

			// left
			glBegin(GL_LINE_LOOP);
			glVertex3f(-0.025, 0.25, 0.0);
			glVertex3f(-0.025, 0.4, 0.05);
			glVertex3f(-0.145, 0.4, 0.15);
			glVertex3f(-0.175, 0.25, 0.15);
			glEnd();

			// back left
			glBegin(GL_LINE_LOOP);
			glVertex3f(-0.175, 0.25, 0.15);
			glVertex3f(-0.145, 0.4, 0.15);
			glVertex3f(-0.145, 0.4, 0.45);
			glVertex3f(-0.175, 0.25, 0.35);
			glEnd();

			// right
			glBegin(GL_LINE_LOOP);
			glVertex3f(0.025, 0.25, 0.0);
			glVertex3f(0.025, 0.4, 0.05);
			glVertex3f(0.145, 0.4, 0.15);
			glVertex3f(0.175, 0.25, 0.15);
			glEnd();

			// back right
			glBegin(GL_LINE_LOOP);
			glVertex3f(0.175, 0.25, 0.15);
			glVertex3f(0.145, 0.4, 0.15);
			glVertex3f(0.145, 0.4, 0.45);
			glVertex3f(0.175, 0.25, 0.35);
			glEnd();

			// behind
			glBegin(GL_LINE_LOOP);
			glVertex3f(-0.145, 0.4, 0.45);
			glVertex3f(0.145, 0.4, 0.45);
			glVertex3f(0.175, 0.25, 0.35);
			glVertex3f(-0.175, 0.25, 0.35);
			glEnd();

			// top
			glBegin(GL_LINE_LOOP);
			glVertex3f(0.145, 0.4, 0.15);
			glVertex3f(0.025, 0.4, 0.05);
			glVertex3f(-0.025, 0.4, 0.05);
			glVertex3f(-0.145, 0.4, 0.15);
			glVertex3f(-0.145, 0.4, 0.45);
			glVertex3f(0.145, 0.4, 0.45);
			glEnd();

			//// bottom
			glBegin(GL_LINE_LOOP);
			glVertex3f(0.175, 0.25, 0.15);
			glVertex3f(0.025, 0.25, 0.0);
			glVertex3f(-0.025, 0.25, 0.);
			glVertex3f(-0.175, 0.25, 0.15);
			glVertex3f(-0.175, 0.25, 0.35);
			glVertex3f(0.175, 0.25, 0.35);
			glEnd();
		}
		glPopMatrix();
	}
	glPopMatrix();

	// middle body
	glPushMatrix();
	{
		// fill
		glPushMatrix();
		{
			texture[2] = LoadTexture("blackMetal.bmp");
			// front
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.025, 0.25, 0.05);
			glTexCoord2f(1, 1); glVertex3f(0.025, 0.25, 0.05);
			glTexCoord2f(1, 0); glVertex3f(0.025, 0.05, 0.1);
			glTexCoord2f(0, 0); glVertex3f(-0.025, 0.05, 0.1);
			glEnd();

			// left
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.025, 0.25, 0.05);
			glTexCoord2f(1, 1); glVertex3f(-0.025, 0.05, 0.1);
			glTexCoord2f(1, 0); glVertex3f(-0.085, 0.05, 0.15);
			glTexCoord2f(0, 0); glVertex3f(-0.115, 0.25, 0.2);
			glEnd();

			// back left
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.085, 0.05, 0.15);
			glTexCoord2f(1, 1); glVertex3f(-0.115, 0.25, 0.2);
			glTexCoord2f(1, 0); glVertex3f(0.0, 0.25, 0.3);
			glTexCoord2f(0, 0); glVertex3f(0.0, 0.05, 0.2);
			glEnd();

			// right
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(0.025, 0.25, 0.05);
			glTexCoord2f(1, 1); glVertex3f(0.025, 0.05, 0.1);
			glTexCoord2f(1, 0); glVertex3f(0.085, 0.05, 0.15);
			glTexCoord2f(0, 0); glVertex3f(0.115, 0.25, 0.2);
			glEnd();

			// back right
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(0.085, 0.05, 0.15);
			glTexCoord2f(1, 1); glVertex3f(0.115, 0.25, 0.2);
			glTexCoord2f(1, 0); glVertex3f(0.0, 0.25, 0.3);
			glTexCoord2f(0, 0); glVertex3f(0.0, 0.05, 0.2);
			glEnd();

			// top
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(0.115, 0.25, 0.2);
			glTexCoord2f(1, 1); glVertex3f(0.025, 0.25, 0.05);
			glTexCoord2f(1, 0); glVertex3f(-0.025, 0.25, 0.05);
			glTexCoord2f(0, 0); glVertex3f(-0.115, 0.25, 0.2);
			glTexCoord2f(0, 1); glVertex3f(0.0, 0.25, 0.3);
			glEnd();

			// bottom
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(0.085, 0.05, 0.15);
			glTexCoord2f(1, 1); glVertex3f(0.025, 0.05, 0.1);
			glTexCoord2f(1, 0); glVertex3f(-0.025, 0.05, 0.1);
			glTexCoord2f(0, 0); glVertex3f(-0.085, 0.05, 0.15);
			glTexCoord2f(0, 1); glVertex3f(0.0, 0.05, 0.2);
			glEnd();
			glDeleteTextures(1, &texture[2]);
		}
		glPopMatrix();

		// line
		glPushMatrix();
		{
			glLineWidth(2);

			// front
			glBegin(GL_LINE_LOOP);
			glVertex3f(-0.025, 0.25, 0.05);
			glVertex3f(0.025, 0.25, 0.05);
			glVertex3f(0.025, 0.05, 0.1);
			glVertex3f(-0.025, 0.05, 0.1);
			glEnd();

			// left
			glBegin(GL_LINE_LOOP);
			glVertex3f(-0.025, 0.25, 0.05);
			glVertex3f(-0.025, 0.05, 0.1);
			glVertex3f(-0.085, 0.05, 0.15);
			glVertex3f(-0.115, 0.25, 0.2);
			glEnd();

			// back left
			glBegin(GL_LINE_LOOP);
			glVertex3f(-0.085, 0.05, 0.15);
			glVertex3f(-0.115, 0.25, 0.2);
			glVertex3f(0.0, 0.25, 0.3);
			glVertex3f(0.0, 0.05, 0.2);
			glEnd();

			// right
			glBegin(GL_LINE_LOOP);
			glVertex3f(0.025, 0.25, 0.05);
			glVertex3f(0.025, 0.05, 0.1);
			glVertex3f(0.085, 0.05, 0.15);
			glVertex3f(0.115, 0.25, 0.2);
			glEnd();

			// back right
			glBegin(GL_LINE_LOOP);
			glVertex3f(0.085, 0.05, 0.15);
			glVertex3f(0.115, 0.25, 0.2);
			glVertex3f(0.0, 0.25, 0.3);
			glVertex3f(0.0, 0.05, 0.2);
			glEnd();

			// top
			glBegin(GL_LINE_LOOP);
			glVertex3f(0.115, 0.25, 0.2);
			glVertex3f(0.025, 0.25, 0.05);
			glVertex3f(-0.025, 0.25, 0.05);
			glVertex3f(-0.115, 0.25, 0.2);
			glVertex3f(0.0, 0.25, 0.3);
			glEnd();

			// bottom
			glBegin(GL_LINE_LOOP);
			glVertex3f(0.085, 0.05, 0.15);
			glVertex3f(0.025, 0.05, 0.1);
			glVertex3f(-0.025, 0.05, 0.1);
			glVertex3f(-0.085, 0.05, 0.15);
			glVertex3f(0.0, 0.05, 0.2);
			glEnd();
		}
		glPopMatrix();
	}
	glPopMatrix();

	// lower body
	glPushMatrix();
	{
		// fill
		glPushMatrix();
		{
			texture[3] = LoadTexture("greyMetal.bmp");
			// front
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.025, 0.05, 0.05);
			glTexCoord2f(1, 1); glVertex3f(0.025, 0.05, 0.05);
			glTexCoord2f(1, 0); glVertex3f(0.025, -0.05, 0.1);
			glTexCoord2f(0, 0); glVertex3f(-0.025, -0.05, 0.1);
			glEnd();

			// left
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.025, 0.05, 0.05);
			glTexCoord2f(1, 1); glVertex3f(-0.025, -0.05, 0.1);
			glTexCoord2f(1, 0); glVertex3f(-0.085, -0.05, 0.15);
			glTexCoord2f(0, 0); glVertex3f(-0.115, 0.05, 0.2);
			glEnd();

			// back left
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.085, -0.05, 0.15);
			glTexCoord2f(1, 1); glVertex3f(-0.115, 0.05, 0.2);
			glTexCoord2f(1, 0); glVertex3f(0.0, 0.05, 0.3);
			glTexCoord2f(0, 0); glVertex3f(0.0, -0.05, 0.2);
			glEnd();

			// right
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(0.025, 0.05, 0.05);
			glTexCoord2f(1, 1); glVertex3f(0.025, -0.05, 0.1);
			glTexCoord2f(1, 0); glVertex3f(0.085, -0.05, 0.15);
			glTexCoord2f(0, 0); glVertex3f(0.115, 0.05, 0.2);
			glEnd();

			// back right
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(0.085, -0.05, 0.15);
			glTexCoord2f(1, 1); glVertex3f(0.115, 0.05, 0.2);
			glTexCoord2f(1, 0); (0.0, 0.05, 0.3);
			glTexCoord2f(0, 0); glVertex3f(0.0, -0.05, 0.2);
			glEnd();

			// top
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(0.115, 0.05, 0.2);
			glTexCoord2f(1, 1); glVertex3f(0.025, 0.05, 0.05);
			glTexCoord2f(1, 0); glVertex3f(-0.025, 0.05, 0.05);
			glTexCoord2f(0, 0); glVertex3f(-0.115, 0.05, 0.2);
			glTexCoord2f(0, 1); glVertex3f(0.0, 0.05, 0.3);
			glEnd();

			// bottom
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(0.085, 0.05, 0.15);
			glTexCoord2f(1, 1); glVertex3f(0.025, 0.05, 0.1);
			glTexCoord2f(1, 0); glVertex3f(-0.025, 0.05, 0.1);
			glTexCoord2f(0, 0); glVertex3f(-0.085, 0.05, 0.15);
			glTexCoord2f(0, 1); glVertex3f(0.0, 0.05, 0.2);
			glEnd();
			glDeleteTextures(1, &texture[3]);
		}
		glPopMatrix();

		// line
		glPushMatrix();
		{
			glLineWidth(2);
			// front
			glBegin(GL_LINE_LOOP);
			glVertex3f(-0.025, 0.05, 0.05);
			glVertex3f(0.025, 0.05, 0.05);
			glVertex3f(0.025, -0.05, 0.1);
			glVertex3f(-0.025, -0.05, 0.1);
			glEnd();

			// left
			glBegin(GL_LINE_LOOP);
			glVertex3f(-0.025, 0.05, 0.05);
			glVertex3f(-0.025, -0.05, 0.1);
			glVertex3f(-0.085, -0.05, 0.15);
			glVertex3f(-0.115, 0.05, 0.2);
			glEnd();

			// back left
			glBegin(GL_LINE_LOOP);
			glVertex3f(-0.085, -0.05, 0.15);
			glVertex3f(-0.115, 0.05, 0.2);
			glVertex3f(0.0, 0.05, 0.3);
			glVertex3f(0.0, -0.05, 0.2);
			glEnd();

			// right
			glBegin(GL_LINE_LOOP);
			glVertex3f(0.025, 0.05, 0.05);
			glVertex3f(0.025, -0.05, 0.1);
			glVertex3f(0.085, -0.05, 0.15);
			glVertex3f(0.115, 0.05, 0.2);
			glEnd();

			// back right
			glBegin(GL_LINE_LOOP);
			glVertex3f(0.085, -0.05, 0.15);
			glVertex3f(0.115, 0.05, 0.2);
			glVertex3f(0.0, 0.05, 0.3);
			glVertex3f(0.0, -0.05, 0.2);
			glEnd();

			// top
			glBegin(GL_LINE_LOOP);
			glVertex3f(0.115, 0.05, 0.2);
			glVertex3f(0.025, 0.05, 0.05);
			glVertex3f(-0.025, 0.05, 0.05);
			glVertex3f(-0.115, 0.05, 0.2);
			glVertex3f(0.0, 0.05, 0.3);
			glEnd();

			// bottom
			glBegin(GL_LINE_LOOP);
			glVertex3f(0.085, 0.05, 0.15);
			glVertex3f(0.025, 0.05, 0.1);
			glVertex3f(-0.025, 0.05, 0.1);
			glVertex3f(-0.085, 0.05, 0.15);
			glVertex3f(0.0, 0.05, 0.2);
			glEnd();
		}
		glPopMatrix();
	}
	glPopMatrix();

	// left leg joint
	glPushMatrix();
	{
		glColor3f(0.6, 0.6, 0.6);

		glPushMatrix();
		{
			glTranslatef(-0.045, -0.025, 0.15);
			glRotatef(270, 0, 1, 0);
			glRotatef(50, 1, 0, 0);
			drawCone(2, 0.05, 0.175, 20, 20);
		}
		glPopMatrix();

		glPushMatrix();
		{
			glTranslatef(-0.155, -0.13, 0.15);
			drawSphere(2, 0.05, 20, 20);
		}
		glPopMatrix();
	}
	glPopMatrix();

	// right leg joint
	glPushMatrix();
	{
		glColor3f(0.6, 0.6, 0.6);

		glPushMatrix();
		{
			glTranslatef(0.045, -0.025, 0.15);
			glRotatef(90, 0, 1, 0);
			glRotatef(50, 1, 0, 0);
			drawCone(2, 0.05, 0.175, 20, 20);
		}
		glPopMatrix();

		glPushMatrix();
		{
			glTranslatef(0.155, -0.13, 0.15);
			drawSphere(2, 0.05, 20, 20);
		}
		glPopMatrix();
	}
	glPopMatrix();
}

void rightArm()
{	//bigarm
	glPushMatrix();
	{
		glTranslatef(-0.3, 0.14, 0.3);
		glPushMatrix();
		{
			glPushMatrix();
			{//1
				texture[2] = LoadTexture("blackMetal.bmp");
				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.1, 0.3, 0);
				glTexCoord2f(1, 1); glVertex3f(0, 0.3, 0);
				glTexCoord2f(1, 0); glVertex3f(0, 0, 0);
				glTexCoord2f(0, 0); glVertex3f(-0.1, 0, 0);
				glEnd();
				//2
				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.1, 0.3, 0);
				glTexCoord2f(1, 1); glVertex3f(-0.1, 0.3, -0.1);
				glTexCoord2f(1, 0); glVertex3f(0, 0.3, -0.1);
				glTexCoord2f(0, 0); glVertex3f(0, 0.3, 0);
				glEnd();
				//3
				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(0, 0.3, 0);
				glTexCoord2f(1, 1); glVertex3f(0, 0.3, -0.1);
				glTexCoord2f(1, 0); glVertex3f(0, 0, -0.1);
				glTexCoord2f(0, 0); glVertex3f(0, 0, 0);
				glEnd();
				//4
				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(0, 0, 0);
				glTexCoord2f(1, 1); glVertex3f(0, 0, -0.1);
				glTexCoord2f(1, 0); glVertex3f(-0.1, 0, -0.1);
				glTexCoord2f(0, 0); glVertex3f(-0.1, 0, 0);
				glEnd();
				//5
				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.1, 0, 0);
				glTexCoord2f(0, 1); glVertex3f(-0.1, 0, -0.1);
				glTexCoord2f(0, 1); glVertex3f(-0.1, 0.3, -0.1);
				glTexCoord2f(0, 1); glVertex3f(-0.1, 0.3, 0);
				glEnd();

				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.1, 0.3, -0.1);
				glTexCoord2f(1, 1); glVertex3f(0, 0.3, -0.1);
				glTexCoord2f(1, 0); glVertex3f(0, 0, -0.1);
				glTexCoord2f(0, 0); glVertex3f(-0.1, 0, -0.1);
				glEnd();

				glDeleteTextures(1, &texture[2]);
			}
			glPopMatrix();

			glPushMatrix();
			{//1
				texture[3] = LoadTexture("greyMetal.bmp");
				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.1, 0.6, 0);
				glTexCoord2f(1, 1); glVertex3f(-0.13, 0.6, 0);
				glTexCoord2f(1, 0); glVertex3f(-0.13, 0, 0);
				glTexCoord2f(0, 0); glVertex3f(-0.1, 0, 0);
				glEnd();
				//2
				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.1, 0.6, 0);
				glTexCoord2f(1, 1); glVertex3f(-0.1, 0.6, -0.1);
				glTexCoord2f(1, 0); glVertex3f(-0.13, 0.6, -0.1);
				glTexCoord2f(0, 0); glVertex3f(-0.13, 0.6, 0);
				glEnd();
				//3
				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.13, 0.6, 0);
				glTexCoord2f(1, 1); glVertex3f(-0.13, 0.6, -0.1);
				glTexCoord2f(1, 0); glVertex3f(-0.13, 0, -0.1);
				glTexCoord2f(0, 0); glVertex3f(-0.13, 0, 0);
				glEnd();
				//4
				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.13, 0, 0);
				glTexCoord2f(1, 1); glVertex3f(-0.13, 0, -0.1);
				glTexCoord2f(1, 0); glVertex3f(-0.1, 0, -0.1);
				glTexCoord2f(0, 0); glVertex3f(-0.1, 0, 0);
				glEnd();
				//5
				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.1, 0, 0);
				glTexCoord2f(1, 1); glVertex3f(-0.1, 0, -0.1);
				glTexCoord2f(1, 0); glVertex3f(-0.1, 0.6, -0.1);
				glTexCoord2f(0, 0); glVertex3f(-0.1, 0.6, 0);
				glEnd();

				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.1, 0.6, -0.1);
				glTexCoord2f(1, 1); glVertex3f(-0.13, 0.6, -0.1);
				glTexCoord2f(1, 0); glVertex3f(-0.13, 0, -0.1);
				glTexCoord2f(0, 0); glVertex3f(-0.1, 0, -0.1);
				glEnd();

				glDeleteTextures(1, &texture[3]);
			}
			glPopMatrix();

			glPushMatrix();
			{//1
				texture[3] = LoadTexture("greyMetal.bmp");
				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.1, 0.4, 0);
				glTexCoord2f(1, 1); glVertex3f(-0.16, 0.4, 0);
				glTexCoord2f(1, 0); glVertex3f(-0.16, 0, 0);
				glTexCoord2f(0, 0); glVertex3f(-0.1, 0, 0);
				glEnd();
				//2
				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.1, 0.4, 0);
				glTexCoord2f(1, 1); glVertex3f(-0.1, 0.4, -0.1);
				glTexCoord2f(1, 0); glVertex3f(-0.16, 0.4, -0.1);
				glTexCoord2f(0, 0); glVertex3f(-0.16, 0.4, 0);
				glEnd();
				//3
				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.16, 0.4, 0);
				glTexCoord2f(1, 1); glVertex3f(-0.16, 0.4, -0.1);
				glTexCoord2f(1, 0); glVertex3f(-0.16, 0, -0.1);
				glTexCoord2f(0, 0); glVertex3f(-0.16, 0, 0);
				glEnd();
				//4
				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.16, 0, 0);
				glTexCoord2f(1, 1); glVertex3f(-0.16, 0, -0.1);
				glTexCoord2f(1, 0); glVertex3f(-0.1, 0, -0.1);
				glTexCoord2f(0, 0); glVertex3f(-0.1, 0, 0);
				glEnd();
				//5
				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.1, 0, 0);
				glTexCoord2f(1, 1); glVertex3f(-0.1, 0, -0.1);
				glTexCoord2f(1, 0); glVertex3f(-0.1, 0.4, -0.1);
				glTexCoord2f(0, 0); glVertex3f(-0.1, 0.4, 0);
				glEnd();

				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glTexCoord2f(0, 1); glVertex3f(-0.1, 0.4, -0.1);
				glTexCoord2f(1, 1); glVertex3f(-0.16, 0.4, -0.1);
				glTexCoord2f(1, 0); glVertex3f(-0.16, 0, -0.1);
				glTexCoord2f(0, 0); glVertex3f(-0.1, 0, -0.1);
				glEnd();
				glDeleteTextures(1, &texture[3]);
			}
			glPopMatrix();

			glPushMatrix();
			{//1
				texture[2] = LoadTexture("blackMetal.bmp");
				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.1, 0.4, 0);
				glTexCoord2f(1, 1); glVertex3f(-0.16, 0.4, 0);
				glTexCoord2f(1, 0); glVertex3f(-0.16, 0, 0);
				glTexCoord2f(0, 0); glVertex3f(-0.1, 0, 0);
				glEnd();
				//2
				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.1, 0.4, 0);
				glTexCoord2f(1, 1); glVertex3f(-0.1, 0.4, -0.1);
				glTexCoord2f(1, 0); glVertex3f(-0.16, 0.4, -0.1);
				glTexCoord2f(0, 0); glVertex3f(-0.16, 0.4, 0);
				glEnd();
				//3
				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.16, 0.4, 0);
				glTexCoord2f(1, 1); glVertex3f(-0.16, 0.4, -0.1);
				glTexCoord2f(1, 0); glVertex3f(-0.16, 0, -0.1);
				glTexCoord2f(0, 0); glVertex3f(-0.16, 0, 0);
				glEnd();
				//4
				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.16, 0, 0);
				glTexCoord2f(1, 1); glVertex3f(-0.16, 0, -0.1);
				glTexCoord2f(1, 0); glVertex3f(-0.1, 0, -0.1);
				glTexCoord2f(0, 0); glVertex3f(-0.1, 0, 0);
				glEnd();
				//5
				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.1, 0, 0);
				glTexCoord2f(1, 1); glVertex3f(-0.1, 0, -0.1);
				glTexCoord2f(1, 0); glVertex3f(-0.1, 0.4, -0.1);
				glTexCoord2f(0, 0); glVertex3f(-0.1, 0.4, 0);
				glEnd();

				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.1, 0.4, -0.1);
				glTexCoord2f(1, 1); glVertex3f(-0.16, 0.4, -0.1);
				glTexCoord2f(1, 0); glVertex3f(-0.16, 0, -0.1);
				glTexCoord2f(0, 0); glVertex3f(-0.1, 0, -0.1);
				glEnd();

				glDeleteTextures(1, &texture[2]);
			}
			glPopMatrix();

			glPushMatrix();
			{//1
				texture[2] = LoadTexture("blackMetal.bmp");
				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.13, 0, 0);
				glTexCoord2f(1, 1); glVertex3f(-0.03, 0, 0);
				glTexCoord2f(1, 0); glVertex3f(-0.03, -0.3, 0);
				glTexCoord2f(0, 0); glVertex3f(-0.13, -0.3, 0);
				glEnd();
				//2
				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.13, 0, 0);
				glTexCoord2f(1, 1); glVertex3f(-0.13, 0, -0.1);
				glTexCoord2f(1, 0); glVertex3f(-0.03, 0, -0.1);
				glTexCoord2f(0, 0); glVertex3f(-0.03, 0, 0);
				glEnd();
				//3
				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.03, 0, 0);
				glTexCoord2f(1, 1); glVertex3f(-0.03, 0, -0.1);
				glTexCoord2f(1, 0); glVertex3f(-0.03, -0.3, -0.1);
				glTexCoord2f(0, 0); glVertex3f(-0.03, -0.3, 0);
				glEnd();
				//4
				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.03, -0.3, 0);
				glTexCoord2f(1, 1); glVertex3f(-0.03, -0.3, -0.1);
				glTexCoord2f(1, 0); glVertex3f(-0.13, -0.3, -0.1);
				glTexCoord2f(0, 0); glVertex3f(-0.13, -0.3, 0);
				glEnd();
				//5
				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.13, -0.3, 0);
				glTexCoord2f(1, 1); glVertex3f(-0.13, -0.3, -0.1);
				glTexCoord2f(1, 0); glVertex3f(-0.13, 0, -0.1);
				glTexCoord2f(0, 0); glVertex3f(-0.13, 0, 0);
				glEnd();

				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.13, 0, -0.1);
				glTexCoord2f(1, 1); glVertex3f(-0.03, 0, -0.1);
				glTexCoord2f(1, 0); glVertex3f(-0.03, -0.3, -0.1);
				glTexCoord2f(0, 0); glVertex3f(-0.13, -0.3, -0.1);
				glEnd();

				glDeleteTextures(1, &texture[2]);
			}
			glPopMatrix();

			glPushMatrix();
			{
				GLUquadricObj* armCylinder = NULL;
				armCylinder = gluNewQuadric();

				glPushMatrix();
				{
					texture[1] = LoadTexture("steel.bmp");
					glTranslatef(-0.15, -0.07, -0.05);
					glRotatef(90, 0, 1, 0);
					gluQuadricDrawStyle(armCylinder, GLU_FILL);
					gluQuadricTexture(armCylinder, true);
					gluCylinder(armCylinder, 0.05, 0.05, 0.07, 20, 20);
					gluDeleteQuadric(armCylinder);
					glDeleteTextures(1, &texture[1]);
				}
				glPopMatrix();
			}
			glPopMatrix();

			//smallarm
			glPushMatrix();
			{
				glTranslatef(0, -0.3, 0);
				glRotatef(rotateAngle, 1, 0, 0);
				glTranslatef(0, 0.3, 0);

				GLUquadricObj* rightArmJoint = NULL;
				rightArmJoint = gluNewQuadric();

				glPushMatrix();
				{
					texture[1] = LoadTexture("steel.bmp");
					glTranslatef(-0.08, -0.3, -0.05);
					gluQuadricDrawStyle(rightArmJoint, GLU_FILL);
					gluQuadricTexture(rightArmJoint, true);
					gluSphere(rightArmJoint, radius / 25.0f, slices, stacks);
					gluDeleteQuadric(rightArmJoint);
					glDeleteTextures(1, &texture[1]);
				}
				glPopMatrix();

				glPushMatrix();
				{
					texture[3] = LoadTexture("greyMetal.bmp");
					glBegin(GL_POLYGON);
					glTexCoord2f(0, 1); glVertex3f(-0.13, -0.32, 0);
					glTexCoord2f(1, 1); glVertex3f(-0.03, -0.32, 0);
					glTexCoord2f(1, 0); glVertex3f(-0.03, -0.5, 0);
					glTexCoord2f(0, 0); glVertex3f(-0.13, -0.5, 0);
					glEnd();
					//2
					glBegin(GL_POLYGON);
					glTexCoord2f(0, 1); glVertex3f(-0.13, -0.32, 0);
					glTexCoord2f(1, 1); glVertex3f(-0.13, -0.32, -0.1);
					glTexCoord2f(1, 0); glVertex3f(-0.03, -0.32, -0.1);
					glTexCoord2f(0, 0); glVertex3f(-0.03, -0.32, 0);
					glEnd();
					//3
					glBegin(GL_POLYGON);
					glTexCoord2f(0, 1); glVertex3f(-0.03, -0.32, 0);
					glTexCoord2f(1, 1); glVertex3f(-0.03, -0.32, -0.1);
					glTexCoord2f(1, 0); glVertex3f(-0.03, -0.5, -0.1);
					glTexCoord2f(0, 0); glVertex3f(-0.03, -0.5, 0);
					glEnd();
					//4
					glBegin(GL_POLYGON);
					glTexCoord2f(0, 1); glVertex3f(-0.03, -0.5, 0);
					glTexCoord2f(1, 1); glVertex3f(-0.03, -0.5, -0.1);
					glTexCoord2f(1, 0); glVertex3f(-0.13, -0.5, -0.1);
					glTexCoord2f(0, 0); glVertex3f(-0.13, -0.5, 0);
					glEnd();
					//5
					glBegin(GL_POLYGON);
					glTexCoord2f(0, 1); glVertex3f(-0.13, -0.5, 0);
					glTexCoord2f(1, 1); glVertex3f(-0.13, -0.5, -0.1);
					glTexCoord2f(1, 0); glVertex3f(-0.13, -0.32, -0.1);
					glTexCoord2f(0, 0); glVertex3f(-0.13, -0.32, 0);
					glEnd();

					glBegin(GL_POLYGON);
					glTexCoord2f(0, 1); glVertex3f(-0.13, -0.32, -0.1);
					glTexCoord2f(1, 1); glVertex3f(-0.03, -0.32, -0.1);
					glTexCoord2f(1, 0); glVertex3f(-0.03, -0.5, -0.1);
					glTexCoord2f(0, 0); glVertex3f(-0.13, -0.5, -0.1);
					glEnd();

					glDeleteTextures(1, &texture[3]);
				}
				glPopMatrix();

				glPushMatrix();
				{
					texture[3] = LoadTexture("greyMetal.bmp");

					glBegin(GL_POLYGON);
					glTexCoord2f(0, 1); glVertex3f(-0.13, -0.5, -0.1);
					glTexCoord2f(1, 1); glVertex3f(-0.03, -0.5, -0.1);
					glTexCoord2f(1, 0); glVertex3f(-0.03, -0.55, -0.05);
					glTexCoord2f(0, 0); glVertex3f(-0.13, -0.55, -0.05);
					glEnd();
					//2
					glBegin(GL_POLYGON);
					glTexCoord2f(0, 1); glVertex3f(-0.13, -0.5, -0.1);
					glTexCoord2f(1, 1); glVertex3f(-0.13, -0.5, 0.05);
					glTexCoord2f(1, 0); glVertex3f(-0.03, -0.5, 0.05);
					glTexCoord2f(0, 0); glVertex3f(-0.03, -0.5, -0.1);
					glEnd();
					//3
					glBegin(GL_POLYGON);
					glTexCoord2f(0, 1); glVertex3f(-0.03, -0.5, -0.1);
					glTexCoord2f(1, 1); glVertex3f(-0.03, -0.5, 0.05);
					glTexCoord2f(1, 0); glVertex3f(-0.03, -0.55, 0.05);
					glTexCoord2f(0, 0); glVertex3f(-0.03, -0.55, -0.05);
					glEnd();
					//4
					glBegin(GL_POLYGON);
					glTexCoord2f(0, 1); glVertex3f(-0.03, -0.55, -0.05);
					glTexCoord2f(1, 1); glVertex3f(-0.03, -0.55, 0.05);
					glTexCoord2f(1, 0); glVertex3f(-0.13, -0.55, 0.05);
					glTexCoord2f(0, 0); glVertex3f(-0.13, -0.55, -0.05);
					glEnd();
					//5
					glBegin(GL_POLYGON);
					glTexCoord2f(0, 1); glVertex3f(-0.13, -0.55, -0.05);
					glTexCoord2f(1, 1); glVertex3f(-0.13, -0.55, 0.05);
					glTexCoord2f(1, 0); glVertex3f(-0.13, -0.5, 0.05);
					glTexCoord2f(0, 0); glVertex3f(-0.13, -0.5, -0.1);
					glEnd();

					glBegin(GL_POLYGON);
					glTexCoord2f(0, 1); glVertex3f(-0.13, -0.5, 0.05);
					glTexCoord2f(1, 1); glVertex3f(-0.03, -0.5, 0.05);
					glTexCoord2f(1, 0); glVertex3f(-0.03, -0.55, 0.05);
					glTexCoord2f(0, 0); glVertex3f(-0.13, -0.55, 0.05);
					glEnd();

					glDeleteTextures(1, &texture[3]);
				}
				glPopMatrix();

				glPushMatrix();
				{
					texture[3] = LoadTexture("greyMetal.bmp");

					glBegin(GL_POLYGON);
					glTexCoord2f(0, 1); glVertex3f(-0.13, -0.4, 0);
					glTexCoord2f(1, 1); glVertex3f(-0.03, -0.4, 0);
					glTexCoord2f(1, 0); glVertex3f(-0.03, -0.55, 0);
					glTexCoord2f(0, 0); glVertex3f(-0.13, -0.55, 0);
					glEnd();
					//2
					glBegin(GL_POLYGON);
					glTexCoord2f(0, 1); glVertex3f(-0.13, -0.4, 0);
					glTexCoord2f(1, 1); glVertex3f(-0.13, -0.3, 0.1);
					glTexCoord2f(1, 0); glVertex3f(-0.03, -0.3, 0.1);
					glTexCoord2f(0, 0); glVertex3f(-0.03, -0.4, 0);
					glEnd();
					//3
					glBegin(GL_POLYGON);
					glTexCoord2f(0, 1); glVertex3f(-0.03, -0.4, 0);
					glTexCoord2f(1, 1); glVertex3f(-0.03, -0.3, 0.1);
					glTexCoord2f(1, 0); glVertex3f(-0.03, -0.55, 0.1);
					glTexCoord2f(0, 0); glVertex3f(-0.03, -0.55, 0);
					glEnd();
					//4
					glBegin(GL_POLYGON);
					glTexCoord2f(0, 1); glVertex3f(-0.03, -0.55, 0);
					glTexCoord2f(1, 1); glVertex3f(-0.03, -0.55, 0.1);
					glTexCoord2f(1, 0); glVertex3f(-0.13, -0.55, 0.1);
					glTexCoord2f(0, 0); glVertex3f(-0.13, -0.55, 0);
					glEnd();
					//5
					glBegin(GL_POLYGON);
					glTexCoord2f(0, 1); glVertex3f(-0.13, -0.55, 0);
					glTexCoord2f(1, 1); glVertex3f(-0.13, -0.55, 0.1);
					glTexCoord2f(1, 0); glVertex3f(-0.13, -0.3, 0.1);
					glTexCoord2f(0, 0); glVertex3f(-0.13, -0.4, 0);
					glEnd();

					glBegin(GL_POLYGON);
					glTexCoord2f(0, 1); glVertex3f(-0.13, -0.3, 0.1);
					glTexCoord2f(1, 1); glVertex3f(-0.03, -0.3, 0.1);
					glTexCoord2f(1, 0); glVertex3f(-0.03, -0.55, 0.1);
					glTexCoord2f(0, 0); glVertex3f(-0.13, -0.55, 0.1);
					glEnd();

					glDeleteTextures(1, &texture[3]);
				}
				glPopMatrix();

				glPushMatrix();
				{
					texture[3] = LoadTexture("greyMetal.bmp");
					glBegin(GL_POLYGON);
					glTexCoord2f(0, 1); glVertex3f(-0.13, -0.55, -0.05);
					glTexCoord2f(1, 1); glVertex3f(-0.03, -0.55, -0.05);
					glTexCoord2f(1, 0); glVertex3f(-0.03, -0.65, -0.05);
					glTexCoord2f(0, 0); glVertex3f(-0.13, -0.65, -0.05);
					glEnd();
					//2
					glBegin(GL_POLYGON);
					glTexCoord2f(0, 1); glVertex3f(-0.13, -0.55, -0.05);
					glTexCoord2f(1, 1); glVertex3f(-0.13, -0.55, 0.05);
					glTexCoord2f(1, 0); glVertex3f(-0.03, -0.55, 0.05);
					glTexCoord2f(0, 0); glVertex3f(-0.03, -0.55, -0.05);
					glEnd();
					//3
					glBegin(GL_POLYGON);
					glTexCoord2f(0, 1); glVertex3f(-0.03, -0.55, -0.05);
					glTexCoord2f(1, 1); (-0.03, -0.55, 0.1);
					glTexCoord2f(1, 0); glVertex3f(-0.03, -0.65, 0.05);
					glTexCoord2f(0, 0); glVertex3f(-0.03, -0.65, -0.05);
					glEnd();
					//4
					glBegin(GL_POLYGON);
					glTexCoord2f(0, 1); glVertex3f(-0.03, -0.65, -0.05);
					glTexCoord2f(1, 1); glVertex3f(-0.03, -0.65, 0.05);
					glTexCoord2f(1, 0); glVertex3f(-0.13, -0.65, 0.05);
					glTexCoord2f(0, 0); glVertex3f(-0.13, -0.65, -0.05);
					glEnd();
					//5
					glBegin(GL_POLYGON);
					glTexCoord2f(0, 1); glVertex3f(-0.13, -0.65, -0.05);
					glTexCoord2f(1, 1); glVertex3f(-0.13, -0.65, 0.05);
					glTexCoord2f(1, 0); glVertex3f(-0.13, -0.55, 0.1);
					glTexCoord2f(0, 0); glVertex3f(-0.13, -0.55, -0.05);
					glEnd();

					glBegin(GL_POLYGON);
					glTexCoord2f(0, 1); glVertex3f(-0.13, -0.55, 0.1);
					glTexCoord2f(1, 1); glVertex3f(-0.03, -0.55, 0.1);
					glTexCoord2f(1, 0); glVertex3f(-0.03, -0.65, 0.05);
					glTexCoord2f(0, 0); glVertex3f(-0.13, -0.65, 0.05);
					glEnd();

					glDeleteTextures(1, &texture[3]);
				}
				glPopMatrix();

				glPushMatrix();
				{
					texture[3] = LoadTexture("greyMetal.bmp");
					glBegin(GL_POLYGON);
					glTexCoord2f(0, 1); glVertex3f(-0.13, -0.65, -0.05);
					glTexCoord2f(1, 1); glVertex3f(-0.03, -0.65, -0.05);
					glTexCoord2f(1, 0); glVertex3f(-0.03, -0.8, -0.05);
					glTexCoord2f(0, 0); glVertex3f(-0.13, -0.8, -0.05);
					glEnd();
					//2
					glBegin(GL_POLYGON);
					glTexCoord2f(0, 1); glVertex3f(-0.13, -0.65, -0.05);
					glTexCoord2f(1, 1); glVertex3f(-0.13, -0.65, 0.05);
					glTexCoord2f(1, 0); glVertex3f(-0.03, -0.65, 0.05);
					glTexCoord2f(0, 0); glVertex3f(-0.03, -0.65, -0.05);
					glEnd();
					//3
					glBegin(GL_POLYGON);
					glTexCoord2f(0, 1); glVertex3f(-0.03, -0.65, -0.05);
					glTexCoord2f(1, 1); glVertex3f(-0.03, -0.65, 0.05);
					glTexCoord2f(1, 0); glVertex3f(-0.03, -0.8, 0.05);
					glTexCoord2f(0, 0); glVertex3f(-0.03, -0.8, -0.05);
					glEnd();
					//4
					glBegin(GL_POLYGON);
					glTexCoord2f(0, 1); glVertex3f(-0.03, -0.8, -0.05);
					glTexCoord2f(1, 1); glVertex3f(-0.03, -0.8, 0.05);
					glTexCoord2f(1, 0); glVertex3f(-0.13, -0.8, 0.05);
					glTexCoord2f(0, 0); glVertex3f(-0.13, -0.8, -0.05);
					glEnd();
					//5
					glBegin(GL_POLYGON);
					glTexCoord2f(0, 1); glVertex3f(-0.13, -0.8, -0.05);
					glTexCoord2f(1, 1); glVertex3f(-0.13, -0.8, 0.05);
					glTexCoord2f(1, 0); glVertex3f(-0.13, -0.65, 0.05);
					glTexCoord2f(0, 0); glVertex3f(-0.13, -0.65, -0.05);
					glEnd();

					glBegin(GL_POLYGON);
					glTexCoord2f(0, 1); glVertex3f(-0.13, -0.65, 0.05);
					glTexCoord2f(1, 1); glVertex3f(-0.03, -0.65, 0.05);
					glTexCoord2f(1, 0); glVertex3f(-0.03, -0.8, 0.05);
					glTexCoord2f(0, 0); glVertex3f(-0.13, -0.8, 0.05);
					glEnd();
					glDeleteTextures(1, &texture[3]);

					glPushMatrix();
					{
						texture[1] = LoadTexture("steel.bmp");
						//hand
						GLUquadricObj* rightHandJoint = NULL;
						rightHandJoint = gluNewQuadric();

						glTranslatef(-0.08, -0.8, 0);
						gluQuadricDrawStyle(rightHandJoint, GLU_FILL);
						gluQuadricTexture(rightHandJoint, true);
						gluSphere(rightHandJoint, radius / 25.0f, slices, stacks);
						gluDeleteQuadric(rightHandJoint);

						glDeleteTextures(1, &texture[1]);
					}
					glPopMatrix();
				}
				glPopMatrix();

				glPushMatrix();
				{
					glTranslatef(-0.05, -0.92, -0.05);
					glRotatef(90, 0, 1, 0);
					hand();
				}
				glPopMatrix();

				glPushMatrix();
				{
					glTranslatef(-0.19, -1, 0);
					glRotatef(-90, 0, 1, 0);
					weapon();
				}
				glPopMatrix();
			}
			glPopMatrix();

		}
		glPopMatrix();
	}
	glPopMatrix();
}

void leftArm()
{	//bigarm
	glPushMatrix();
	{
		glTranslatef(0.3, 0.14, 0.2);
		glRotatef(180, 0, 1, 0);
		glPushMatrix();
		{//1
			texture[2] = LoadTexture("blackMetal.bmp");
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.1, 0.3, 0);
			glTexCoord2f(1, 1); glVertex3f(0, 0.3, 0);
			glTexCoord2f(1, 0); glVertex3f(0, 0, 0);
			glTexCoord2f(0, 0); glVertex3f(-0.1, 0, 0);
			glEnd();
			//2
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.1, 0.3, 0);
			glTexCoord2f(1, 1); glVertex3f(-0.1, 0.3, -0.1);
			glTexCoord2f(1, 0); glVertex3f(0, 0.3, -0.1);
			glTexCoord2f(0, 0); glVertex3f(0, 0.3, 0);
			glEnd();
			//3
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(0, 0.3, 0);
			glTexCoord2f(1, 1); glVertex3f(0, 0.3, -0.1);
			glTexCoord2f(1, 0); glVertex3f(0, 0, -0.1);
			glTexCoord2f(0, 0); glVertex3f(0, 0, 0);
			glEnd();
			//4
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(0, 0, 0);
			glTexCoord2f(1, 1); glVertex3f(0, 0, -0.1);
			glTexCoord2f(1, 0); glVertex3f(-0.1, 0, -0.1);
			glTexCoord2f(0, 0); glVertex3f(-0.1, 0, 0);
			glEnd();
			//5
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.1, 0, 0);
			glTexCoord2f(0, 1); glVertex3f(-0.1, 0, -0.1);
			glTexCoord2f(0, 1); glVertex3f(-0.1, 0.3, -0.1);
			glTexCoord2f(0, 1); glVertex3f(-0.1, 0.3, 0);
			glEnd();

			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.1, 0.3, -0.1);
			glTexCoord2f(1, 1); glVertex3f(0, 0.3, -0.1);
			glTexCoord2f(1, 0); glVertex3f(0, 0, -0.1);
			glTexCoord2f(0, 0); glVertex3f(-0.1, 0, -0.1);
			glEnd();

			glDeleteTextures(1, &texture[2]);
		}
		glPopMatrix();

		glPushMatrix();
		{//1
			texture[3] = LoadTexture("greyMetal.bmp");
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.1, 0.6, 0);
			glTexCoord2f(1, 1); glVertex3f(-0.13, 0.6, 0);
			glTexCoord2f(1, 0); glVertex3f(-0.13, 0, 0);
			glTexCoord2f(0, 0); glVertex3f(-0.1, 0, 0);
			glEnd();
			//2
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.1, 0.6, 0);
			glTexCoord2f(1, 1); glVertex3f(-0.1, 0.6, -0.1);
			glTexCoord2f(1, 0); glVertex3f(-0.13, 0.6, -0.1);
			glTexCoord2f(0, 0); glVertex3f(-0.13, 0.6, 0);
			glEnd();
			//3
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.13, 0.6, 0);
			glTexCoord2f(1, 1); glVertex3f(-0.13, 0.6, -0.1);
			glTexCoord2f(1, 0); glVertex3f(-0.13, 0, -0.1);
			glTexCoord2f(0, 0); glVertex3f(-0.13, 0, 0);
			glEnd();
			//4
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.13, 0, 0);
			glTexCoord2f(1, 1); glVertex3f(-0.13, 0, -0.1);
			glTexCoord2f(1, 0); glVertex3f(-0.1, 0, -0.1);
			glTexCoord2f(0, 0); glVertex3f(-0.1, 0, 0);
			glEnd();
			//5
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.1, 0, 0);
			glTexCoord2f(1, 1); glVertex3f(-0.1, 0, -0.1);
			glTexCoord2f(1, 0); glVertex3f(-0.1, 0.6, -0.1);
			glTexCoord2f(0, 0); glVertex3f(-0.1, 0.6, 0);
			glEnd();

			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.1, 0.6, -0.1);
			glTexCoord2f(1, 1); glVertex3f(-0.13, 0.6, -0.1);
			glTexCoord2f(1, 0); glVertex3f(-0.13, 0, -0.1);
			glTexCoord2f(0, 0); glVertex3f(-0.1, 0, -0.1);
			glEnd();

			glDeleteTextures(1, &texture[3]);
		}
		glPopMatrix();

		glPushMatrix();
		{//1
			texture[3] = LoadTexture("greyMetal.bmp");
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.1, 0.4, 0);
			glTexCoord2f(1, 1); glVertex3f(-0.16, 0.4, 0);
			glTexCoord2f(1, 0); glVertex3f(-0.16, 0, 0);
			glTexCoord2f(0, 0); glVertex3f(-0.1, 0, 0);
			glEnd();
			//2
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.1, 0.4, 0);
			glTexCoord2f(1, 1); glVertex3f(-0.1, 0.4, -0.1);
			glTexCoord2f(1, 0); glVertex3f(-0.16, 0.4, -0.1);
			glTexCoord2f(0, 0); glVertex3f(-0.16, 0.4, 0);
			glEnd();
			//3
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.16, 0.4, 0);
			glTexCoord2f(1, 1); glVertex3f(-0.16, 0.4, -0.1);
			glTexCoord2f(1, 0); glVertex3f(-0.16, 0, -0.1);
			glTexCoord2f(0, 0); glVertex3f(-0.16, 0, 0);
			glEnd();
			//4
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.16, 0, 0);
			glTexCoord2f(1, 1); glVertex3f(-0.16, 0, -0.1);
			glTexCoord2f(1, 0); glVertex3f(-0.1, 0, -0.1);
			glTexCoord2f(0, 0); glVertex3f(-0.1, 0, 0);
			glEnd();
			//5
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.1, 0, 0);
			glTexCoord2f(1, 1); glVertex3f(-0.1, 0, -0.1);
			glTexCoord2f(1, 0); glVertex3f(-0.1, 0.4, -0.1);
			glTexCoord2f(0, 0); glVertex3f(-0.1, 0.4, 0);
			glEnd();

			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glTexCoord2f(0, 1); glVertex3f(-0.1, 0.4, -0.1);
			glTexCoord2f(1, 1); glVertex3f(-0.16, 0.4, -0.1);
			glTexCoord2f(1, 0); glVertex3f(-0.16, 0, -0.1);
			glTexCoord2f(0, 0); glVertex3f(-0.1, 0, -0.1);
			glEnd();
			glDeleteTextures(1, &texture[3]);
		}
		glPopMatrix();

		glPushMatrix();
		{//1
			texture[2] = LoadTexture("blackMetal.bmp");
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.1, 0.4, 0);
			glTexCoord2f(1, 1); glVertex3f(-0.16, 0.4, 0);
			glTexCoord2f(1, 0); glVertex3f(-0.16, 0, 0);
			glTexCoord2f(0, 0); glVertex3f(-0.1, 0, 0);
			glEnd();
			//2
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.1, 0.4, 0);
			glTexCoord2f(1, 1); glVertex3f(-0.1, 0.4, -0.1);
			glTexCoord2f(1, 0); glVertex3f(-0.16, 0.4, -0.1);
			glTexCoord2f(0, 0); glVertex3f(-0.16, 0.4, 0);
			glEnd();
			//3
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.16, 0.4, 0);
			glTexCoord2f(1, 1); glVertex3f(-0.16, 0.4, -0.1);
			glTexCoord2f(1, 0); glVertex3f(-0.16, 0, -0.1);
			glTexCoord2f(0, 0); glVertex3f(-0.16, 0, 0);
			glEnd();
			//4
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.16, 0, 0);
			glTexCoord2f(1, 1); glVertex3f(-0.16, 0, -0.1);
			glTexCoord2f(1, 0); glVertex3f(-0.1, 0, -0.1);
			glTexCoord2f(0, 0); glVertex3f(-0.1, 0, 0);
			glEnd();
			//5
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.1, 0, 0);
			glTexCoord2f(1, 1); glVertex3f(-0.1, 0, -0.1);
			glTexCoord2f(1, 0); glVertex3f(-0.1, 0.4, -0.1);
			glTexCoord2f(0, 0); glVertex3f(-0.1, 0.4, 0);
			glEnd();

			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.1, 0.4, -0.1);
			glTexCoord2f(1, 1); glVertex3f(-0.16, 0.4, -0.1);
			glTexCoord2f(1, 0); glVertex3f(-0.16, 0, -0.1);
			glTexCoord2f(0, 0); glVertex3f(-0.1, 0, -0.1);
			glEnd();

			glDeleteTextures(1, &texture[2]);
		}
		glPopMatrix();

		glPushMatrix();
		{//1
			texture[2] = LoadTexture("blackMetal.bmp");
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.13, 0, 0);
			glTexCoord2f(1, 1); glVertex3f(-0.03, 0, 0);
			glTexCoord2f(1, 0); glVertex3f(-0.03, -0.3, 0);
			glTexCoord2f(0, 0); glVertex3f(-0.13, -0.3, 0);
			glEnd();
			//2
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.13, 0, 0);
			glTexCoord2f(1, 1); glVertex3f(-0.13, 0, -0.1);
			glTexCoord2f(1, 0); glVertex3f(-0.03, 0, -0.1);
			glTexCoord2f(0, 0); glVertex3f(-0.03, 0, 0);
			glEnd();
			//3
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.03, 0, 0);
			glTexCoord2f(1, 1); glVertex3f(-0.03, 0, -0.1);
			glTexCoord2f(1, 0); glVertex3f(-0.03, -0.3, -0.1);
			glTexCoord2f(0, 0); glVertex3f(-0.03, -0.3, 0);
			glEnd();
			//4
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.03, -0.3, 0);
			glTexCoord2f(1, 1); glVertex3f(-0.03, -0.3, -0.1);
			glTexCoord2f(1, 0); glVertex3f(-0.13, -0.3, -0.1);
			glTexCoord2f(0, 0); glVertex3f(-0.13, -0.3, 0);
			glEnd();
			//5
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.13, -0.3, 0);
			glTexCoord2f(1, 1); glVertex3f(-0.13, -0.3, -0.1);
			glTexCoord2f(1, 0); glVertex3f(-0.13, 0, -0.1);
			glTexCoord2f(0, 0); glVertex3f(-0.13, 0, 0);
			glEnd();

			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.13, 0, -0.1);
			glTexCoord2f(1, 1); glVertex3f(-0.03, 0, -0.1);
			glTexCoord2f(1, 0); glVertex3f(-0.03, -0.3, -0.1);
			glTexCoord2f(0, 0); glVertex3f(-0.13, -0.3, -0.1);
			glEnd();

			glDeleteTextures(1, &texture[2]);
		}
		glPopMatrix();

		glPushMatrix();
		{
			GLUquadricObj* armCylinder = NULL;
			armCylinder = gluNewQuadric();

			glPushMatrix();
			{
				texture[1] = LoadTexture("steel.bmp");
				glTranslatef(-0.15, -0.07, -0.05);
				glRotatef(90, 0, 1, 0);
				gluQuadricDrawStyle(armCylinder, GLU_FILL);
				gluQuadricTexture(armCylinder, true);
				gluCylinder(armCylinder, 0.05, 0.05, 0.07, 20, 20);
				gluDeleteQuadric(armCylinder);
				glDeleteTextures(1, &texture[1]);
			}
			glPopMatrix();
		}
		glPopMatrix();

		//smallarm
		glPushMatrix();
		{
			glRotatef(180, 0, 1, 0);
			glTranslatef(0.155, 0, 0.1);
			glTranslatef(0, -0.3, 0);
			glRotatef(rotateAngle, 1, 0, 0);
			glTranslatef(0, 0.3, 0);

			GLUquadricObj* rightArmJoint = NULL;
			rightArmJoint = gluNewQuadric();

			glPushMatrix();
			{
				texture[1] = LoadTexture("steel.bmp");
				glTranslatef(-0.08, -0.3, -0.05);
				gluQuadricDrawStyle(rightArmJoint, GLU_FILL);
				gluQuadricTexture(rightArmJoint, true);
				gluSphere(rightArmJoint, radius / 25.0f, slices, stacks);
				gluDeleteQuadric(rightArmJoint);
				glDeleteTextures(1, &texture[1]);
			}
			glPopMatrix();

			glPushMatrix();
			{
				texture[3] = LoadTexture("greyMetal.bmp");
				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.13, -0.32, 0);
				glTexCoord2f(1, 1); glVertex3f(-0.03, -0.32, 0);
				glTexCoord2f(1, 0); glVertex3f(-0.03, -0.5, 0);
				glTexCoord2f(0, 0); glVertex3f(-0.13, -0.5, 0);
				glEnd();
				//2
				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.13, -0.32, 0);
				glTexCoord2f(1, 1); glVertex3f(-0.13, -0.32, -0.1);
				glTexCoord2f(1, 0); glVertex3f(-0.03, -0.32, -0.1);
				glTexCoord2f(0, 0); glVertex3f(-0.03, -0.32, 0);
				glEnd();
				//3
				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.03, -0.32, 0);
				glTexCoord2f(1, 1); glVertex3f(-0.03, -0.32, -0.1);
				glTexCoord2f(1, 0); glVertex3f(-0.03, -0.5, -0.1);
				glTexCoord2f(0, 0); glVertex3f(-0.03, -0.5, 0);
				glEnd();
				//4
				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.03, -0.5, 0);
				glTexCoord2f(1, 1); glVertex3f(-0.03, -0.5, -0.1);
				glTexCoord2f(1, 0); glVertex3f(-0.13, -0.5, -0.1);
				glTexCoord2f(0, 0); glVertex3f(-0.13, -0.5, 0);
				glEnd();
				//5
				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.13, -0.5, 0);
				glTexCoord2f(1, 1); glVertex3f(-0.13, -0.5, -0.1);
				glTexCoord2f(1, 0); glVertex3f(-0.13, -0.32, -0.1);
				glTexCoord2f(0, 0); glVertex3f(-0.13, -0.32, 0);
				glEnd();

				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.13, -0.32, -0.1);
				glTexCoord2f(1, 1); glVertex3f(-0.03, -0.32, -0.1);
				glTexCoord2f(1, 0); glVertex3f(-0.03, -0.5, -0.1);
				glTexCoord2f(0, 0); glVertex3f(-0.13, -0.5, -0.1);
				glEnd();

				glDeleteTextures(1, &texture[3]);
			}
			glPopMatrix();

			glPushMatrix();
			{
				texture[3] = LoadTexture("greyMetal.bmp");

				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.13, -0.5, -0.1);
				glTexCoord2f(1, 1); glVertex3f(-0.03, -0.5, -0.1);
				glTexCoord2f(1, 0); glVertex3f(-0.03, -0.55, -0.05);
				glTexCoord2f(0, 0); glVertex3f(-0.13, -0.55, -0.05);
				glEnd();
				//2
				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.13, -0.5, -0.1);
				glTexCoord2f(1, 1); glVertex3f(-0.13, -0.5, 0.05);
				glTexCoord2f(1, 0); glVertex3f(-0.03, -0.5, 0.05);
				glTexCoord2f(0, 0); glVertex3f(-0.03, -0.5, -0.1);
				glEnd();
				//3
				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.03, -0.5, -0.1);
				glTexCoord2f(1, 1); glVertex3f(-0.03, -0.5, 0.05);
				glTexCoord2f(1, 0); glVertex3f(-0.03, -0.55, 0.05);
				glTexCoord2f(0, 0); glVertex3f(-0.03, -0.55, -0.05);
				glEnd();
				//4
				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.03, -0.55, -0.05);
				glTexCoord2f(1, 1); glVertex3f(-0.03, -0.55, 0.05);
				glTexCoord2f(1, 0); glVertex3f(-0.13, -0.55, 0.05);
				glTexCoord2f(0, 0); glVertex3f(-0.13, -0.55, -0.05);
				glEnd();
				//5
				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.13, -0.55, -0.05);
				glTexCoord2f(1, 1); glVertex3f(-0.13, -0.55, 0.05);
				glTexCoord2f(1, 0); glVertex3f(-0.13, -0.5, 0.05);
				glTexCoord2f(0, 0); glVertex3f(-0.13, -0.5, -0.1);
				glEnd();

				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.13, -0.5, 0.05);
				glTexCoord2f(1, 1); glVertex3f(-0.03, -0.5, 0.05);
				glTexCoord2f(1, 0); glVertex3f(-0.03, -0.55, 0.05);
				glTexCoord2f(0, 0); glVertex3f(-0.13, -0.55, 0.05);
				glEnd();

				glDeleteTextures(1, &texture[3]);
			}
			glPopMatrix();

			glPushMatrix();
			{
				texture[3] = LoadTexture("greyMetal.bmp");

				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.13, -0.4, 0);
				glTexCoord2f(1, 1); glVertex3f(-0.03, -0.4, 0);
				glTexCoord2f(1, 0); glVertex3f(-0.03, -0.55, 0);
				glTexCoord2f(0, 0); glVertex3f(-0.13, -0.55, 0);
				glEnd();
				//2
				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.13, -0.4, 0);
				glTexCoord2f(1, 1); glVertex3f(-0.13, -0.3, 0.1);
				glTexCoord2f(1, 0); glVertex3f(-0.03, -0.3, 0.1);
				glTexCoord2f(0, 0); glVertex3f(-0.03, -0.4, 0);
				glEnd();
				//3
				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.03, -0.4, 0);
				glTexCoord2f(1, 1); glVertex3f(-0.03, -0.3, 0.1);
				glTexCoord2f(1, 0); glVertex3f(-0.03, -0.55, 0.1);
				glTexCoord2f(0, 0); glVertex3f(-0.03, -0.55, 0);
				glEnd();
				//4
				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.03, -0.55, 0);
				glTexCoord2f(1, 1); glVertex3f(-0.03, -0.55, 0.1);
				glTexCoord2f(1, 0); glVertex3f(-0.13, -0.55, 0.1);
				glTexCoord2f(0, 0); glVertex3f(-0.13, -0.55, 0);
				glEnd();
				//5
				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.13, -0.55, 0);
				glTexCoord2f(1, 1); glVertex3f(-0.13, -0.55, 0.1);
				glTexCoord2f(1, 0); glVertex3f(-0.13, -0.3, 0.1);
				glTexCoord2f(0, 0); glVertex3f(-0.13, -0.4, 0);
				glEnd();

				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.13, -0.3, 0.1);
				glTexCoord2f(1, 1); glVertex3f(-0.03, -0.3, 0.1);
				glTexCoord2f(1, 0); glVertex3f(-0.03, -0.55, 0.1);
				glTexCoord2f(0, 0); glVertex3f(-0.13, -0.55, 0.1);
				glEnd();

				glDeleteTextures(1, &texture[3]);
			}
			glPopMatrix();

			glPushMatrix();
			{
				texture[3] = LoadTexture("greyMetal.bmp");
				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.13, -0.55, -0.05);
				glTexCoord2f(1, 1); glVertex3f(-0.03, -0.55, -0.05);
				glTexCoord2f(1, 0); glVertex3f(-0.03, -0.65, -0.05);
				glTexCoord2f(0, 0); glVertex3f(-0.13, -0.65, -0.05);
				glEnd();
				//2
				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.13, -0.55, -0.05);
				glTexCoord2f(1, 1); glVertex3f(-0.13, -0.55, 0.05);
				glTexCoord2f(1, 0); glVertex3f(-0.03, -0.55, 0.05);
				glTexCoord2f(0, 0); glVertex3f(-0.03, -0.55, -0.05);
				glEnd();
				//3
				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.03, -0.55, -0.05);
				glTexCoord2f(1, 1); (-0.03, -0.55, 0.1);
				glTexCoord2f(1, 0); glVertex3f(-0.03, -0.65, 0.05);
				glTexCoord2f(0, 0); glVertex3f(-0.03, -0.65, -0.05);
				glEnd();
				//4
				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.03, -0.65, -0.05);
				glTexCoord2f(1, 1); glVertex3f(-0.03, -0.65, 0.05);
				glTexCoord2f(1, 0); glVertex3f(-0.13, -0.65, 0.05);
				glTexCoord2f(0, 0); glVertex3f(-0.13, -0.65, -0.05);
				glEnd();
				//5
				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.13, -0.65, -0.05);
				glTexCoord2f(1, 1); glVertex3f(-0.13, -0.65, 0.05);
				glTexCoord2f(1, 0); glVertex3f(-0.13, -0.55, 0.1);
				glTexCoord2f(0, 0); glVertex3f(-0.13, -0.55, -0.05);
				glEnd();

				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.13, -0.55, 0.1);
				glTexCoord2f(1, 1); glVertex3f(-0.03, -0.55, 0.1);
				glTexCoord2f(1, 0); glVertex3f(-0.03, -0.65, 0.05);
				glTexCoord2f(0, 0); glVertex3f(-0.13, -0.65, 0.05);
				glEnd();

				glDeleteTextures(1, &texture[3]);
			}
			glPopMatrix();

			glPushMatrix();
			{
				texture[3] = LoadTexture("greyMetal.bmp");
				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.13, -0.65, -0.05);
				glTexCoord2f(1, 1); glVertex3f(-0.03, -0.65, -0.05);
				glTexCoord2f(1, 0); glVertex3f(-0.03, -0.8, -0.05);
				glTexCoord2f(0, 0); glVertex3f(-0.13, -0.8, -0.05);
				glEnd();
				//2
				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.13, -0.65, -0.05);
				glTexCoord2f(1, 1); glVertex3f(-0.13, -0.65, 0.05);
				glTexCoord2f(1, 0); glVertex3f(-0.03, -0.65, 0.05);
				glTexCoord2f(0, 0); glVertex3f(-0.03, -0.65, -0.05);
				glEnd();
				//3
				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.03, -0.65, -0.05);
				glTexCoord2f(1, 1); glVertex3f(-0.03, -0.65, 0.05);
				glTexCoord2f(1, 0); glVertex3f(-0.03, -0.8, 0.05);
				glTexCoord2f(0, 0); glVertex3f(-0.03, -0.8, -0.05);
				glEnd();
				//4
				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.03, -0.8, -0.05);
				glTexCoord2f(1, 1); glVertex3f(-0.03, -0.8, 0.05);
				glTexCoord2f(1, 0); glVertex3f(-0.13, -0.8, 0.05);
				glTexCoord2f(0, 0); glVertex3f(-0.13, -0.8, -0.05);
				glEnd();
				//5
				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.13, -0.8, -0.05);
				glTexCoord2f(1, 1); glVertex3f(-0.13, -0.8, 0.05);
				glTexCoord2f(1, 0); glVertex3f(-0.13, -0.65, 0.05);
				glTexCoord2f(0, 0); glVertex3f(-0.13, -0.65, -0.05);
				glEnd();

				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.13, -0.65, 0.05);
				glTexCoord2f(1, 1); glVertex3f(-0.03, -0.65, 0.05);
				glTexCoord2f(1, 0); glVertex3f(-0.03, -0.8, 0.05);
				glTexCoord2f(0, 0); glVertex3f(-0.13, -0.8, 0.05);
				glEnd();
				glDeleteTextures(1, &texture[3]);

				glPushMatrix();
				{
					texture[1] = LoadTexture("steel.bmp");
					//hand
					GLUquadricObj* rightHandJoint = NULL;
					rightHandJoint = gluNewQuadric();

					glTranslatef(-0.08, -0.8, 0);
					gluQuadricDrawStyle(rightHandJoint, GLU_FILL);
					gluQuadricTexture(rightHandJoint, true);
					gluSphere(rightHandJoint, radius / 25.0f, slices, stacks);
					gluDeleteQuadric(rightHandJoint);

					glDeleteTextures(1, &texture[1]);
				}
				glPopMatrix();
			}
			glPopMatrix();

			glPushMatrix();
			{
				glTranslatef(-0.11, -0.92, 0.055);
				glRotatef(-90, 0, 1, 0);
				hand();
			}
			glPopMatrix();
			glPushMatrix();
			{
				glTranslatef(0.02, -1, 0);
				glRotatef(90, 0, 1, 0);
				weapon();
			}
			glPopMatrix();
		}
		glPopMatrix();

	}
	glPopMatrix();
}

void hand()
{
	glPushMatrix();
	{//shouzhang
		texture[3] = LoadTexture("greyMetal.bmp");
		glBegin(GL_POLYGON);
		glTexCoord2f(0, 1); glVertex3f(-0.1, 0.1, -0.01);
		glTexCoord2f(1, 1); glVertex3f(0, 0.1, -0.01);
		glTexCoord2f(1, 0); glVertex3f(0, 0, -0.01);
		glTexCoord2f(0, 0); glVertex3f(-0.1, 0, -0.01);
		glEnd();

		//2
		glBegin(GL_POLYGON);
		glTexCoord2f(0, 1); glVertex3f(-0.1, 0.1, -0.01);
		glTexCoord2f(1, 1); glVertex3f(-0.1, 0.1, -0.05);
		glTexCoord2f(1, 0); glVertex3f(0, 0.1, -0.05);
		glTexCoord2f(0, 0); glVertex3f(0, 0.1, -0.01);
		glEnd();
		//3
		glBegin(GL_POLYGON);
		glTexCoord2f(0, 1); glVertex3f(0, 0.1, -0.01);
		glTexCoord2f(1, 1); glVertex3f(0, 0.1, -0.05);
		glTexCoord2f(1, 0); glVertex3f(0, 0, -0.05);
		glTexCoord2f(0, 0); glVertex3f(0, 0, -0.01);
		glEnd();
		//4
		glBegin(GL_POLYGON);
		glTexCoord2f(0, 1); glVertex3f(0, 0, -0.01);
		glTexCoord2f(1, 1); glVertex3f(0, 0, -0.05);
		glTexCoord2f(1, 0); glVertex3f(-0.1, 0, -0.05);
		glTexCoord2f(0, 0); glVertex3f(-0.1, 0, -0.01);
		glEnd();
		//5
		glBegin(GL_POLYGON);
		glTexCoord2f(0, 1); glVertex3f(-0.1, 0, -0.01);
		glTexCoord2f(1, 1); glVertex3f(-0.1, 0, -0.05);
		glTexCoord2f(1, 0); glVertex3f(-0.1, 0.1, -0.05);
		glTexCoord2f(0, 0); glVertex3f(-0.1, 0.1, -0.01);
		glEnd();

		glBegin(GL_POLYGON);
		glTexCoord2f(0, 1); glVertex3f(-0.1, 0.1, -0.05);
		glTexCoord2f(1, 1); glVertex3f(0, 0.1, -0.05);
		glTexCoord2f(1, 0); glVertex3f(0, 0, -0.05);
		glTexCoord2f(0, 0); glVertex3f(-0.1, 0, -0.05);
		glEnd();

		glDeleteTextures(1, &texture[3]);

		glPushMatrix();
		{
			texture[1] = LoadTexture("steel.bmp");

			glPushMatrix();
			{//diyijiederotation
				glTranslatef(0, 0.02, -0.03);
				glRotatef(rotateLowerFinger, 1, 0, 0);
				glTranslatef(0, -0.02, 0.03);
				//glPushMatrix();
				//{//muzhi (diyijie)
				//	glBegin(GL_POLYGON);
				//	glVertex3f(0, 0.06, -0.02);
				//	glVertex3f(0.03, 0.06, -0.02);
				//	glVertex3f(0.03, 0.03, -0.02);
				//	glVertex3f(0, 0.03, -0.02);
				//	glEnd();
				//	//2
				//	glBegin(GL_POLYGON);
				//	glVertex3f(0, 0.06, -0.02);
				//	glVertex3f(0, 0.06, -0.04);
				//	glVertex3f(0.03, 0.06, -0.04);
				//	glVertex3f(0.03, 0.06, -0.02);
				//	glEnd();
				//	//3
				//	glBegin(GL_POLYGON);
				//	glVertex3f(0.03, 0.06, -0.02);
				//	glVertex3f(0.03, 0.06, -0.04);
				//	glVertex3f(0.03, 0.03, -0.04);
				//	glVertex3f(0.03, 0.03, -0.02);
				//	glEnd();
				//	//4
				//	glBegin(GL_POLYGON);
				//	glVertex3f(0.03, 0.03, -0.02);
				//	glVertex3f(0.03, 0.03, -0.04);
				//	glVertex3f(0, 0.03, -0.04);
				//	glVertex3f(0, 0.03, -0.02);
				//	glEnd();
				//	//5
				//	glBegin(GL_POLYGON);
				//	glVertex3f(0, 0.03, -0.02);
				//	glVertex3f(0, 0.03, -0.04);
				//	glVertex3f(0, 0.06, -0.04);
				//	glVertex3f(0, 0.06, -0.02);
				//	glEnd();
				//	glBegin(GL_POLYGON);
				//	glVertex3f(0, 0.06, -0.04);
				//	glVertex3f(0.03, 0.06, -0.04);
				//	glVertex3f(0.03, 0.03, -0.04);
				//	glVertex3f(0, 0.03, -0.04);
				//	glEnd();
				//}
				//glPopMatrix();

				glPushMatrix();
				{//shizhi(diyijie)
					glBegin(GL_POLYGON);
					glTexCoord2f(0, 1); glVertex3f(-0.02, 0, -0.02);
					glTexCoord2f(1, 1); glVertex3f(0, 0, -0.02);
					glTexCoord2f(1, 0); glVertex3f(0, -0.03, -0.02);
					glTexCoord2f(0, 0); glVertex3f(-0.02, -0.03, -0.02);
					glEnd();
					//2
					glBegin(GL_POLYGON);
					glTexCoord2f(0, 1); glVertex3f(-0.02, 0, -0.02);
					glTexCoord2f(1, 1); glVertex3f(-0.02, 0, -0.04);
					glTexCoord2f(1, 0); glVertex3f(0, 0, -0.04);
					glTexCoord2f(0, 0); glVertex3f(0, 0, -0.02);
					glEnd();
					//3
					glBegin(GL_POLYGON);
					glTexCoord2f(0, 1); glVertex3f(0, 0, -0.02);
					glTexCoord2f(1, 1); glVertex3f(0, 0, -0.04);
					glTexCoord2f(1, 0); glVertex3f(0, -0.03, -0.04);
					glTexCoord2f(0, 0); glVertex3f(0, -0.03, -0.02);
					glEnd();
					//4
					glBegin(GL_POLYGON);
					glTexCoord2f(0, 1); glVertex3f(0, -0.03, -0.02);
					glTexCoord2f(1, 1); glVertex3f(0, -0.03, -0.04);
					glTexCoord2f(1, 0); glVertex3f(-0.02, -0.03, -0.04);
					glTexCoord2f(0, 0); glVertex3f(-0.02, -0.03, -0.02);
					glEnd();
					//5
					glBegin(GL_POLYGON);
					glTexCoord2f(0, 1); glVertex3f(-0.02, -0.03, -0.02);
					glTexCoord2f(1, 1); glVertex3f(-0.02, -0.03, -0.04);
					glTexCoord2f(1, 0); glVertex3f(-0.02, 0, -0.04);
					glTexCoord2f(0, 0); glVertex3f(-0.02, 0, -0.02);
					glEnd();

					glBegin(GL_POLYGON);
					glTexCoord2f(0, 1); glVertex3f(-0.02, 0, -0.04);
					glTexCoord2f(1, 1); glVertex3f(0, 0, -0.04);
					glTexCoord2f(1, 0); glVertex3f(0, -0.03, -0.04);
					glTexCoord2f(0, 0); glVertex3f(-0.02, -0.03, -0.04);
					glEnd();
				}
				glPopMatrix();

				glPushMatrix();
				{//zhongzhi(diyijie)
					glBegin(GL_POLYGON);
					glTexCoord2f(0, 1); glVertex3f(-0.045, 0, -0.02);
					glTexCoord2f(1, 1); glVertex3f(-0.025, 0, -0.02);
					glTexCoord2f(1, 0); glVertex3f(-0.025, -0.03, -0.02);
					glTexCoord2f(0, 0); glVertex3f(-0.045, -0.03, -0.02);
					glEnd();
					//2
					glBegin(GL_POLYGON);
					glTexCoord2f(0, 1); glVertex3f(-0.045, 0, -0.02);
					glTexCoord2f(1, 1); glVertex3f(-0.045, 0, -0.04);
					glTexCoord2f(1, 0); glVertex3f(-0.025, 0, -0.04);
					glTexCoord2f(0, 0); glVertex3f(-0.025, 0, -0.02);
					glEnd();
					//3
					glBegin(GL_POLYGON);
					glTexCoord2f(0, 1); glVertex3f(-0.025, 0, -0.02);
					glTexCoord2f(1, 1); glVertex3f(-0.025, 0, -0.04);
					glTexCoord2f(1, 0); glVertex3f(-0.025, -0.03, -0.04);
					glTexCoord2f(0, 0); glVertex3f(-0.025, -0.03, -0.02);
					glEnd();
					//4
					glBegin(GL_POLYGON);
					glTexCoord2f(0, 1); glVertex3f(-0.025, -0.03, -0.02);
					glTexCoord2f(1, 1); glVertex3f(-0.025, -0.03, -0.04);
					glTexCoord2f(1, 0); glVertex3f(-0.045, -0.03, -0.04);
					glTexCoord2f(0, 0); glVertex3f(-0.045, -0.03, -0.02);
					glEnd();
					//5
					glBegin(GL_POLYGON);
					glTexCoord2f(0, 1); glVertex3f(-0.045, -0.03, -0.02);
					glTexCoord2f(1, 1); glVertex3f(-0.045, -0.03, -0.04);
					glTexCoord2f(1, 0); glVertex3f(-0.045, 0, -0.04);
					glTexCoord2f(0, 0); glVertex3f(-0.045, 0, -0.02);
					glEnd();

					glBegin(GL_POLYGON);
					glTexCoord2f(0, 1); glVertex3f(-0.045, 0, -0.04);
					glTexCoord2f(1, 1); glVertex3f(-0.025, 0, -0.04);
					glTexCoord2f(1, 0); glVertex3f(-0.025, -0.03, -0.04);
					glTexCoord2f(0, 0); glVertex3f(-0.045, -0.03, -0.04);
					glEnd();
				}
				glPopMatrix();

				glPushMatrix();
				{//wumingzhi(diyijie)
					glBegin(GL_POLYGON);
					glTexCoord2f(0, 1); glVertex3f(-0.05, 0, -0.02);
					glTexCoord2f(1, 1); glVertex3f(-0.07, 0, -0.02);
					glTexCoord2f(1, 0); glVertex3f(-0.07, -0.03, -0.02);
					glTexCoord2f(0, 0); glVertex3f(-0.05, -0.03, -0.02);
					glEnd();
					//2
					glBegin(GL_POLYGON);
					glTexCoord2f(0, 1); glVertex3f(-0.05, 0, -0.02);
					glTexCoord2f(1, 1); glVertex3f(-0.05, 0, -0.04);
					glTexCoord2f(1, 0); glVertex3f(-0.07, 0, -0.04);
					glTexCoord2f(0, 0); glVertex3f(-0.07, 0, -0.02);
					glEnd();
					//3
					glBegin(GL_POLYGON);
					glTexCoord2f(0, 1); glVertex3f(-0.07, 0, -0.02);
					glTexCoord2f(1, 1); glVertex3f(-0.07, 0, -0.04);
					glTexCoord2f(1, 0); glVertex3f(-0.07, -0.03, -0.04);
					glTexCoord2f(0, 0); glVertex3f(-0.07, -0.03, -0.02);
					glEnd();
					//4
					glBegin(GL_POLYGON);
					glTexCoord2f(0, 1); glVertex3f(-0.07, -0.03, -0.02);
					glTexCoord2f(1, 1); glVertex3f(-0.07, -0.03, -0.04);
					glTexCoord2f(1, 0); glVertex3f(-0.05, -0.03, -0.04);
					glTexCoord2f(0, 0); glVertex3f(-0.05, -0.03, -0.02);
					glEnd();
					//5
					glBegin(GL_POLYGON);
					glTexCoord2f(0, 1); glVertex3f(-0.05, -0.03, -0.02);
					glTexCoord2f(1, 1); glVertex3f(-0.05, -0.03, -0.04);
					glTexCoord2f(1, 0); glVertex3f(-0.05, 0, -0.04);
					glTexCoord2f(0, 0); glVertex3f(-0.05, 0, -0.02);
					glEnd();

					glBegin(GL_POLYGON);
					glTexCoord2f(0, 1); glVertex3f(-0.05, 0, -0.04);
					glTexCoord2f(1, 1); glVertex3f(-0.07, 0, -0.04);
					glTexCoord2f(1, 0); glVertex3f(-0.07, -0.03, -0.04);
					glTexCoord2f(0, 0); glVertex3f(-0.05, -0.03, -0.04);
					glEnd();
				}
				glPopMatrix();

				glPushMatrix();
				{//weizhi(diyijie)
					glBegin(GL_POLYGON);
					glTexCoord2f(0, 1); glVertex3f(-0.075, 0, -0.02);
					glTexCoord2f(1, 1); glVertex3f(-0.095, 0, -0.02);
					glTexCoord2f(1, 0); glVertex3f(-0.095, -0.03, -0.02);
					glTexCoord2f(0, 0); glVertex3f(-0.075, -0.03, -0.02);
					glEnd();
					//2
					glBegin(GL_POLYGON);
					glTexCoord2f(0, 1); glVertex3f(-0.075, 0, -0.02);
					glTexCoord2f(1, 1); glVertex3f(-0.075, 0, -0.04);
					glTexCoord2f(1, 0); glVertex3f(-0.095, 0, -0.04);
					glTexCoord2f(0, 0); glVertex3f(-0.095, 0, -0.02);
					glEnd();
					//3
					glBegin(GL_POLYGON);
					glTexCoord2f(0, 1); glVertex3f(-0.095, 0, -0.02);
					glTexCoord2f(1, 1); glVertex3f(-0.095, 0, -0.04);
					glTexCoord2f(1, 0); glVertex3f(-0.095, -0.03, -0.04);
					glTexCoord2f(0, 0); glVertex3f(-0.095, -0.03, -0.02);
					glEnd();
					//4
					glBegin(GL_POLYGON);
					glTexCoord2f(0, 1); glVertex3f(-0.095, -0.03, -0.02);
					glTexCoord2f(1, 1); glVertex3f(-0.095, -0.03, -0.04);
					glTexCoord2f(1, 0); glVertex3f(-0.075, -0.03, -0.04);
					glTexCoord2f(0, 0); glVertex3f(-0.075, -0.03, -0.02);
					glEnd();
					//5
					glBegin(GL_POLYGON);
					glTexCoord2f(0, 1); glVertex3f(-0.075, -0.03, -0.02);
					glTexCoord2f(1, 1); glVertex3f(-0.075, -0.03, -0.04);
					glTexCoord2f(1, 0); glVertex3f(-0.075, 0, -0.04);
					glTexCoord2f(0, 0); glVertex3f(-0.075, 0, -0.02);
					glEnd();

					glBegin(GL_POLYGON);
					glTexCoord2f(0, 1); glVertex3f(-0.075, 0, -0.04);
					glTexCoord2f(1, 1); glVertex3f(-0.095, 0, -0.04);
					glTexCoord2f(1, 0); glVertex3f(-0.095, -0.03, -0.04);
					glTexCoord2f(0, 0); glVertex3f(-0.075, -0.03, -0.04);
					glEnd();
				}
				glPopMatrix();

				glPushMatrix();
				{//dierjiederotation
					glTranslatef(0, -0.035, -0.035);
					glRotatef(rotateUpperFinger, 1, 0, 0);
					glTranslatef(0, 0.035, 0.035);
					//glPushMatrix();
					//{//muzhi (dierjie)
					//	glBegin(GL_POLYGON);
					//	glVertex3f(0.031, 0.06, -0.02);
					//	glVertex3f(0.06, 0.06, -0.02);
					//	glVertex3f(0.06, 0.03, -0.02);
					//	glVertex3f(0.031, 0.03, -0.02);
					//	glEnd();
					//	//2
					//	glBegin(GL_POLYGON);
					//	glVertex3f(0.031, 0.06, -0.02);
					//	glVertex3f(0.031, 0.06, -0.04);
					//	glVertex3f(0.06, 0.06, -0.04);
					//	glVertex3f(0.06, 0.06, -0.02);
					//	glEnd();
					//	//3
					//	glBegin(GL_POLYGON);
					//	glVertex3f(0.06, 0.06, -0.02);
					//	glVertex3f(0.06, 0.06, -0.04);
					//	glVertex3f(0.06, 0.03, -0.04);
					//	glVertex3f(0.06, 0.03, -0.02);
					//	glEnd();
					//	//4
					//	glBegin(GL_POLYGON);
					//	glVertex3f(0.06, 0.03, -0.02);
					//	glVertex3f(0.06, 0.03, -0.04);
					//	glVertex3f(0.031, 0.03, -0.04);
					//	glVertex3f(0.031, 0.03, -0.02);
					//	glEnd();
					//	//5
					//	glBegin(GL_POLYGON);
					//	glVertex3f(0.031, 0.03, -0.02);
					//	glVertex3f(0.031, 0.03, -0.04);
					//	glVertex3f(0.031, 0.06, -0.04);
					//	glVertex3f(0.031, 0.06, -0.02);
					//	glEnd();
					//	glBegin(GL_POLYGON);
					//	glVertex3f(0.031, 0.06, -0.04);
					//	glVertex3f(0.06, 0.06, -0.04);
					//	glVertex3f(0.06, 0.03, -0.04);
					//	glVertex3f(0.031, 0.03, -0.04);
					//	glEnd();
					//}
					//glPopMatrix();

					glPushMatrix();
					{//shizhi(dierjie)
						glBegin(GL_POLYGON);
						glTexCoord2f(0, 1); glVertex3f(-0.02, -0.031, -0.02);
						glTexCoord2f(1, 1); glVertex3f(0, -0.031, -0.02);
						glTexCoord2f(1, 0); glVertex3f(0, -0.07, -0.02);
						glTexCoord2f(0, 0); glVertex3f(-0.02, -0.07, -0.02);
						glEnd();
						//2
						glBegin(GL_POLYGON);
						glTexCoord2f(0, 1); glVertex3f(-0.02, -0.031, -0.02);
						glTexCoord2f(1, 1); glVertex3f(-0.02, -0.031, -0.04);
						glTexCoord2f(1, 0); glVertex3f(0, -0.031, -0.04);
						glTexCoord2f(0, 0); glVertex3f(0, -0.031, -0.02);
						glEnd();
						//3
						glBegin(GL_POLYGON);
						glTexCoord2f(0, 1); glVertex3f(0, -0.031, -0.02);
						glTexCoord2f(1, 1); glVertex3f(0, -0.031, -0.04);
						glTexCoord2f(1, 0); glVertex3f(0, -0.07, -0.04);
						glTexCoord2f(0, 0); glVertex3f(0, -0.07, -0.02);
						glEnd();
						//4
						glBegin(GL_POLYGON);
						glTexCoord2f(0, 1); glVertex3f(0, -0.07, -0.02);
						glTexCoord2f(1, 1); glVertex3f(0, -0.07, -0.04);
						glTexCoord2f(1, 0); glVertex3f(-0.02, -0.07, -0.04);
						glTexCoord2f(0, 0); glVertex3f(-0.02, -0.07, -0.02);
						glEnd();
						//5
						glBegin(GL_POLYGON);
						glTexCoord2f(0, 1); glVertex3f(-0.02, -0.07, -0.02);
						glTexCoord2f(1, 1); glVertex3f(-0.02, -0.07, -0.04);
						glTexCoord2f(1, 0); glVertex3f(-0.02, -0.031, -0.04);
						glTexCoord2f(0, 0); glVertex3f(-0.02, -0.031, -0.02);
						glEnd();

						glBegin(GL_POLYGON);
						glTexCoord2f(0, 1); glVertex3f(-0.02, -0.031, -0.04);
						glTexCoord2f(1, 1); glVertex3f(0, -0.031, -0.04);
						glTexCoord2f(1, 0); glVertex3f(0, -0.07, -0.04);
						glTexCoord2f(0, 0); glVertex3f(-0.02, -0.07, -0.04);
						glEnd();
					}
					glPopMatrix();

					glPushMatrix();
					{//zhongzhi(dierjie)
						glBegin(GL_POLYGON);
						glTexCoord2f(0, 1); glVertex3f(-0.045, -0.031, -0.02);
						glTexCoord2f(1, 1); glVertex3f(-0.024, -0.031, -0.02);
						glTexCoord2f(1, 0); glVertex3f(-0.025, -0.081, -0.02);
						glTexCoord2f(0, 0); glVertex3f(-0.045, -0.081, -0.02);
						glEnd();
						//2
						glBegin(GL_POLYGON);
						glTexCoord2f(0, 1); glVertex3f(-0.045, -0.031, -0.02);
						glTexCoord2f(1, 1); glVertex3f(-0.045, -0.031, -0.04);
						glTexCoord2f(1, 0); glVertex3f(-0.024, -0.031, -0.04);
						glTexCoord2f(0, 0); glVertex3f(-0.024, -0.031, -0.02);
						glEnd();
						//3
						glBegin(GL_POLYGON);
						glTexCoord2f(0, 1); glVertex3f(-0.024, -0.031, -0.02);
						glTexCoord2f(1, 1); glVertex3f(-0.024, -0.031, -0.04);
						glTexCoord2f(1, 0); glVertex3f(-0.024, -0.081, -0.04);
						glTexCoord2f(0, 0); glVertex3f(-0.024, -0.081, -0.02);
						glEnd();
						//4
						glBegin(GL_POLYGON);
						glTexCoord2f(0, 1); glVertex3f(-0.025, -0.081, -0.02);
						glTexCoord2f(1, 1); glVertex3f(-0.025, -0.081, -0.04);
						glTexCoord2f(1, 0); glVertex3f(-0.045, -0.081, -0.04);
						glTexCoord2f(0, 0); glVertex3f(-0.045, -0.081, -0.02);
						glEnd();
						//5
						glBegin(GL_POLYGON);
						glTexCoord2f(0, 1); glVertex3f(-0.045, -0.081, -0.02);
						glTexCoord2f(1, 1); glVertex3f(-0.045, -0.081, -0.04);
						glTexCoord2f(1, 0); glVertex3f(-0.045, -0.031, -0.04);
						glTexCoord2f(0, 0); glVertex3f(-0.045, -0.031, -0.02);
						glEnd();

						glBegin(GL_POLYGON);
						glTexCoord2f(0, 1); glVertex3f(-0.045, -0.081, -0.04);
						glTexCoord2f(1, 1); glVertex3f(-0.024, -0.081, -0.04);
						glTexCoord2f(1, 0); glVertex3f(-0.025, -0.031, -0.04);
						glTexCoord2f(0, 0); glVertex3f(-0.045, -0.031, -0.04);
						glEnd();
					}
					glPopMatrix();

					glPushMatrix();
					{//wumingzhi(dierjie)
						glBegin(GL_POLYGON);
						glTexCoord2f(0, 1); glVertex3f(-0.07, -0.031, -0.02);
						glTexCoord2f(1, 1); glVertex3f(-0.05, -0.031, -0.02);
						glTexCoord2f(1, 0); glVertex3f(-0.05, -0.081, -0.02);
						glTexCoord2f(0, 0); glVertex3f(-0.07, -0.081, -0.02);
						glEnd();
						//2
						glBegin(GL_POLYGON);
						glTexCoord2f(0, 1); glTexCoord2f(0, 1); glVertex3f(-0.07, -0.031, -0.02);
						glTexCoord2f(1, 1); glVertex3f(-0.07, -0.031, -0.04);
						glTexCoord2f(1, 0); glVertex3f(-0.05, -0.031, -0.04);
						glTexCoord2f(0, 0); glVertex3f(-0.05, -0.031, -0.02);
						glEnd();
						//3
						glBegin(GL_POLYGON);
						glTexCoord2f(0, 1); glVertex3f(-0.05, -0.031, -0.02);
						glTexCoord2f(1, 1); glVertex3f(-0.05, -0.031, -0.04);
						glTexCoord2f(1, 0); glVertex3f(-0.05, -0.081, -0.04);
						glTexCoord2f(0, 0); glVertex3f(-0.05, -0.081, -0.02);
						glEnd();
						//4
						glBegin(GL_POLYGON);
						glTexCoord2f(0, 1); glVertex3f(-0.05, -0.081, -0.02);
						glTexCoord2f(1, 1); glVertex3f(-0.05, -0.081, -0.04);
						glTexCoord2f(1, 0); glVertex3f(-0.07, -0.081, -0.04);
						glTexCoord2f(0, 0); glVertex3f(-0.07, -0.081, -0.02);
						glEnd();
						//5
						glBegin(GL_POLYGON);
						glTexCoord2f(0, 1); glVertex3f(-0.07, -0.081, -0.02);
						glTexCoord2f(1, 1); glVertex3f(-0.07, -0.081, -0.04);
						glTexCoord2f(1, 0); glVertex3f(-0.07, -0.031, -0.04);
						glTexCoord2f(0, 0); glVertex3f(-0.07, -0.031, -0.02);
						glEnd();

						glBegin(GL_POLYGON);
						glTexCoord2f(0, 1); glVertex3f(-0.07, -0.081, -0.04);
						glTexCoord2f(1, 1); glVertex3f(-0.05, -0.081, -0.04);
						glTexCoord2f(1, 0); glVertex3f(-0.05, -0.031, -0.04);
						glTexCoord2f(0, 0); glVertex3f(-0.07, -0.031, -0.04);
						glEnd();
					}
					glPopMatrix();

					glPushMatrix();
					{//wumingzhi(dierjie)
						glBegin(GL_POLYGON);
						glTexCoord2f(0, 1); glVertex3f(-0.075, -0.031, -0.02);
						glTexCoord2f(1, 1); glVertex3f(-0.095, -0.031, -0.02);
						glTexCoord2f(1, 0); glVertex3f(-0.095, -0.075, -0.02);
						glTexCoord2f(0, 0); glVertex3f(-0.075, -0.075, -0.02);
						glEnd();
						//2
						glBegin(GL_POLYGON);
						glTexCoord2f(0, 1); glVertex3f(-0.075, -0.031, -0.02);
						glTexCoord2f(1, 1); glVertex3f(-0.075, -0.031, -0.04);
						glTexCoord2f(1, 0); glVertex3f(-0.095, -0.031, -0.04);
						glTexCoord2f(0, 0); glVertex3f(-0.095, -0.031, -0.02);
						glEnd();
						//3
						glBegin(GL_POLYGON);
						glTexCoord2f(0, 1); glVertex3f(-0.095, -0.031, -0.02);
						glTexCoord2f(1, 1); glVertex3f(-0.095, -0.031, -0.04);
						glTexCoord2f(1, 0); glVertex3f(-0.095, -0.075, -0.04);
						glTexCoord2f(0, 0); glVertex3f(-0.095, -0.075, -0.02);
						glEnd();
						//4
						glBegin(GL_POLYGON);
						glTexCoord2f(0, 1); glVertex3f(-0.095, -0.075, -0.02);
						glTexCoord2f(1, 1); glVertex3f(-0.095, -0.075, -0.04);
						glTexCoord2f(1, 0); glVertex3f(-0.075, -0.075, -0.04);
						glTexCoord2f(0, 0); glVertex3f(-0.075, -0.075, -0.02);
						glEnd();
						//5
						glBegin(GL_POLYGON);
						glTexCoord2f(0, 1); glVertex3f(-0.075, -0.075, -0.02);
						glTexCoord2f(1, 1); glVertex3f(-0.075, -0.075, -0.04);
						glTexCoord2f(1, 0); glVertex3f(-0.075, -0.031, -0.04);
						glTexCoord2f(0, 0); glVertex3f(-0.075, -0.031, -0.02);
						glEnd();

						glBegin(GL_POLYGON);
						glTexCoord2f(0, 1); glVertex3f(-0.075, -0.075, -0.04);
						glTexCoord2f(1, 1); glVertex3f(-0.095, -0.075, -0.04);
						glTexCoord2f(1, 0); glVertex3f(-0.095, -0.031, -0.04);
						glTexCoord2f(0, 0); glVertex3f(-0.075, -0.031, -0.04);
						glEnd();
					}
					glPopMatrix();
				}
				glPopMatrix();
			}
			glPopMatrix();

			glDeleteTextures(1, &texture[1]);
		}
		glPopMatrix();

	}
	glPopMatrix();
}

void weapon()
{
	glPushMatrix();
	{
		glPushMatrix();
		{//shoubing
			GLUquadricObj* weaponHandle = NULL;
			weaponHandle = gluNewQuadric();

			texture[1] = LoadTexture("steel.bmp");
			glTranslatef(0, 0.5, -0.1);
			//glRotatef(90, 1, 0, 0);
			gluQuadricDrawStyle(weaponHandle, GLU_FILL);
			gluQuadricTexture(weaponHandle, true);
			gluCylinder(weaponHandle, 0.03, 0.03, 0.1, 20, 20);
			gluDeleteQuadric(weaponHandle);
			glDeleteTextures(1, &texture[1]);
		}
		glPopMatrix();

		glPushMatrix();
		{//shoubingxiamian
			texture[3] = LoadTexture("greyMetal.bmp");
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(0.05, 0.6, 0.06);
			glTexCoord2f(1, 1); glVertex3f(-0.05, 0.6, 0.06);
			glTexCoord2f(1, 0); glVertex3f(-0.05, 0.4, 0.06);
			glTexCoord2f(0, 0); glVertex3f(0.05, 0.4, 0.06);
			glEnd();
			//2
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(0.05, 0.6, 0.06);
			glTexCoord2f(1, 1); glVertex3f(0.05, 0.6, -0.03);
			glTexCoord2f(1, 0); glVertex3f(-0.05, 0.6, -0.03);
			glTexCoord2f(0, 0); glVertex3f(-0.05, 0.6, 0.06);
			glEnd();
			//3
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.05, 0.6, 0.06);
			glTexCoord2f(1, 1); glVertex3f(-0.05, 0.6, -0.03);
			glTexCoord2f(1, 0); glVertex3f(-0.05, 0.4, -0.03);
			glTexCoord2f(0, 0); glVertex3f(-0.05, 0.4, 0.06);
			glEnd();
			//4
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.05, 0.6, 0.06);
			glTexCoord2f(1, 1); glVertex3f(-0.05, 0.4, -0.03);
			glTexCoord2f(1, 0); glVertex3f(0.05, 0.4, -0.03);
			glTexCoord2f(0, 0); glVertex3f(0.05, 0.4, 0.06);
			glEnd();
			//5
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(0.05, 0.4, 0.06);
			glTexCoord2f(1, 1); glVertex3f(0.05, 0.4, -0.03);
			glTexCoord2f(1, 0); glVertex3f(0.05, 0.6, -0.03);
			glTexCoord2f(0, 0); glVertex3f(0.05, 0.6, 0.06);
			glEnd();

			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(0.05, 0.6, -0.03);
			glTexCoord2f(1, 1); glVertex3f(-0.05, 0.6, -0.03);
			glTexCoord2f(1, 0); glVertex3f(-0.05, 0.4, -0.03);
			glTexCoord2f(0, 0); glVertex3f(0.05, 0.4, -0.03);
			glEnd();
			glDeleteTextures(1, &texture[3]);
		}
		glPopMatrix();

		glPushMatrix();
		{//shoubingxiamian
			texture[3] = LoadTexture("greyMetal.bmp");
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.05, 0.4, -0.03);
			glTexCoord2f(1, 1); glVertex3f(0.05, 0.4, -0.03);
			glTexCoord2f(1, 0); glVertex3f(0.05, 0.35, 0);
			glTexCoord2f(0, 0); glVertex3f(-0.05, 0.35, 0);
			glEnd();
			//2
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.05, 0.4, -0.03);
			glTexCoord2f(1, 1); glVertex3f(-0.05, 0.4, 0.06);
			glTexCoord2f(1, 0); glVertex3f(0.05, 0.4, 0.06);
			glTexCoord2f(0, 0); glVertex3f(0.05, 0.4, -0.03);
			glEnd();
			//3
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(0.05, 0.4, -0.03);
			glTexCoord2f(1, 1); glVertex3f(0.05, 0.4, 0.06);
			glTexCoord2f(1, 0); glVertex3f(0.05, 0.35, 0.06);
			glTexCoord2f(0, 0); glVertex3f(0.05, 0.35, 0);
			glEnd();
			//4
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(0.05, 0.35, 0);
			glTexCoord2f(1, 1); glVertex3f(0.05, 0.35, 0.06);
			glTexCoord2f(1, 0); glVertex3f(-0.05, 0.35, 0.06);
			glTexCoord2f(0, 0); glVertex3f(-0.05, 0.35, 0);
			glEnd();
			//5
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.05, 0.35, 0);
			glTexCoord2f(1, 1); glVertex3f(-0.05, 0.35, 0.06);
			glTexCoord2f(1, 0); glVertex3f(-0.05, 0.4, 0.06);
			glTexCoord2f(0, 0); glVertex3f(-0.05, 0.4, -0.03);
			glEnd();

			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.05, 0.4, 0.06);
			glTexCoord2f(1, 1); glVertex3f(0.05, 0.4, 0.06);
			glTexCoord2f(1, 0); glVertex3f(0.05, 0.35, 0.06);
			glTexCoord2f(0, 0); glVertex3f(-0.05, 0.35, 0.06);
			glEnd();
			glDeleteTextures(1, &texture[3]);
		}
		glPopMatrix();

		glPushMatrix();
		{//qiandaoshen
			texture[1] = LoadTexture("steel.bmp");
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.02, 0.35, 0);
			glTexCoord2f(1, 1); glVertex3f(0.02, 0.35, 0);
			glTexCoord2f(1, 0); glVertex3f(0.02, 0, 0);
			glTexCoord2f(0, 0); glVertex3f(-0.02, 0, 0);
			glEnd();
			//2
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.02, 0.35, 0);
			glTexCoord2f(1, 1); glVertex3f(-0.02, 0.35, 0.03);
			glTexCoord2f(1, 0); glVertex3f(0.02, 0.35, 0.03);
			glTexCoord2f(0, 0); glVertex3f(0.02, 0.35, 0);
			glEnd();
			//3
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(0.02, 0.35, 0);
			glTexCoord2f(1, 1); glVertex3f(0.02, 0.35, 0.03);
			glTexCoord2f(1, 0); glVertex3f(0.02, 0, 0.03);
			glTexCoord2f(0, 0); glVertex3f(0.02, 0, 0);
			glEnd();
			//4
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(0.02, 0, 0);
			glTexCoord2f(1, 1); glVertex3f(0.02, 0, 0.03);
			glTexCoord2f(1, 0); glVertex3f(-0.02, 0, 0.03);
			glTexCoord2f(0, 0); glVertex3f(-0.02, 0, 0);
			glEnd();
			//5
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.02, 0, 0);
			glTexCoord2f(1, 1); glVertex3f(-0.02, 0, 0.03);
			glTexCoord2f(1, 0); glVertex3f(-0.02, 0.35, 0.03);
			glTexCoord2f(0, 0); glVertex3f(-0.02, 0.35, 0);
			glEnd();

			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.02, 0.35, 0.03);
			glTexCoord2f(1, 1); glVertex3f(0.02, 0.35, 0.03);
			glTexCoord2f(1, 0); glVertex3f(0.02, 0, 0.03);
			glTexCoord2f(0, 0); glVertex3f(-0.02, 0, 0.03);
			glEnd();
			glDeleteTextures(1, &texture[1]);
		}
		glPopMatrix();

		glPushMatrix();
		{//daojian(kandonhxidedifang)
			texture[1] = LoadTexture("steel.bmp");
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.02, 0.35, 0.03);
			glTexCoord2f(1, 1); glVertex3f(0.02, 0.35, 0.03);
			glTexCoord2f(1, 0); glVertex3f(0.02, 0, 0.03);
			glTexCoord2f(0, 0); glVertex3f(-0.02, 0, 0.03);
			glEnd();
			//2
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.02, 0.35, 0.03);
			glTexCoord2f(1, 1); glVertex3f(0, 0.35, 0.05);
			glTexCoord2f(0, 0); glVertex3f(0, 0, 0.05);
			glTexCoord2f(0, 0); glVertex3f(-0.02, 0, 0.03);
			glEnd();
			//3
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(0.02, 0.35, 0.03);
			glTexCoord2f(1, 1); glVertex3f(0, 0.35, 0.05);
			glTexCoord2f(1, 0); glVertex3f(0, 0, 0.05);
			glTexCoord2f(0, 0); glVertex3f(0.02, 0, 0.03);
			glEnd();
			//4
			glBegin(GL_TRIANGLES);
			glTexCoord2f(0, 1); glVertex3f(0.02, 0, 0.03);
			glTexCoord2f(1, 1); glVertex3f(-0.02, 0, 0.03);
			glTexCoord2f(1, 0); glVertex3f(0, 0, 0.05);
			glEnd();
			glDeleteTextures(1, &texture[1]);
		}
		glPopMatrix();

		glPushMatrix();
		{//daojian(chidongxidedifang)
			texture[1] = LoadTexture("steel.bmp");
			glBegin(GL_TRIANGLES);
			glTexCoord2f(0, 1); glVertex3f(-0.02, 0, 0);
			glTexCoord2f(1, 0); glVertex3f(0.02, 0, 0);
			glTexCoord2f(1, 1); glVertex3f(0, -0.1, 0);
			glEnd();
			//2
			glBegin(GL_TRIANGLES);
			glTexCoord2f(0, 1); glVertex3f(-0.02, 0, 0);
			glTexCoord2f(1, 0); glVertex3f(-0.02, 0, 0.03);
			glTexCoord2f(1, 1); glVertex3f(0, -0.1, 0);
			glEnd();
			//3
			glBegin(GL_TRIANGLES);
			glTexCoord2f(0, 1); glVertex3f(0.02, 0, 0);
			glTexCoord2f(1, 0); glVertex3f(0.02, 0, 0.03);
			glTexCoord2f(1, 1); glVertex3f(0, -0.1, 0);
			glEnd();
			//4
			glBegin(GL_TRIANGLES);
			glTexCoord2f(0, 1); glVertex3f(-0.02, 0, 0.03);
			glTexCoord2f(1, 0); glVertex3f(0.02, 0, 0.03);
			glTexCoord2f(1, 1); glVertex3f(0, -0.1, 0);
			glEnd();
			//5
			glBegin(GL_TRIANGLES);
			glTexCoord2f(0, 1); glVertex3f(0, -0.1, 0);
			glTexCoord2f(1, 0); glVertex3f(0.02, 0, 0.03);
			glTexCoord2f(1, 1); glVertex3f(0, 0, 0.05);
			glEnd();
			//6
			glBegin(GL_TRIANGLES);
			glTexCoord2f(0, 1); glVertex3f(0, -0.1, 0);
			glTexCoord2f(1, 1); glVertex3f(-0.02, 0, 0.03);
			glTexCoord2f(1, 1); glVertex3f(0, 0, 0.05);
			glEnd();
			glDeleteTextures(1, &texture[1]);
		}
		glPopMatrix();
	}
	glPopMatrix();
}

void rightLeg()
{
	//bigleg
	glPushMatrix();
	{//bigleg
		glTranslatef(-0.1, -0.7, 0.2);
		glPushMatrix();
		{//1
			texture[2] = LoadTexture("blackMetal.bmp");
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.1, 0.7, 0);
			glTexCoord2f(1, 1); glVertex3f(0, 0.7, 0);
			glTexCoord2f(1, 0); glVertex3f(0, 0.4, 0);
			glTexCoord2f(0, 0); glVertex3f(-0.1, 0.4, 0);
			glEnd();
			//2
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.1, 0.7, 0);
			glTexCoord2f(1, 1); glVertex3f(-0.1, 0.7, -0.1);
			glTexCoord2f(1, 0); glVertex3f(0, 0.7, -0.1);
			glTexCoord2f(0, 0); glVertex3f(0, 0.7, 0);
			glEnd();
			//3
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(0, 0.7, 0);
			glTexCoord2f(1, 1); glVertex3f(0, 0.7, -0.1);
			glTexCoord2f(1, 0); glVertex3f(0, 0.4, -0.1);
			glTexCoord2f(0, 0); glVertex3f(0, 0.4, 0);
			glEnd();
			//4
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(0, 0.4, 0);
			glTexCoord2f(1, 1); glVertex3f(0, 0.4, -0.1);
			glTexCoord2f(1, 0); glVertex3f(-0.1, 0.4, -0.1);
			glTexCoord2f(0, 0); glVertex3f(-0.1, 0.4, 0);
			glEnd();
			//5
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.1, 0.4, 0);
			glTexCoord2f(1, 1); glVertex3f(-0.1, 0.4, -0.1);
			glTexCoord2f(1, 0); glVertex3f(-0.1, 0.7, -0.1);
			glTexCoord2f(0, 0); glVertex3f(-0.1, 0.7, 0);
			glEnd();

			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.1, 0.4, -0.1);
			glTexCoord2f(1, 1); glVertex3f(0, 0.4, -0.1);
			glTexCoord2f(1, 0); glVertex3f(0, 0.7, -0.1);
			glTexCoord2f(0, 0); glVertex3f(-0.1, 0.7, -0.1);
			glEnd();

			glDeleteTextures(1, &texture[2]);
		}
		glPopMatrix();

		GLUquadricObj* legCylinder = NULL;
		legCylinder = gluNewQuadric();

		glPushMatrix();
		{
			texture[1] = LoadTexture("steel.bmp");
			glTranslatef(-0.1, 0.8, -0.05);
			glRotatef(90, 1, 0, 0);
			gluQuadricDrawStyle(legCylinder, GLU_FILL);
			gluQuadricTexture(legCylinder, true);
			gluCylinder(legCylinder, 0.03, 0.03, 0.2, 20, 20);
			gluDeleteQuadric(legCylinder);
			glDeleteTextures(1, &texture[1]);
		}
		glPopMatrix();

		glPushMatrix();
		{//1
			texture[3] = LoadTexture("greyMetal.bmp");

			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.2, 0.7, 0);
			glTexCoord2f(1, 1); glVertex3f(-0.1, 0.7, 0);
			glTexCoord2f(1, 0); glVertex3f(-0.1, 0.35, 0);
			glTexCoord2f(0, 0); glVertex3f(-0.2, 0.35, 0);
			glEnd();
			//2
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.2, 0.7, 0);
			glTexCoord2f(1, 1); glVertex3f(-0.2, 0.7, -0.1);
			glTexCoord2f(1, 0); glVertex3f(-0.1, 0.7, -0.1);
			glTexCoord2f(0, 0); glVertex3f(-0.1, 0.7, 0);
			glEnd();
			//3
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.1, 0.7, 0);
			glTexCoord2f(1, 1); glVertex3f(-0.1, 0.7, -0.1);
			glTexCoord2f(1, 0); glVertex3f(-0.1, 0.35, -0.1);
			glTexCoord2f(0, 0); glVertex3f(-0.1, 0.35, 0);
			glEnd();
			//4
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.1, 0.35, 0);
			glTexCoord2f(1, 1); glVertex3f(-0.1, 0.35, -0.1);
			glTexCoord2f(1, 0); glVertex3f(-0.2, 0.35, -0.1);
			glTexCoord2f(0, 0); glVertex3f(-0.2, 0.35, 0);
			glEnd();
			//5
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.2, 0.35, 0);
			glTexCoord2f(1, 1); glVertex3f(-0.2, 0.35, -0.1);
			glTexCoord2f(1, 0); glVertex3f(-0.2, 0.7, -0.1);
			glTexCoord2f(0, 0); glVertex3f(-0.2, 0.7, 0);
			glEnd();

			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.2, 0.7, -0.1);
			glTexCoord2f(1, 1); glVertex3f(-0.1, 0.7, -0.1);
			glTexCoord2f(1, 0); glVertex3f(-0.1, 0.35, -0.1);
			glTexCoord2f(0, 0); glVertex3f(-0.2, 0.35, -0.1);
			glEnd();

			glDeleteTextures(1, &texture[3]);
		}
		glPopMatrix();

		glPushMatrix();
		{//1
			texture[3] = LoadTexture("greyMetal.bmp");

			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.2, 0.75, 0);
			glTexCoord2f(1, 1); glVertex3f(-0.1, 0.75, 0);
			glTexCoord2f(1, 0); glVertex3f(-0.1, 0.35, 0);
			glTexCoord2f(0, 0); glVertex3f(-0.2, 0.35, 0);
			glEnd();
			//2
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.2, 0.75, 0);
			glTexCoord2f(1, 1); glVertex3f(-0.2, 0.75, -0.1);
			glTexCoord2f(1, 0); glVertex3f(-0.1, 0.75, -0.1);
			glTexCoord2f(0, 0); glVertex3f(-0.1, 0.75, 0);
			glEnd();
			//3
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.1, 0.75, 0);
			glTexCoord2f(1, 1); glVertex3f(-0.1, 0.75, -0.1);
			glTexCoord2f(1, 0); glVertex3f(-0.1, 0.35, -0.1);
			glTexCoord2f(0, 0); glVertex3f(-0.1, 0.35, 0);
			glEnd();
			//4
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.1, 0.35, 0);
			glTexCoord2f(1, 1); glVertex3f(-0.1, 0.35, -0.1);
			glTexCoord2f(1, 0); glVertex3f(-0.2, 0.35, -0.1);
			glTexCoord2f(0, 0); glVertex3f(-0.2, 0.35, 0);
			glEnd();
			//5
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.2, 0.35, 0);
			glTexCoord2f(1, 1); glVertex3f(-0.2, 0.35, -0.1);
			glTexCoord2f(1, 0); glVertex3f(-0.2, 0.75, -0.1);
			glTexCoord2f(0, 0); glVertex3f(-0.2, 0.75, 0);
			glEnd();

			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.2, 0.75, -0.1);
			glTexCoord2f(1, 1); glVertex3f(-0.1, 0.75, -0.1);
			glTexCoord2f(1, 0); glVertex3f(-0.1, 0.35, -0.1);
			glTexCoord2f(0, 0); glVertex3f(-0.2, 0.35, -0.1);
			glEnd();

			glDeleteTextures(1, &texture[3]);
		}
		glPopMatrix();

		glPushMatrix();
		{//1
			texture[2] = LoadTexture("blackMetal.bmp");

			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.1, 0.35, 0);
			glTexCoord2f(1, 1); glVertex3f(-0.05, 0.35, 0);
			glTexCoord2f(1, 0); glVertex3f(0, 0.4, 0);
			glTexCoord2f(0, 0); glVertex3f(-0.1, 0.4, 0);
			glEnd();
			//2
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.1, 0.35, 0);
			glTexCoord2f(1, 1); glVertex3f(-0.1, 0.35, -0.1);
			glTexCoord2f(1, 0); glVertex3f(-0.05, 0.35, -0.1);
			glTexCoord2f(0, 0); glVertex3f(-0.05, 0.35, 0);
			glEnd();
			//3
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.05, 0.35, 0);
			glTexCoord2f(1, 1); glVertex3f(-0.05, 0.35, -0.1);
			glTexCoord2f(1, 0); glVertex3f(0, 0.4, -0.1);
			glTexCoord2f(0, 0); glVertex3f(0, 0.4, 0);
			glEnd();
			//4
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(0, 0.4, 0);
			glTexCoord2f(1, 1); glVertex3f(0, 0.4, -0.1);
			glTexCoord2f(1, 0); glVertex3f(-0.1, 0.4, -0.1);
			glTexCoord2f(0, 0); glVertex3f(-0.1, 0.4, 0);
			glEnd();
			//5
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.1, 0.4, 0);
			glTexCoord2f(1, 1); glVertex3f(-0.1, 0.4, -0.1);
			glTexCoord2f(1, 0); glVertex3f(-0.2, 0.75, -0.1);
			glTexCoord2f(0, 0); glVertex3f(-0.2, 0.75, 0);
			glEnd();

			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.1, 0.35, -0.1);
			glTexCoord2f(1, 1); glVertex3f(-0.05, 0.35, -0.1);
			glTexCoord2f(1, 0); glVertex3f(0, 0.4, -0.1);
			glTexCoord2f(0, 0); glVertex3f(-0.1, 0.4, -0.1);
			glEnd();

			glDeleteTextures(1, &texture[2]);
		}
		glPopMatrix();

		glPushMatrix();
		{//1
			texture[2] = LoadTexture("blackMetal.bmp");
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.15, 0.35, 0);
			glTexCoord2f(1, 1); glVertex3f(-0.05, 0.35, 0);
			glTexCoord2f(1, 0); glVertex3f(-0.05, 0.15, 0);
			glTexCoord2f(0, 0); glVertex3f(-0.15, 0.15, 0);
			glEnd();
			//2
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.15, 0.35, 0);
			glTexCoord2f(1, 1); glVertex3f(-0.15, 0.35, -0.1);
			glTexCoord2f(1, 0); glVertex3f(-0.05, 0.35, -0.1);
			glTexCoord2f(0, 0); glVertex3f(-0.05, 0.35, 0);
			glEnd();
			//3
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.05, 0.35, 0);
			glTexCoord2f(1, 1); glVertex3f(-0.05, 0.35, -0.1);
			glTexCoord2f(1, 0); glVertex3f(-0.05, 0.15, -0.1);
			glTexCoord2f(0, 0); glVertex3f(-0.05, 0.15, 0);
			glEnd();
			//4
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.05, 0.15, 0);
			glTexCoord2f(1, 1); glVertex3f(-0.05, 0.15, -0.1);
			glTexCoord2f(1, 0); glVertex3f(-0.15, 0.15, -0.1);
			glTexCoord2f(0, 0); glVertex3f(-0.15, 0.15, 0);
			glEnd();
			//5
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.15, 0.15, 0);
			glTexCoord2f(1, 1); glVertex3f(-0.15, 0.15, -0.1);
			glTexCoord2f(1, 0); glVertex3f(-0.15, 0.35, -0.1);
			glTexCoord2f(0, 0); glVertex3f(-0.15, 0.35, 0);
			glEnd();

			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.15, 0.35, -0.1);
			glTexCoord2f(1, 1); glVertex3f(-0.05, 0.35, -0.1);
			glTexCoord2f(1, 0); glVertex3f(-0.05, 0.15, -0.1);
			glTexCoord2f(0, 0); glVertex3f(-0.15, 0.15, -0.1);
			glEnd();
			glDeleteTextures(1, &texture[2]);
		}
		glPopMatrix();

		//smallleg
		glPushMatrix();
		{//smallleg
			glTranslatef(0, 0.15, 0);
			glRotatef(rotateAngle, 1, 0, 0);
			glTranslatef(0, -0.15, 0);

			GLUquadricObj* rightLegJoint = NULL;
			rightLegJoint = gluNewQuadric();

			glPushMatrix();
			{
				texture[1] = LoadTexture("steel.bmp");
				glTranslatef(-0.1, 0.15, -0.05);
				gluQuadricDrawStyle(rightLegJoint, GLU_FILL);
				gluQuadricTexture(rightLegJoint, true);
				gluSphere(rightLegJoint, radius / 25.0f, slices, stacks);
				gluDeleteQuadric(rightLegJoint);
				glDeleteTextures(1, &texture[1]);
			}
			glPopMatrix();

			glPushMatrix();
			{//1
				texture[3] = LoadTexture("greyMetal.bmp");
				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.15, 0.12, 0);
				glTexCoord2f(1, 1); glVertex3f(-0.05, 0.12, 0);
				glTexCoord2f(1, 0); glVertex3f(-0.05, 0, 0);
				glTexCoord2f(0, 0); glVertex3f(-0.15, 0, 0);
				glEnd();
				//2
				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.15, 0.12, 0);
				glTexCoord2f(1, 1); glVertex3f(-0.15, 0.12, -0.1);
				glTexCoord2f(1, 0); glVertex3f(-0.05, 0.12, -0.1);
				glTexCoord2f(0, 0); glVertex3f(-0.05, 0.12, 0);
				glEnd();
				//3
				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.05, 0.12, 0);
				glTexCoord2f(1, 1); glVertex3f(-0.05, 0.12, -0.1);
				glTexCoord2f(1, 0); glVertex3f(-0.05, 0, -0.1);
				glTexCoord2f(0, 0); glVertex3f(-0.05, 0, 0);
				glEnd();
				//4
				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.05, 0, 0);
				glTexCoord2f(1, 1); glVertex3f(-0.05, 0, -0.1);
				glTexCoord2f(1, 0); glVertex3f(-0.15, 0, -0.1);
				glTexCoord2f(0, 0); glVertex3f(-0.15, 0, 0);
				glEnd();
				//5
				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.15, 0, 0);
				glTexCoord2f(1, 1); glVertex3f(-0.15, 0, -0.1);
				glTexCoord2f(1, 0); glVertex3f(-0.15, 0.12, -0.1);
				glTexCoord2f(0, 0); glVertex3f(-0.15, 0.12, 0);
				glEnd();

				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.15, 0.12, -0.1);
				glTexCoord2f(1, 1); glVertex3f(-0.05, 0.12, -0.1);
				glTexCoord2f(1, 0); glVertex3f(-0.05, 0, -0.1);
				glTexCoord2f(0, 0); glVertex3f(-0.15, 0, -0.1);
				glEnd();

				glDeleteTextures(1, &texture[3]);
			}
			glPopMatrix();

			glPushMatrix();
			{//1
				texture[3] = LoadTexture("greyMetal.bmp");

				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.15, 0, -0.1);
				glTexCoord2f(1, 1); glVertex3f(-0.05, 0, -0.1);
				glTexCoord2f(1, 0); glVertex3f(-0.05, -0.15, -0.05);
				glTexCoord2f(0, 0); glVertex3f(-0.15, -0.15, -0.05);
				glEnd();
				//2
				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.15, 0, -0.1);
				glTexCoord2f(1, 1); glVertex3f(-0.15, 0, 0.05);
				glTexCoord2f(1, 0); glVertex3f(-0.05, 0, 0.05);
				glTexCoord2f(0, 0); glVertex3f(-0.05, 0, -0.1);
				glEnd();
				//3
				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.05, 0, -0.1);
				glTexCoord2f(1, 1); glVertex3f(-0.05, 0, 0.05);
				glTexCoord2f(1, 0); glVertex3f(-0.05, -0.15, 0.05);
				glTexCoord2f(0, 0); glVertex3f(-0.05, -0.15, -0.05);
				glEnd();
				//4
				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.05, -0.15, -0.05);
				glTexCoord2f(1, 1); glVertex3f(-0.05, -0.15, 0.05);
				glTexCoord2f(1, 0); glVertex3f(-0.15, -0.15, 0.05);
				glTexCoord2f(0, 0); glVertex3f(-0.15, -0.15, -0.05);
				glEnd();
				//5
				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.15, -0.15, -0.05);
				glTexCoord2f(1, 1); glVertex3f(-0.15, -0.15, 0.05);
				glTexCoord2f(1, 0); glVertex3f(-0.15, 0, 0.05);
				glTexCoord2f(0, 0); glVertex3f(-0.15, 0, -0.1);
				glEnd();

				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.15, 0, 0.05);
				glTexCoord2f(1, 1); glVertex3f(-0.05, 0, 0.05);
				glTexCoord2f(1, 0); glVertex3f(-0.05, -0.15, 0.05);
				glTexCoord2f(0, 0); glVertex3f(-0.15, -0.15, 0.05);
				glEnd();

				glDeleteTextures(1, &texture[3]);
			}
			glPopMatrix();

			glPushMatrix();
			{//1
				texture[3] = LoadTexture("blackMetal.bmp");
				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.12, 0, 0.05);
				glTexCoord2f(1, 1); glVertex3f(-0.08, 0, 0.05);
				glTexCoord2f(1, 0); glVertex3f(-0.08, -0.4, 0.05);
				glTexCoord2f(0, 0); glVertex3f(-0.12, -0.4, 0.05);
				glEnd();
				//2
				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.12, 0, 0.05);
				glTexCoord2f(1, 1); glVertex3f(-0.12, 0, 0.07);
				glTexCoord2f(1, 0); glVertex3f(-0.08, 0, 0.07);
				glTexCoord2f(0, 0); glVertex3f(-0.08, 0, 0.05);
				glEnd();
				//3
				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.08, 0, 0.05);
				glTexCoord2f(1, 1); glVertex3f(-0.08, 0, 0.07);
				glTexCoord2f(1, 0); glVertex3f(-0.08, -0.35, 0.07);
				glTexCoord2f(0, 0); glVertex3f(-0.08, -0.4, 0.05);
				glEnd();
				//4
				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.08, -0.4, 0.05);
				glTexCoord2f(1, 1); glVertex3f(-0.08, -0.35, 0.07);
				glTexCoord2f(1, 0); glVertex3f(-0.12, -0.35, 0.07);
				glTexCoord2f(0, 0); glVertex3f(-0.12, -0.4, 0.05);
				glEnd();
				//5
				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.12, -0.4, 0.05);
				glTexCoord2f(1, 1); glVertex3f(-0.12, -0.35, 0.07);
				glTexCoord2f(1, 0); glVertex3f(-0.12, 0, 0.07);
				glTexCoord2f(0, 0); glVertex3f(-0.12, 0, 0.05);
				glEnd();

				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.12, 0, 0.07);
				glTexCoord2f(1, 1); glVertex3f(-0.08, 0, 0.07);
				glTexCoord2f(1, 0); glVertex3f(-0.08, -0.35, 0.07);
				glTexCoord2f(0, 0); glVertex3f(-0.12, -0.35, 0.07);
				glEnd();
				glDeleteTextures(1, &texture[2]);
			}
			glPopMatrix();

			glPushMatrix();
			{//1
				texture[2] = LoadTexture("blackMetal.bmp");
				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.12, 0.1, 0.07);
				glTexCoord2f(1, 1); glVertex3f(-0.08, 0.1, 0.07);
				glTexCoord2f(1, 0); glVertex3f(-0.08, -0.2, 0.07);
				glTexCoord2f(0, 0); glVertex3f(-0.12, -0.2, 0.07);
				glEnd();
				//2
				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.12, 0.1, 0.07);
				glTexCoord2f(1, 1); glVertex3f(-0.12, 0.1, 0.09);
				glTexCoord2f(1, 0); glVertex3f(-0.08, 0.1, 0.09);
				glTexCoord2f(0, 0); glVertex3f(-0.08, 0.1, 0.07);
				glEnd();
				//3
				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.08, 0.1, 0.07);
				glTexCoord2f(1, 1); glVertex3f(-0.08, 0.1, 0.09);
				glTexCoord2f(1, 0); glVertex3f(-0.08, -0.15, 0.09);
				glTexCoord2f(0, 0); glVertex3f(-0.08, -0.2, 0.07);
				glEnd();
				//4
				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.08, -0.2, 0.07);
				glTexCoord2f(1, 1); glVertex3f(-0.08, -0.15, 0.09);
				glTexCoord2f(1, 0); glVertex3f(-0.12, -0.15, 0.09);
				glTexCoord2f(0, 0); glVertex3f(-0.12, -0.2, 0.07);
				glEnd();
				//5
				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.12, -0.2, 0.07);
				glTexCoord2f(1, 1); glVertex3f(-0.12, -0.15, 0.09);
				glTexCoord2f(1, 0); glVertex3f(-0.12, 0.1, 0.09);
				glTexCoord2f(0, 0); glVertex3f(-0.12, 0.1, 0.07);
				glEnd();

				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.12, 0.1, 0.09);
				glTexCoord2f(1, 1); glVertex3f(-0.08, 0.1, 0.09);
				glTexCoord2f(1, 0); glVertex3f(-0.08, -0.15, 0.09);
				glTexCoord2f(0, 0); glVertex3f(-0.12, -0.2, 0.07);
				glEnd();
				glDeleteTextures(1, &texture[2]);
			}
			glPopMatrix();

			//smallsmallleg
			glPushMatrix();
			{//1
				texture[3] = LoadTexture("greyMetal.bmp");
				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.15, -0.15, -0.05);
				glTexCoord2f(1, 1); glVertex3f(-0.05, -0.15, -0.05);
				glTexCoord2f(1, 0); glVertex3f(-0.05, -0.4, -0.03);
				glTexCoord2f(0, 0); glVertex3f(-0.15, -0.4, -0.03);
				glEnd();
				//2
				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.15, -0.15, -0.05);
				glTexCoord2f(1, 1); glVertex3f(-0.15, -0.15, 0.05);
				glTexCoord2f(1, 0); glVertex3f(-0.05, -0.15, 0.05);
				glTexCoord2f(0, 0); glVertex3f(-0.05, -0.15, -0.05);
				glEnd();
				//3
				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.05, -0.15, -0.05);
				glTexCoord2f(1, 1); glVertex3f(-0.05, -0.15, 0.05);
				glTexCoord2f(1, 0); glVertex3f(-0.05, -0.4, 0.05);
				glTexCoord2f(0, 1); glVertex3f(-0.05, -0.4, -0.03);
				glEnd();
				//4
				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.05, -0.4, -0.03);
				glTexCoord2f(1, 1); glVertex3f(-0.05, -0.4, 0.05);
				glTexCoord2f(1, 0); glVertex3f(-0.15, -0.4, 0.05);
				glTexCoord2f(0, 0); glVertex3f(-0.15, -0.4, -0.03);
				glEnd();
				//5
				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.15, -0.4, -0.03);
				glTexCoord2f(1, 1); glVertex3f(-0.15, -0.4, 0.05);
				glTexCoord2f(1, 0); glVertex3f(-0.15, -0.15, 0.05);
				glTexCoord2f(0, 0); glVertex3f(-0.15, -0.15, -0.05);
				glEnd();

				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.15, -0.15, 0.05);
				glTexCoord2f(1, 1); glVertex3f(-0.05, -0.15, 0.05);
				glTexCoord2f(1, 0); glVertex3f(-0.05, -0.4, 0.05);
				glTexCoord2f(0, 0); glVertex3f(-0.15, -0.4, 0.05);
				glEnd();
				glDeleteTextures(1, &texture[3]);

				glPushMatrix();
				{//smallsmallleg

					GLUquadricObj* rightSmallLegJoint = NULL;
					rightSmallLegJoint = gluNewQuadric();

					glPushMatrix();
					{
						glTranslatef(-0.1, -0.4, 0);
						texture[1] = LoadTexture("steel.bmp");
						gluQuadricDrawStyle(rightSmallLegJoint, GLU_FILL);
						gluQuadricTexture(rightSmallLegJoint, true);
						gluSphere(rightSmallLegJoint, radius / 35.0f, slices, stacks);
						gluDeleteQuadric(rightSmallLegJoint);
						glDeleteTextures(1, &texture[1]);
					}
					glPopMatrix();

					glPushMatrix();
					{
						glTranslatef(0.05, -0.42, -0.02);
						feet();
					}
					glPopMatrix();
				}
				glPopMatrix();
			}
			glPopMatrix();
		}
		glPopMatrix();
	}
	glPopMatrix();
}

void leftLeg()
{
	//bigleg
	glPushMatrix();
	{//bigleg
		glTranslatef(0.1, -0.7, 0.1);
		glRotatef(180, 0, 1, 0);
		glPushMatrix();
		{//1
			texture[2] = LoadTexture("blackMetal.bmp");
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.1, 0.7, 0);
			glTexCoord2f(1, 1); glVertex3f(0, 0.7, 0);
			glTexCoord2f(1, 0); glVertex3f(0, 0.4, 0);
			glTexCoord2f(0, 0); glVertex3f(-0.1, 0.4, 0);
			glEnd();
			//2
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.1, 0.7, 0);
			glTexCoord2f(1, 1); glVertex3f(-0.1, 0.7, -0.1);
			glTexCoord2f(1, 0); glVertex3f(0, 0.7, -0.1);
			glTexCoord2f(0, 0); glVertex3f(0, 0.7, 0);
			glEnd();
			//3
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(0, 0.7, 0);
			glTexCoord2f(1, 1); glVertex3f(0, 0.7, -0.1);
			glTexCoord2f(1, 0); glVertex3f(0, 0.4, -0.1);
			glTexCoord2f(0, 0); glVertex3f(0, 0.4, 0);
			glEnd();
			//4
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(0, 0.4, 0);
			glTexCoord2f(1, 1); glVertex3f(0, 0.4, -0.1);
			glTexCoord2f(1, 0); glVertex3f(-0.1, 0.4, -0.1);
			glTexCoord2f(0, 0); glVertex3f(-0.1, 0.4, 0);
			glEnd();
			//5
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.1, 0.4, 0);
			glTexCoord2f(1, 1); glVertex3f(-0.1, 0.4, -0.1);
			glTexCoord2f(1, 0); glVertex3f(-0.1, 0.7, -0.1);
			glTexCoord2f(0, 0); glVertex3f(-0.1, 0.7, 0);
			glEnd();

			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.1, 0.4, -0.1);
			glTexCoord2f(1, 1); glVertex3f(0, 0.4, -0.1);
			glTexCoord2f(1, 0); glVertex3f(0, 0.7, -0.1);
			glTexCoord2f(0, 0); glVertex3f(-0.1, 0.7, -0.1);
			glEnd();

			glDeleteTextures(1, &texture[2]);
		}
		glPopMatrix();

		GLUquadricObj* legCylinder = NULL;
		legCylinder = gluNewQuadric();

		glPushMatrix();
		{
			texture[1] = LoadTexture("steel.bmp");
			glTranslatef(-0.1, 0.8, -0.05);
			glRotatef(90, 1, 0, 0);
			gluQuadricDrawStyle(legCylinder, GLU_FILL);
			gluQuadricTexture(legCylinder, true);
			gluCylinder(legCylinder, 0.03, 0.03, 0.2, 20, 20);
			gluDeleteQuadric(legCylinder);
			glDeleteTextures(1, &texture[1]);
		}
		glPopMatrix();

		glPushMatrix();
		{//1
			texture[3] = LoadTexture("greyMetal.bmp");

			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.2, 0.7, 0);
			glTexCoord2f(1, 1); glVertex3f(-0.1, 0.7, 0);
			glTexCoord2f(1, 0); glVertex3f(-0.1, 0.35, 0);
			glTexCoord2f(0, 0); glVertex3f(-0.2, 0.35, 0);
			glEnd();
			//2
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.2, 0.7, 0);
			glTexCoord2f(1, 1); glVertex3f(-0.2, 0.7, -0.1);
			glTexCoord2f(1, 0); glVertex3f(-0.1, 0.7, -0.1);
			glTexCoord2f(0, 0); glVertex3f(-0.1, 0.7, 0);
			glEnd();
			//3
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.1, 0.7, 0);
			glTexCoord2f(1, 1); glVertex3f(-0.1, 0.7, -0.1);
			glTexCoord2f(1, 0); glVertex3f(-0.1, 0.35, -0.1);
			glTexCoord2f(0, 0); glVertex3f(-0.1, 0.35, 0);
			glEnd();
			//4
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.1, 0.35, 0);
			glTexCoord2f(1, 1); glVertex3f(-0.1, 0.35, -0.1);
			glTexCoord2f(1, 0); glVertex3f(-0.2, 0.35, -0.1);
			glTexCoord2f(0, 0); glVertex3f(-0.2, 0.35, 0);
			glEnd();
			//5
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.2, 0.35, 0);
			glTexCoord2f(1, 1); glVertex3f(-0.2, 0.35, -0.1);
			glTexCoord2f(1, 0); glVertex3f(-0.2, 0.7, -0.1);
			glTexCoord2f(0, 0); glVertex3f(-0.2, 0.7, 0);
			glEnd();

			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.2, 0.7, -0.1);
			glTexCoord2f(1, 1); glVertex3f(-0.1, 0.7, -0.1);
			glTexCoord2f(1, 0); glVertex3f(-0.1, 0.35, -0.1);
			glTexCoord2f(0, 0); glVertex3f(-0.2, 0.35, -0.1);
			glEnd();

			glDeleteTextures(1, &texture[3]);
		}
		glPopMatrix();

		glPushMatrix();
		{//1
			texture[3] = LoadTexture("greyMetal.bmp");

			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.2, 0.75, 0);
			glTexCoord2f(1, 1); glVertex3f(-0.1, 0.75, 0);
			glTexCoord2f(1, 0); glVertex3f(-0.1, 0.35, 0);
			glTexCoord2f(0, 0); glVertex3f(-0.2, 0.35, 0);
			glEnd();
			//2
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.2, 0.75, 0);
			glTexCoord2f(1, 1); glVertex3f(-0.2, 0.75, -0.1);
			glTexCoord2f(1, 0); glVertex3f(-0.1, 0.75, -0.1);
			glTexCoord2f(0, 0); glVertex3f(-0.1, 0.75, 0);
			glEnd();
			//3
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.1, 0.75, 0);
			glTexCoord2f(1, 1); glVertex3f(-0.1, 0.75, -0.1);
			glTexCoord2f(1, 0); glVertex3f(-0.1, 0.35, -0.1);
			glTexCoord2f(0, 0); glVertex3f(-0.1, 0.35, 0);
			glEnd();
			//4
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.1, 0.35, 0);
			glTexCoord2f(1, 1); glVertex3f(-0.1, 0.35, -0.1);
			glTexCoord2f(1, 0); glVertex3f(-0.2, 0.35, -0.1);
			glTexCoord2f(0, 0); glVertex3f(-0.2, 0.35, 0);
			glEnd();
			//5
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.2, 0.35, 0);
			glTexCoord2f(1, 1); glVertex3f(-0.2, 0.35, -0.1);
			glTexCoord2f(1, 0); glVertex3f(-0.2, 0.75, -0.1);
			glTexCoord2f(0, 0); glVertex3f(-0.2, 0.75, 0);
			glEnd();

			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.2, 0.75, -0.1);
			glTexCoord2f(1, 1); glVertex3f(-0.1, 0.75, -0.1);
			glTexCoord2f(1, 0); glVertex3f(-0.1, 0.35, -0.1);
			glTexCoord2f(0, 0); glVertex3f(-0.2, 0.35, -0.1);
			glEnd();

			glDeleteTextures(1, &texture[3]);
		}
		glPopMatrix();

		glPushMatrix();
		{//1
			texture[2] = LoadTexture("blackMetal.bmp");

			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.1, 0.35, 0);
			glTexCoord2f(1, 1); glVertex3f(-0.05, 0.35, 0);
			glTexCoord2f(1, 0); glVertex3f(0, 0.4, 0);
			glTexCoord2f(0, 0); glVertex3f(-0.1, 0.4, 0);
			glEnd();
			//2
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.1, 0.35, 0);
			glTexCoord2f(1, 1); glVertex3f(-0.1, 0.35, -0.1);
			glTexCoord2f(1, 0); glVertex3f(-0.05, 0.35, -0.1);
			glTexCoord2f(0, 0); glVertex3f(-0.05, 0.35, 0);
			glEnd();
			//3
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.05, 0.35, 0);
			glTexCoord2f(1, 1); glVertex3f(-0.05, 0.35, -0.1);
			glTexCoord2f(1, 0); glVertex3f(0, 0.4, -0.1);
			glTexCoord2f(0, 0); glVertex3f(0, 0.4, 0);
			glEnd();
			//4
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(0, 0.4, 0);
			glTexCoord2f(1, 1); glVertex3f(0, 0.4, -0.1);
			glTexCoord2f(1, 0); glVertex3f(-0.1, 0.4, -0.1);
			glTexCoord2f(0, 0); glVertex3f(-0.1, 0.4, 0);
			glEnd();
			//5
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.1, 0.4, 0);
			glTexCoord2f(1, 1); glVertex3f(-0.1, 0.4, -0.1);
			glTexCoord2f(1, 0); glVertex3f(-0.2, 0.75, -0.1);
			glTexCoord2f(0, 0); glVertex3f(-0.2, 0.75, 0);
			glEnd();

			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.1, 0.35, -0.1);
			glTexCoord2f(1, 1); glVertex3f(-0.05, 0.35, -0.1);
			glTexCoord2f(1, 0); glVertex3f(0, 0.4, -0.1);
			glTexCoord2f(0, 0); glVertex3f(-0.1, 0.4, -0.1);
			glEnd();

			glDeleteTextures(1, &texture[2]);
		}
		glPopMatrix();

		glPushMatrix();
		{//1
			texture[2] = LoadTexture("blackMetal.bmp");
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.15, 0.35, 0);
			glTexCoord2f(1, 1); glVertex3f(-0.05, 0.35, 0);
			glTexCoord2f(1, 0); glVertex3f(-0.05, 0.15, 0);
			glTexCoord2f(0, 0); glVertex3f(-0.15, 0.15, 0);
			glEnd();
			//2
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.15, 0.35, 0);
			glTexCoord2f(1, 1); glVertex3f(-0.15, 0.35, -0.1);
			glTexCoord2f(1, 0); glVertex3f(-0.05, 0.35, -0.1);
			glTexCoord2f(0, 0); glVertex3f(-0.05, 0.35, 0);
			glEnd();
			//3
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.05, 0.35, 0);
			glTexCoord2f(1, 1); glVertex3f(-0.05, 0.35, -0.1);
			glTexCoord2f(1, 0); glVertex3f(-0.05, 0.15, -0.1);
			glTexCoord2f(0, 0); glVertex3f(-0.05, 0.15, 0);
			glEnd();
			//4
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.05, 0.15, 0);
			glTexCoord2f(1, 1); glVertex3f(-0.05, 0.15, -0.1);
			glTexCoord2f(1, 0); glVertex3f(-0.15, 0.15, -0.1);
			glTexCoord2f(0, 0); glVertex3f(-0.15, 0.15, 0);
			glEnd();
			//5
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.15, 0.15, 0);
			glTexCoord2f(1, 1); glVertex3f(-0.15, 0.15, -0.1);
			glTexCoord2f(1, 0); glVertex3f(-0.15, 0.35, -0.1);
			glTexCoord2f(0, 0); glVertex3f(-0.15, 0.35, 0);
			glEnd();

			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.15, 0.35, -0.1);
			glTexCoord2f(1, 1); glVertex3f(-0.05, 0.35, -0.1);
			glTexCoord2f(1, 0); glVertex3f(-0.05, 0.15, -0.1);
			glTexCoord2f(0, 0); glVertex3f(-0.15, 0.15, -0.1);
			glEnd();
			glDeleteTextures(1, &texture[2]);
		}
		glPopMatrix();

		//smallleg
		glPushMatrix();
		{//smallleg
			glRotatef(180, 0, 1, 0);
			glTranslatef(0.2, 0, 0.1);
			glTranslatef(0, 0.15, 0);
			glRotatef(rotateAngle, 1, 0, 0);
			glTranslatef(0, -0.15, 0);

			GLUquadricObj* rightLegJoint = NULL;
			rightLegJoint = gluNewQuadric();

			glPushMatrix();
			{
				texture[1] = LoadTexture("steel.bmp");
				glTranslatef(-0.1, 0.15, -0.05);
				gluQuadricDrawStyle(rightLegJoint, GLU_FILL);
				gluQuadricTexture(rightLegJoint, true);
				gluSphere(rightLegJoint, radius / 25.0f, slices, stacks);
				gluDeleteQuadric(rightLegJoint);
				glDeleteTextures(1, &texture[1]);
			}
			glPopMatrix();

			glPushMatrix();
			{//1
				texture[3] = LoadTexture("greyMetal.bmp");
				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.15, 0.12, 0);
				glTexCoord2f(1, 1); glVertex3f(-0.05, 0.12, 0);
				glTexCoord2f(1, 0); glVertex3f(-0.05, 0, 0);
				glTexCoord2f(0, 0); glVertex3f(-0.15, 0, 0);
				glEnd();
				//2
				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.15, 0.12, 0);
				glTexCoord2f(1, 1); glVertex3f(-0.15, 0.12, -0.1);
				glTexCoord2f(1, 0); glVertex3f(-0.05, 0.12, -0.1);
				glTexCoord2f(0, 0); glVertex3f(-0.05, 0.12, 0);
				glEnd();
				//3
				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.05, 0.12, 0);
				glTexCoord2f(1, 1); glVertex3f(-0.05, 0.12, -0.1);
				glTexCoord2f(1, 0); glVertex3f(-0.05, 0, -0.1);
				glTexCoord2f(0, 0); glVertex3f(-0.05, 0, 0);
				glEnd();
				//4
				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.05, 0, 0);
				glTexCoord2f(1, 1); glVertex3f(-0.05, 0, -0.1);
				glTexCoord2f(1, 0); glVertex3f(-0.15, 0, -0.1);
				glTexCoord2f(0, 0); glVertex3f(-0.15, 0, 0);
				glEnd();
				//5
				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.15, 0, 0);
				glTexCoord2f(1, 1); glVertex3f(-0.15, 0, -0.1);
				glTexCoord2f(1, 0); glVertex3f(-0.15, 0.12, -0.1);
				glTexCoord2f(0, 0); glVertex3f(-0.15, 0.12, 0);
				glEnd();

				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.15, 0.12, -0.1);
				glTexCoord2f(1, 1); glVertex3f(-0.05, 0.12, -0.1);
				glTexCoord2f(1, 0); glVertex3f(-0.05, 0, -0.1);
				glTexCoord2f(0, 0); glVertex3f(-0.15, 0, -0.1);
				glEnd();

				glDeleteTextures(1, &texture[3]);
			}
			glPopMatrix();

			glPushMatrix();
			{//1
				texture[3] = LoadTexture("greyMetal.bmp");

				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.15, 0, -0.1);
				glTexCoord2f(1, 1); glVertex3f(-0.05, 0, -0.1);
				glTexCoord2f(1, 0); glVertex3f(-0.05, -0.15, -0.05);
				glTexCoord2f(0, 0); glVertex3f(-0.15, -0.15, -0.05);
				glEnd();
				//2
				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.15, 0, -0.1);
				glTexCoord2f(1, 1); glVertex3f(-0.15, 0, 0.05);
				glTexCoord2f(1, 0); glVertex3f(-0.05, 0, 0.05);
				glTexCoord2f(0, 0); glVertex3f(-0.05, 0, -0.1);
				glEnd();
				//3
				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.05, 0, -0.1);
				glTexCoord2f(1, 1); glVertex3f(-0.05, 0, 0.05);
				glTexCoord2f(1, 0); glVertex3f(-0.05, -0.15, 0.05);
				glTexCoord2f(0, 0); glVertex3f(-0.05, -0.15, -0.05);
				glEnd();
				//4
				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.05, -0.15, -0.05);
				glTexCoord2f(1, 1); glVertex3f(-0.05, -0.15, 0.05);
				glTexCoord2f(1, 0); glVertex3f(-0.15, -0.15, 0.05);
				glTexCoord2f(0, 0); glVertex3f(-0.15, -0.15, -0.05);
				glEnd();
				//5
				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.15, -0.15, -0.05);
				glTexCoord2f(1, 1); glVertex3f(-0.15, -0.15, 0.05);
				glTexCoord2f(1, 0); glVertex3f(-0.15, 0, 0.05);
				glTexCoord2f(0, 0); glVertex3f(-0.15, 0, -0.1);
				glEnd();

				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.15, 0, 0.05);
				glTexCoord2f(1, 1); glVertex3f(-0.05, 0, 0.05);
				glTexCoord2f(1, 0); glVertex3f(-0.05, -0.15, 0.05);
				glTexCoord2f(0, 0); glVertex3f(-0.15, -0.15, 0.05);
				glEnd();

				glDeleteTextures(1, &texture[3]);
			}
			glPopMatrix();

			glPushMatrix();
			{//1
				texture[3] = LoadTexture("blackMetal.bmp");
				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.12, 0, 0.05);
				glTexCoord2f(1, 1); glVertex3f(-0.08, 0, 0.05);
				glTexCoord2f(1, 0); glVertex3f(-0.08, -0.4, 0.05);
				glTexCoord2f(0, 0); glVertex3f(-0.12, -0.4, 0.05);
				glEnd();
				//2
				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.12, 0, 0.05);
				glTexCoord2f(1, 1); glVertex3f(-0.12, 0, 0.07);
				glTexCoord2f(1, 0); glVertex3f(-0.08, 0, 0.07);
				glTexCoord2f(0, 0); glVertex3f(-0.08, 0, 0.05);
				glEnd();
				//3
				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.08, 0, 0.05);
				glTexCoord2f(1, 1); glVertex3f(-0.08, 0, 0.07);
				glTexCoord2f(1, 0); glVertex3f(-0.08, -0.35, 0.07);
				glTexCoord2f(0, 0); glVertex3f(-0.08, -0.4, 0.05);
				glEnd();
				//4
				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.08, -0.4, 0.05);
				glTexCoord2f(1, 1); glVertex3f(-0.08, -0.35, 0.07);
				glTexCoord2f(1, 0); glVertex3f(-0.12, -0.35, 0.07);
				glTexCoord2f(0, 0); glVertex3f(-0.12, -0.4, 0.05);
				glEnd();
				//5
				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.12, -0.4, 0.05);
				glTexCoord2f(1, 1); glVertex3f(-0.12, -0.35, 0.07);
				glTexCoord2f(1, 0); glVertex3f(-0.12, 0, 0.07);
				glTexCoord2f(0, 0); glVertex3f(-0.12, 0, 0.05);
				glEnd();

				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.12, 0, 0.07);
				glTexCoord2f(1, 1); glVertex3f(-0.08, 0, 0.07);
				glTexCoord2f(1, 0); glVertex3f(-0.08, -0.35, 0.07);
				glTexCoord2f(0, 0); glVertex3f(-0.12, -0.35, 0.07);
				glEnd();
				glDeleteTextures(1, &texture[2]);
			}
			glPopMatrix();

			glPushMatrix();
			{//1
				texture[2] = LoadTexture("blackMetal.bmp");
				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.12, 0.1, 0.07);
				glTexCoord2f(1, 1); glVertex3f(-0.08, 0.1, 0.07);
				glTexCoord2f(1, 0); glVertex3f(-0.08, -0.2, 0.07);
				glTexCoord2f(0, 0); glVertex3f(-0.12, -0.2, 0.07);
				glEnd();
				//2
				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.12, 0.1, 0.07);
				glTexCoord2f(1, 1); glVertex3f(-0.12, 0.1, 0.09);
				glTexCoord2f(1, 0); glVertex3f(-0.08, 0.1, 0.09);
				glTexCoord2f(0, 0); glVertex3f(-0.08, 0.1, 0.07);
				glEnd();
				//3
				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.08, 0.1, 0.07);
				glTexCoord2f(1, 1); glVertex3f(-0.08, 0.1, 0.09);
				glTexCoord2f(1, 0); glVertex3f(-0.08, -0.15, 0.09);
				glTexCoord2f(0, 0); glVertex3f(-0.08, -0.2, 0.07);
				glEnd();
				//4
				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.08, -0.2, 0.07);
				glTexCoord2f(1, 1); glVertex3f(-0.08, -0.15, 0.09);
				glTexCoord2f(1, 0); glVertex3f(-0.12, -0.15, 0.09);
				glTexCoord2f(0, 0); glVertex3f(-0.12, -0.2, 0.07);
				glEnd();
				//5
				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.12, -0.2, 0.07);
				glTexCoord2f(1, 1); glVertex3f(-0.12, -0.15, 0.09);
				glTexCoord2f(1, 0); glVertex3f(-0.12, 0.1, 0.09);
				glTexCoord2f(0, 0); glVertex3f(-0.12, 0.1, 0.07);
				glEnd();

				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.12, 0.1, 0.09);
				glTexCoord2f(1, 1); glVertex3f(-0.08, 0.1, 0.09);
				glTexCoord2f(1, 0); glVertex3f(-0.08, -0.15, 0.09);
				glTexCoord2f(0, 0); glVertex3f(-0.12, -0.2, 0.07);
				glEnd();
				glDeleteTextures(1, &texture[2]);
			}
			glPopMatrix();

			//smallsmallleg
			glPushMatrix();
			{//1
				texture[3] = LoadTexture("greyMetal.bmp");
				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.15, -0.15, -0.05);
				glTexCoord2f(1, 1); glVertex3f(-0.05, -0.15, -0.05);
				glTexCoord2f(1, 0); glVertex3f(-0.05, -0.4, -0.03);
				glTexCoord2f(0, 0); glVertex3f(-0.15, -0.4, -0.03);
				glEnd();
				//2
				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.15, -0.15, -0.05);
				glTexCoord2f(1, 1); glVertex3f(-0.15, -0.15, 0.05);
				glTexCoord2f(1, 0); glVertex3f(-0.05, -0.15, 0.05);
				glTexCoord2f(0, 0); glVertex3f(-0.05, -0.15, -0.05);
				glEnd();
				//3
				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.05, -0.15, -0.05);
				glTexCoord2f(1, 1); glVertex3f(-0.05, -0.15, 0.05);
				glTexCoord2f(1, 0); glVertex3f(-0.05, -0.4, 0.05);
				glTexCoord2f(0, 1); glVertex3f(-0.05, -0.4, -0.03);
				glEnd();
				//4
				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.05, -0.4, -0.03);
				glTexCoord2f(1, 1); glVertex3f(-0.05, -0.4, 0.05);
				glTexCoord2f(1, 0); glVertex3f(-0.15, -0.4, 0.05);
				glTexCoord2f(0, 0); glVertex3f(-0.15, -0.4, -0.03);
				glEnd();
				//5
				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.15, -0.4, -0.03);
				glTexCoord2f(1, 1); glVertex3f(-0.15, -0.4, 0.05);
				glTexCoord2f(1, 0); glVertex3f(-0.15, -0.15, 0.05);
				glTexCoord2f(0, 0); glVertex3f(-0.15, -0.15, -0.05);
				glEnd();

				glBegin(GL_POLYGON);
				glTexCoord2f(0, 1); glVertex3f(-0.15, -0.15, 0.05);
				glTexCoord2f(1, 1); glVertex3f(-0.05, -0.15, 0.05);
				glTexCoord2f(1, 0); glVertex3f(-0.05, -0.4, 0.05);
				glTexCoord2f(0, 0); glVertex3f(-0.15, -0.4, 0.05);
				glEnd();
				glDeleteTextures(1, &texture[3]);

				glPushMatrix();
				{//smallsmallleg

					GLUquadricObj* rightSmallLegJoint = NULL;
					rightSmallLegJoint = gluNewQuadric();

					glPushMatrix();
					{
						glTranslatef(-0.1, -0.4, 0);
						texture[1] = LoadTexture("steel.bmp");
						gluQuadricDrawStyle(rightSmallLegJoint, GLU_FILL);
						gluQuadricTexture(rightSmallLegJoint, true);
						gluSphere(rightSmallLegJoint, radius / 35.0f, slices, stacks);
						gluDeleteQuadric(rightSmallLegJoint);
						glDeleteTextures(1, &texture[1]);
					}
					glPopMatrix();

					glPushMatrix();
					{
						glTranslatef(0.05, -0.42, -0.02);
						feet();
					}
					glPopMatrix();
				}
				glPopMatrix();
			}
			glPopMatrix();
		}
		glPopMatrix();
	}
	glPopMatrix();
}

void feet()
{
	glPushMatrix();
	{//1
		texture[3] = LoadTexture("greyMetal.bmp");
		glBegin(GL_TRIANGLES);
		glTexCoord2f(1, 0); glVertex3f(-0.2, 0, 0);
		glTexCoord2f(0, 1); glVertex3f(-0.1, 0, 0);
		glTexCoord2f(0.5, 0); glVertex3f(-0.15, -0.1, -0.1);
		glEnd();
		//2
		glBegin(GL_TRIANGLES);
		glTexCoord2f(1, 0); glVertex3f(-0.2, 0, 0);
		glTexCoord2f(0, 1); glVertex3f(-0.2, -0.05, 0);
		glTexCoord2f(0.5, 0); glVertex3f(-0.15, -0.1, -0.1);
		glEnd();
		//3
		glBegin(GL_TRIANGLES);
		glTexCoord2f(1, 0); glVertex3f(-0.1, 0, 0);
		glTexCoord2f(0, 1); glVertex3f(-0.1, -0.05, 0);
		glTexCoord2f(0.5, 0); glVertex3f(-0.15, -0.1, -0.1);
		glEnd();
		//4
		glBegin(GL_TRIANGLES);
		glTexCoord2f(1, 0); glVertex3f(-0.1, -0.05, 0);
		glTexCoord2f(0, 1); glVertex3f(-0.2, -0.05, 0);
		glTexCoord2f(0.5, 0); glVertex3f(-0.15, -0.1, -0.1);
		glEnd();
		//5
		glBegin(GL_POLYGON);
		glTexCoord2f(0, 1); glVertex3f(-0.1, -0.05, 0);
		glTexCoord2f(1, 1); glVertex3f(-0.2, -0.05, 0);
		glTexCoord2f(1, 0); glVertex3f(-0.2, 0, 0);
		glTexCoord2f(0, 0); glVertex3f(-0.1, 0, 0);
		glEnd();
		glDeleteTextures(1, &texture[3]);
	}
	glPopMatrix();

	glPushMatrix();
	{//1
		texture[2] = LoadTexture("blackMetal.bmp");
		glBegin(GL_POLYGON);
		glTexCoord2f(0, 1); glVertex3f(-0.2, 0, 0);
		glTexCoord2f(1, 1); glVertex3f(-0.1, 0, 0);
		glTexCoord2f(1, 0); glVertex3f(-0.1, -0.05, 0);
		glTexCoord2f(0, 0); glVertex3f(-0.2, -0.05, 0);
		glEnd();
		//2
		glBegin(GL_POLYGON);
		glTexCoord2f(0, 1); glVertex3f(-0.2, 0, 0);
		glTexCoord2f(1, 1); glVertex3f(-0.2, 0, 0.05);
		glTexCoord2f(1, 0); glVertex3f(-0.1, 0, 0.05);
		glTexCoord2f(0, 0); glVertex3f(-0.1, 0, 0);
		glEnd();
		//3
		glBegin(GL_POLYGON);
		glTexCoord2f(0, 1); glVertex3f(-0.1, 0, 0);
		glTexCoord2f(1, 1); glVertex3f(-0.1, 0, 0.05);
		glTexCoord2f(1, 0); glVertex3f(-0.1, -0.05, 0.05);
		glTexCoord2f(0, 0); glVertex3f(-0.1, -0.05, 0);
		glEnd();
		//4
		glBegin(GL_POLYGON);
		glTexCoord2f(0, 1); glVertex3f(-0.1, -0.05, 0);
		glTexCoord2f(1, 1); glVertex3f(-0.1, -0.05, 0.05);
		glTexCoord2f(1, 0); glVertex3f(-0.2, -0.05, 0.05);
		glTexCoord2f(0, 0); glVertex3f(-0.2, -0.05, 0);
		glEnd();
		//5
		glBegin(GL_POLYGON);
		glTexCoord2f(0, 1); glVertex3f(-0.2, -0.05, 0);
		glTexCoord2f(1, 1); glVertex3f(-0.2, -0.05, 0.05);
		glTexCoord2f(1, 0); glVertex3f(-0.2, 0, 0.05);
		glTexCoord2f(0, 0); glVertex3f(-0.2, 0, 0);
		glEnd();

		glBegin(GL_POLYGON);
		glTexCoord2f(0, 1); glVertex3f(-0.2, 0, 0.05);
		glTexCoord2f(1, 1); glVertex3f(-0.1, 0, 0.05);
		glTexCoord2f(1, 0); glVertex3f(-0.1, -0.05, 0.05);
		glTexCoord2f(0, 0); glVertex3f(-0.2, -0.05, 0.05);
		glEnd();
		glDeleteTextures(1, &texture[2]);
	}
	glPopMatrix();

	glPushMatrix();
	{//1
		texture[2] = LoadTexture("blackMetal.bmp");
		glBegin(GL_POLYGON);
		glTexCoord2f(0, 1); glVertex3f(-0.17, -0.1, -0.1);
		glTexCoord2f(1, 1); glVertex3f(-0.13, -0.1, -0.1);
		glTexCoord2f(1, 0); glVertex3f(-0.13, -0.12, -0.12);
		glTexCoord2f(0, 0); glVertex3f(-0.17, -0.12, -0.12);
		glEnd();
		//2
		glBegin(GL_POLYGON);
		glTexCoord2f(0, 1); glVertex3f(-0.17, -0.1, -0.1);
		glTexCoord2f(1, 1); glVertex3f(-0.17, -0.08, 0);
		glTexCoord2f(1, 0); glVertex3f(-0.13, -0.08, 0);
		glTexCoord2f(0, 0); glVertex3f(-0.13, -0.1, -0.1);
		glEnd();
		//3
		glBegin(GL_POLYGON);
		glTexCoord2f(0, 1); glVertex3f(-0.13, -0.1, -0.1);
		glTexCoord2f(1, 1); glVertex3f(-0.13, -0.08, 0);
		glTexCoord2f(1, 0); glVertex3f(-0.13, -0.12, 0);
		glTexCoord2f(0, 0); glVertex3f(-0.13, -0.12, -0.12);
		glEnd();
		//4
		glBegin(GL_POLYGON);
		glTexCoord2f(0, 1); glVertex3f(-0.13, -0.12, -0.12);
		glTexCoord2f(1, 1); glVertex3f(-0.13, -0.12, 0);
		glTexCoord2f(1, 0); glVertex3f(-0.17, -0.12, 0);
		glTexCoord2f(0, 0); glVertex3f(-0.17, -0.12, -0.12);
		glEnd();
		//5
		glBegin(GL_POLYGON);
		glTexCoord2f(0, 1); glVertex3f(-0.17, -0.12, -0.12);
		glTexCoord2f(1, 1); glVertex3f(-0.17, -0.12, 0);
		glTexCoord2f(1, 0); glVertex3f(-0.17, -0.08, 0);
		glTexCoord2f(0, 0); glVertex3f(-0.17, -0.1, -0.1);
		glEnd();

		glBegin(GL_POLYGON);
		glTexCoord2f(0, 1); glVertex3f(-0.17, -0.08, 0);
		glTexCoord2f(1, 1); glVertex3f(-0.13, -0.08, 0);
		glTexCoord2f(1, 0); glVertex3f(-0.13, -0.12, 0);
		glTexCoord2f(0, 0); glVertex3f(-0.17, -0.12, 0);
		glEnd();
		glDeleteTextures(1, &texture[2]);
	}
	glPopMatrix();

	glPushMatrix();
	{//1
		texture[3] = LoadTexture("greyMetal.bmp");
		glBegin(GL_POLYGON);
		glTexCoord2f(0, 1); glVertex3f(-0.17, -0.12, -0.14);
		glTexCoord2f(1, 1); glVertex3f(-0.13, -0.12, -0.14);
		glTexCoord2f(1, 0); glVertex3f(-0.13, -0.14, -0.14);
		glTexCoord2f(0, 0); glVertex3f(-0.17, -0.14, -0.14);
		glEnd();
		//2
		glBegin(GL_POLYGON);
		glTexCoord2f(0, 1); glVertex3f(-0.17, -0.12, -0.14);
		glTexCoord2f(1, 1); glVertex3f(-0.17, -0.12, 0);
		glTexCoord2f(1, 0); glVertex3f(-0.13, -0.12, 0);
		glTexCoord2f(0, 0); glVertex3f(-0.13, -0.12, -0.14);
		glEnd();
		//3
		glBegin(GL_POLYGON);
		glTexCoord2f(0, 1); glVertex3f(-0.13, -0.12, -0.14);
		glTexCoord2f(1, 1); glVertex3f(-0.13, -0.12, 0);
		glTexCoord2f(1, 0); glVertex3f(-0.13, -0.14, 0);
		glTexCoord2f(0, 0); glVertex3f(-0.13, -0.14, -0.14);
		glEnd();
		//4
		glBegin(GL_POLYGON);
		glTexCoord2f(0, 1); glVertex3f(-0.13, -0.14, -0.14);
		glTexCoord2f(1, 1); glVertex3f(-0.13, -0.14, 0);
		glTexCoord2f(1, 0); glVertex3f(-0.17, -0.14, 0);
		glTexCoord2f(0, 0); glVertex3f(-0.17, -0.14, -0.14);
		glEnd();
		//5
		glBegin(GL_POLYGON);
		glTexCoord2f(0, 1); glVertex3f(-0.17, -0.14, -0.14);
		glTexCoord2f(1, 1); glVertex3f(-0.17, -0.14, 0);
		glTexCoord2f(1, 0); glVertex3f(-0.17, -0.12, 0);
		glTexCoord2f(0, 0); glVertex3f(-0.17, -0.12, -0.14);
		glEnd();

		glBegin(GL_POLYGON);
		glTexCoord2f(0, 1); glVertex3f(-0.17, -0.12, 0);
		glTexCoord2f(1, 1); glVertex3f(-0.13, -0.12, 0);
		glTexCoord2f(1, 0); glVertex3f(-0.13, -0.14, 0);
		glTexCoord2f(0, 0); glVertex3f(-0.17, -0.14, 0);
		glEnd();
		glDeleteTextures(1, &texture[3]);
	}
	glPopMatrix();

	glPushMatrix();
	{//1
		texture[3] = LoadTexture("greyMetal.bmp");
		glBegin(GL_POLYGON);
		glTexCoord2f(0, 1); glVertex3f(-0.17, -0.08, 0);
		glTexCoord2f(1, 1); glVertex3f(-0.13, -0.08, 0);
		glTexCoord2f(1, 0); glVertex3f(-0.13, -0.05, 0);
		glTexCoord2f(0, 0); glVertex3f(-0.17, -0.05, 0);
		glEnd();
		//2
		glBegin(GL_POLYGON);
		glTexCoord2f(0, 1); glVertex3f(-0.17, -0.08, 0);
		glTexCoord2f(1, 1); glVertex3f(-0.17, -0.08, 0.07);
		glTexCoord2f(1, 0); glVertex3f(-0.13, -0.08, 0.07);
		glTexCoord2f(0, 0); glVertex3f(-0.13, -0.08, 0);
		glEnd();
		//3
		glBegin(GL_POLYGON);
		glTexCoord2f(0, 1); glVertex3f(-0.13, -0.08, 0);
		glTexCoord2f(1, 1); glVertex3f(-0.13, -0.08, 0.07);
		glTexCoord2f(1, 0); glVertex3f(-0.13, -0.05, 0.07);
		glTexCoord2f(0, 0); glVertex3f(-0.13, -0.05, 0);
		glEnd();
		//4
		glBegin(GL_POLYGON);
		glTexCoord2f(0, 1); glVertex3f(-0.13, -0.05, 0);
		glTexCoord2f(1, 1); glVertex3f(-0.13, -0.05, 0.07);
		glTexCoord2f(1, 0); glVertex3f(-0.17, -0.05, 0.07);
		glTexCoord2f(0, 0); glVertex3f(-0.17, -0.05, 0);
		glEnd();
		//5
		glBegin(GL_POLYGON);
		glTexCoord2f(0, 1); glVertex3f(-0.17, -0.05, 0);
		glTexCoord2f(1, 1); glVertex3f(-0.17, -0.05, 0.07);
		glTexCoord2f(1, 0); glVertex3f(-0.17, -0.08, 0.07);
		glTexCoord2f(0, 0); glVertex3f(-0.17, -0.08, 0);
		glEnd();

		glBegin(GL_POLYGON);
		glTexCoord2f(0, 1); glVertex3f(-0.17, -0.08, 0.07);
		glTexCoord2f(1, 1); glVertex3f(-0.13, -0.08, 0.07);
		glTexCoord2f(1, 0); glVertex3f(-0.13, -0.05, 0.07);
		glTexCoord2f(0, 0); glVertex3f(-0.17, -0.05, 0.07);
		glEnd();
		glDeleteTextures(1, &texture[3]);
	}
	glPopMatrix();

	glPushMatrix();
	{//1
		glRotatef(180, 0, 1, 0);
		glTranslatef(0.3, 0, -0.05);
		glScalef(1, 1, 0.5);
		glPushMatrix();
		{
			texture[2] = LoadTexture("blackMetal.bmp");
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.17, -0.1, -0.1);
			glTexCoord2f(1, 1); glVertex3f(-0.13, -0.1, -0.1);
			glTexCoord2f(1, 0); glVertex3f(-0.13, -0.12, -0.12);
			glTexCoord2f(0, 0); glVertex3f(-0.17, -0.12, -0.12);
			glEnd();
			//2
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.17, -0.1, -0.1);
			glTexCoord2f(1, 1); glVertex3f(-0.17, -0.08, 0);
			glTexCoord2f(1, 0); glVertex3f(-0.13, -0.08, 0);
			glTexCoord2f(0, 0); glVertex3f(-0.13, -0.1, -0.1);
			glEnd();
			//3
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.13, -0.1, -0.1);
			glTexCoord2f(1, 1); glVertex3f(-0.13, -0.08, 0);
			glTexCoord2f(1, 0); glVertex3f(-0.13, -0.12, 0);
			glTexCoord2f(0, 0); glVertex3f(-0.13, -0.12, -0.12);
			glEnd();
			//4
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.13, -0.12, -0.12);
			glTexCoord2f(1, 1); glVertex3f(-0.13, -0.12, 0);
			glTexCoord2f(1, 0); glVertex3f(-0.17, -0.12, 0);
			glTexCoord2f(0, 0); glVertex3f(-0.17, -0.12, -0.12);
			glEnd();
			//5
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.17, -0.12, -0.12);
			glTexCoord2f(1, 1); glVertex3f(-0.17, -0.12, 0);
			glTexCoord2f(1, 0); glVertex3f(-0.17, -0.08, 0);
			glTexCoord2f(0, 0); glVertex3f(-0.17, -0.1, -0.1);
			glEnd();

			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.17, -0.08, 0);
			glTexCoord2f(1, 1); glVertex3f(-0.13, -0.08, 0);
			glTexCoord2f(1, 0); glVertex3f(-0.13, -0.12, 0);
			glTexCoord2f(0, 0); glVertex3f(-0.17, -0.12, 0);
			glEnd();
			glDeleteTextures(1, &texture[2]);
		}
		glPopMatrix();

		glPushMatrix();
		{//1
			texture[3] = LoadTexture("greyMetal.bmp");
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.17, -0.12, -0.14);
			glTexCoord2f(1, 1); glVertex3f(-0.13, -0.12, -0.14);
			glTexCoord2f(1, 0); glVertex3f(-0.13, -0.14, -0.14);
			glTexCoord2f(0, 0); glVertex3f(-0.17, -0.14, -0.14);
			glEnd();
			//2
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.17, -0.12, -0.14);
			glTexCoord2f(1, 1); glVertex3f(-0.17, -0.12, 0);
			glTexCoord2f(1, 0); glVertex3f(-0.13, -0.12, 0);
			glTexCoord2f(0, 0); glVertex3f(-0.13, -0.12, -0.14);
			glEnd();
			//3
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.13, -0.12, -0.14);
			glTexCoord2f(1, 1); glVertex3f(-0.13, -0.12, 0);
			glTexCoord2f(1, 0); glVertex3f(-0.13, -0.14, 0);
			glTexCoord2f(0, 0); glVertex3f(-0.13, -0.14, -0.14);
			glEnd();
			//4
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.13, -0.14, -0.14);
			glTexCoord2f(1, 1); glVertex3f(-0.13, -0.14, 0);
			glTexCoord2f(1, 0); glVertex3f(-0.17, -0.14, 0);
			glTexCoord2f(0, 0); glVertex3f(-0.17, -0.14, -0.14);
			glEnd();
			//5
			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.17, -0.14, -0.14);
			glTexCoord2f(1, 1); glVertex3f(-0.17, -0.14, 0);
			glTexCoord2f(1, 0); glVertex3f(-0.17, -0.12, 0);
			glTexCoord2f(0, 0); glVertex3f(-0.17, -0.12, -0.14);
			glEnd();

			glBegin(GL_POLYGON);
			glTexCoord2f(0, 1); glVertex3f(-0.17, -0.12, 0);
			glTexCoord2f(1, 1); glVertex3f(-0.13, -0.12, 0);
			glTexCoord2f(1, 0); glVertex3f(-0.13, -0.14, 0);
			glTexCoord2f(0, 0); glVertex3f(-0.17, -0.14, 0);
			glEnd();
			glDeleteTextures(1, &texture[3]);
		}
		glPopMatrix();
	}
	glPopMatrix();
}

void robot()
{
	glPushMatrix();
	{
		glScalef(0.6, 0.6, 0.6);
		drawBody();
		rightArm();
		leftArm();
		rightLeg();
		leftLeg();
	}
	glPopMatrix();
}

void display()
{
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
	glLightfv(GL_LIGHT0, GL_POSITION, pos);
	glEnable(GL_LIGHT0);

	if (lightSwitch == 1)
	{
		glDisable(GL_LIGHT0);
	}


	//projection();
	GLUquadricObj* var = NULL;
	var = gluNewQuadric();

	//glMatrixMode(GL_MODELVIEW); //refer to modelview matrix
	//glLoadIdentity(); //reset modelview matrix
	//glTranslatef(tx, ty, tz);

	robot();


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
		CW_USEDEFAULT, CW_USEDEFAULT, 1200, 1000,
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

		if (goPerspective)
		{
			goPerspectiveView();
			goPerspective = false;
			inWhatProjectionMode = 2;
		}

		if (goOrtho)
		{
			goOrthoView();
			goOrtho = false;
			inWhatProjectionMode = 1;
		}

		if (goBackOrigin) {
			goBackOriginView();
			goBackOrigin = false;
			inWhatProjectionMode = 0;
		}

		display();

		SwapBuffers(hdc);
	}

	UnregisterClass(WINDOW_TITLE, wc.hInstance);

	//hello


	return true;
}
//--------------------------------------------------------------------