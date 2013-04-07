#include <iostream>
#include <windows.h>
#include <cmath>
#include <GL\glut.h>
#include "FDSFluid.h"
#include "FDSNonViscousFluid.h"

using namespace std;

#define FPS_PLANNED						40

GLshort* fluidAreaVertices;
GLfloat* fluidAreaColors;

GLfloat fps;
GLint fps_frames = 0;
GLint fps_time = 0;
GLint fps_timebase = 0;
DWORD time1, time2;

FDSNonViscousFluid* nonViscous;

GLvoid glInit() {
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glShadeModel(GL_SMOOTH);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	gluOrtho2D(-320.0, 320.0, -240.0, 240.0);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

GLvoid simulatorInit() {
	glPointSize(2.0f);
	nonViscous = new FDSNonViscousFluid(2, 100, 100, 1);
	nonViscous->setLocation(-50, -50);
	nonViscous->init();
	GLint size = nonViscous->getSize();
	GLint height = nonViscous->getHeight();
	GLint width = nonViscous->getWidth();
	fluidAreaVertices = new GLshort[2 * size];
	fluidAreaColors = new GLfloat[3 * size];
	for (GLint i = 0; i < height; i++) {
		for (GLint j = 0; j < width; j++) {
			fluidAreaVertices[2 * (i * width + j)] = (nonViscous->getLocationX() + j) * 4.0f;
			fluidAreaVertices[2 * (i * width + j) + 1] = (nonViscous->getLocationY() + i) * 4.0f;
		}
	}
	glVertexPointer(2, GL_SHORT, 0, fluidAreaVertices);
	glColorPointer(3, GL_FLOAT, 0, fluidAreaColors);
}

GLvoid paintScene() {

	GLint size = nonViscous->getSize();
	GLint height = nonViscous->getHeight();
	GLint width = nonViscous->getWidth();
	glColor3f(0.03, 0.03, 0.03);
	
	for (GLint i = 0; i < height; i++) {
		for (GLint j = 0; j < width; j++) {
			fluidAreaColors[3 * (i * width + j)]
			= fluidAreaColors[3 * (i * width + j) + 1]
			= fluidAreaColors[3 * (i * width + j) + 2]
			= sqrt(nonViscous->getP(i, j)) / 2.0f;
		}
	}

	glDrawArrays(GL_POINTS, 0, size);

	fps_frames++;
	fps_time = glutGet(GLUT_ELAPSED_TIME);
	if (fps_time - fps_timebase > 1000) {
		fps = fps_frames * 1000.0 / (fps_time - fps_timebase);
		fps_timebase = fps_time;		
		fps_frames = 0;
		cout.precision(2);
		cout << "FPS: " << fps << endl;
	}
	
}

GLvoid updateScene() {
	// nonViscous->update();
}

GLvoid onKeyDown(GLubyte key, int x, int y) {
	switch (key) {
	case 's':
		nonViscous->stimulate();
		break;
	default:
		nonViscous->update();
		break;
	}
}

GLvoid onDisplay() {
	DWORD dtime;
	long sleepTime;
	updateScene();
	glClear(GL_COLOR_BUFFER_BIT);
	paintScene();
	glutSwapBuffers();
	time2 = GetTickCount();
	dtime = time2 - time1;
	sleepTime = 1000 / FPS_PLANNED - dtime;
	if (sleepTime < 2) {
		sleepTime = 2;
	}
	Sleep(sleepTime);
	time1 = GetTickCount();
	glutPostRedisplay();
}

GLvoid onReshape(GLint w, GLint h) {
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
}


int main(int argc, char* argv[]) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	glutInitWindowSize(640, 480);
	glutInitWindowPosition(200, 50);
	glutCreateWindow("Fluid Dynamics Simulation by CaiyZ (Mr.IroN)");
	glutDisplayFunc(&onDisplay);
	glutReshapeFunc(&onReshape);
	glutKeyboardFunc(&onKeyDown);
	glInit();
	simulatorInit();
	time1 = GetTickCount();
	glutMainLoop();
	return 0;
}