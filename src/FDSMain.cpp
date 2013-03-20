#include <iostream>
#include <windows.h>
#include <cmath>
#include <GL\glut.h>
#include "FDSFluid.h"

using namespace std;

#define FPS_PLANNED						40

GLushort* fluidAreaIndexes;
GLshort* fluidAreaVertices;
GLfloat* fluidAreaColors;

GLfloat fps;
GLint fps_frames = 0;
GLint fps_time = 0;
GLint fps_timebase = 0;
DWORD time1, time2;

FDSFluid fluid;

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
	GLint size = FDSFluid::GRID_SIZE;
	fluidAreaIndexes = new GLushort[size * size];
	fluidAreaVertices = new GLshort[2 * size * size];
	fluidAreaColors = new GLfloat[3 * size * size];
	for (GLint i = 0; i < size; i++) {
		for (GLint j = 0; j < size; j++) {
			fluidAreaIndexes[i * size + j] = i * size + j;
			fluidAreaVertices[2 * (i * size + j)] = (fluid.baseX + j) * 4.0f;
			fluidAreaVertices[2 * (i * size + j) + 1] = (fluid.baseY + i) * 4.0f;
		}
	}
	glVertexPointer(2, GL_SHORT, 0, fluidAreaVertices);
	glColorPointer(3, GL_FLOAT, 0, fluidAreaColors);
}

GLvoid paintScene() {

	GLint size = FDSFluid::GRID_SIZE;
	glColor3f(0.03, 0.03, 0.03);
	
	for (GLint i = 0; i < size; i++) {
		for (GLint j = 0; j < size; j++) {
			fluidAreaColors[3 * (i * size + j)]
			= fluidAreaColors[3 * (i * size + j) + 1]
			= fluidAreaColors[3 * (i * size + j) + 2]
			= sqrt(fluid.getDensity(i, j)) / 2.0f;
		}
	}

	glDrawElements(GL_POINTS, size * size, GL_UNSIGNED_SHORT, fluidAreaIndexes);

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
	// fluid.update();
}

GLvoid onKeyDown(GLubyte key, int x, int y) {
	switch (key) {
	case 's':
		fluid.stimulate();
		break;
	default:
		fluid.update();
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