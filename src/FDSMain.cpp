#include <GL\glew.h>
#include <GL\glut.h>
#include <iostream>
#include <cstdarg>
#include <windows.h>
#include "FDSFluid.h"
#include "FDSEuler.h"
#include "FDSAdvection.h"

using namespace std;

#define FPS_PLANNED						40

GLfloat fps;
GLint fps_frames = 0;
GLint fps_time = 0;
GLint fps_timebase = 0;
DWORD time1, time2;

FDSAdvectionFluid* advec;

GLint render_field = FDSFluid::RENDER_INK;

GLboolean pause = GL_TRUE;

GLfloat scale = 1.0f;
GLfloat viewpoint_x = 0.0f, viewpoint_y = 0.0f;

GLvoid glInit() {
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glShadeModel(GL_SMOOTH);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glEnable(GL_BLEND);
	glEnable(GL_TEXTURE_2D);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(-320.0, 320.0, -240.0, 240.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

GLvoid simulatorInit() {
	advec = new FDSAdvectionFluid(2, 100, 100, 1, -50, -50);
}

GLvoid drawString(GLvoid* font, GLint rasterX, GLint rasterY, GLint hudStyle, const char* _Format, ...) {
	char text[256];
	va_list ap;
	if (_Format == NULL)
		return;
	va_start(ap, _Format);
	vsprintf(text, _Format, ap);
	va_end(ap);
	if (hudStyle) {
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		glOrtho(0, 640, 0, 480, -1, 1);
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();
	}
	glRasterPos2i(rasterX, rasterY);
	char* pStr = text;
	while (*pStr != '\0') {
		glutBitmapCharacter(font, *pStr);
		pStr++;
	}
	if (hudStyle) {
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
	}
}

GLvoid paintScene() {
	glLoadIdentity();
	glScalef(scale, scale, 1.0f);
	glTranslatef(-viewpoint_x, -viewpoint_y, 0.0f);
	advec->render(render_field);
	fps_frames++;
	fps_time = glutGet(GLUT_ELAPSED_TIME);
	if (fps_time - fps_timebase > 1000) {
		fps = fps_frames * 1000.0 / (fps_time - fps_timebase);
		fps_timebase = fps_time;		
		fps_frames = 0;
	}
	drawString(GLUT_BITMAP_HELVETICA_18, 5, 5, 1, "Current FPS: %.1f", fps);
}

GLvoid updateScene() {
	if (!pause) {
		advec->update();
	}
}

GLvoid onKeyDown(GLubyte key, int x, int y) {
	switch (key) {
	case ' ':
		pause = !pause;
		break;
	case 'i':
		advec->switchInkDecay();
		break;
	case '1':
		render_field = FDSFluid::RENDER_DENSITY;
		break;
	case '2':
		render_field = FDSFluid::RENDER_VELOCITY;
		break;
	case '3':
		render_field = FDSFluid::RENDER_INK;
		break;
	case 'a':
		cout << "fuck" << endl;
	default:
		break;
	}
}

GLvoid onKeyUp(GLubyte key, int x, int y) {

}

GLint coordWindowToWorldX(GLint x_window) {
	GLfloat mouse_offset_x = x_window - 320.0f;
	GLint width = advec->getWidth();
	return GLint((mouse_offset_x + FDSFluid::GRID_SIZE * (width / 2)) / FDSFluid::GRID_SIZE);
}

GLint coordWindowToWorldY(GLint y_window) {
	GLfloat mouse_offset_y = 240.0f - y_window;
	GLint height = advec->getHeight();
	return GLint((mouse_offset_y + FDSFluid::GRID_SIZE * (height / 2)) / FDSFluid::GRID_SIZE);
}

GLint last_x, last_y;

GLvoid onMouseDown(GLint button, GLint state, GLint x, GLint y) {
	if (state == GLUT_DOWN) {
		switch (button) {
		case GLUT_LEFT_BUTTON:
			last_x = coordWindowToWorldX(x);
			last_y = coordWindowToWorldY(y);
			break;
		case GLUT_RIGHT_BUTTON:
			break;
		default:
			break;
		}
	}
}

GLvoid onMouseDrag(GLint x, GLint y) {
	GLint cell_x = coordWindowToWorldX(x);
	GLint cell_y = coordWindowToWorldY(y);
	if (cell_x < 0 || cell_x >= advec->getWidth() || cell_y < 0 || cell_y >= advec->getHeight()) {
		return;
	}
	advec->inkLine(last_x, last_y, cell_x, cell_y, 2.0f);
	advec->inkDropAt(cell_x, cell_y, 2.0f);
	last_x = cell_x;  last_y = cell_y;
}

GLvoid onDisplay() {
	DWORD dtime;
	long sleepTime;
	updateScene();
	if (render_field == FDSFluid::RENDER_INK) {
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	}
	else {
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	}
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

#define FBO_LOAD_SUCCESS					0
#define FBO_LOAD_FAILED						-1

GLuint fbo = 0;
GLuint blurTargetTexture;

GLint installFrameBufferObject() {
	glGenFramebuffersEXT(1, &fbo);
	glGenTextures(1, &blurTargetTexture);
	glBindTexture(GL_TEXTURE_2D, blurTargetTexture);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);  
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);  
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);  
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);  
	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 640, 480, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, blurTargetTexture, 0);
	if (glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT) != GL_FRAMEBUFFER_COMPLETE_EXT) {
		return FBO_LOAD_FAILED;
	}
	GLenum fboStatus = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);  
	if (fboStatus != GL_FRAMEBUFFER_COMPLETE_EXT) {
		return FBO_LOAD_FAILED;
	}
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	return FBO_LOAD_SUCCESS;
}

#define SHADER_LOAD_SUCCESS					0
#define SHADER_COMPILE_FAILED				-1
#define SHADER_LINK_FAILED					-2
#define SHADER_SOURCE_LOAD_FAILED			-3

GLcharARB* readShaderSourceFromFile(const GLchar *fileName) {
	GLsizei length;
	FILE* fp = fopen(fileName, "r");
	if (fp == NULL) {
		cerr << "Could not open shader source file: " << fileName << endl;
		fclose(fp);  return NULL;
	}
	fseek(fp, 0x0000, SEEK_END);
	length = ftell(fp);
	GLcharARB* sourceBuffer = (GLcharARB*)malloc(length);
	if (sourceBuffer == NULL) {
		cerr << "Could not allocate read buffer for shader source file: " << fileName << endl;
		fclose(fp);  return NULL;
	}
	fseek(fp, 0x0000, SEEK_SET);
	if (fread((void*)sourceBuffer, sizeof(GLcharARB), length, fp) < 0) {
		cerr << "Could not read file: " << fileName << endl;
		fclose(fp);  return NULL;
	}
	fseek(fp, 0x0000, SEEK_SET);
	length = 0;
	while (fgetc(fp) != EOF) {
		length++;
	}
	sourceBuffer[length] = '\0';
	fclose(fp);
	return sourceBuffer;
}

GLuint shaderProgram = 0;

GLint installShaders(const GLcharARB* vertexShaderFileName, const GLcharARB* fragmentShaderFileName) {
	shaderProgram = glCreateProgramObjectARB();
	if (vertexShaderFileName != NULL) {
		GLhandleARB vShader = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
		const GLcharARB* vertexShaderString = readShaderSourceFromFile(vertexShaderFileName);
		glShaderSourceARB(vShader, 1, &vertexShaderString, NULL);
		GLint vertCompiled;
		glCompileShaderARB(vShader);
		glGetObjectParameterivARB(vShader, GL_OBJECT_COMPILE_STATUS_ARB, &vertCompiled);
		if (!vertCompiled) {
			GLchar info[4096];
			glGetInfoLogARB(vShader, 4096, NULL, info);
			cerr << "Error occured compiling vertex shader:" << endl << info << endl;
			return SHADER_COMPILE_FAILED;
		}
		glBindAttribLocationARB(shaderProgram, 1, "alpha");
		glAttachObjectARB(shaderProgram, vShader);
	}
	if (fragmentShaderFileName != NULL) {
		GLhandleARB fShader = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
		const GLcharARB* fragmentShaderString = readShaderSourceFromFile(fragmentShaderFileName);
		glShaderSourceARB(fShader, 1, &fragmentShaderString, NULL);
		GLint fragCompiled;
		glCompileShaderARB(fShader);
		glGetObjectParameterivARB(fShader, GL_OBJECT_COMPILE_STATUS_ARB, &fragCompiled);
		if (!fragCompiled) {
			GLchar ch[4096];
			glGetInfoLogARB(fShader, 4096, NULL, ch);
			cerr << "Error occured compiling fragment shader:" << endl << ch << endl;
			return SHADER_COMPILE_FAILED;
		}
		glAttachObjectARB(shaderProgram, fShader);
	}
	glLinkProgramARB(shaderProgram);
	GLint linked;
	glGetObjectParameterivARB(shaderProgram, GL_OBJECT_LINK_STATUS_ARB, &linked);
	if (!linked) {
		return SHADER_LINK_FAILED;
	}
	glUniform1iARB(glGetUniformLocationARB(shaderProgram, "tex"), blurTargetTexture);
	return SHADER_LOAD_SUCCESS;
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
	glutKeyboardUpFunc(&onKeyUp);
	glutMouseFunc(&onMouseDown);
	glutMotionFunc(&onMouseDrag);
	glInit();
	GLenum err = glewInit();
	if (err != GLEW_OK) {
		cerr << "Error: " << glewGetErrorString(err) << endl;
	}
	cout << "GLEW Status: Using GLEW " << glewGetString(GLEW_VERSION) << endl << endl;
	simulatorInit();
	time1 = GetTickCount();
	if (installFrameBufferObject() != FBO_LOAD_SUCCESS) {
		cerr << "Failed creating FBO." << endl;
	}
	else {
		cout << "Succeeded creating FBO." << endl;
	}
	if (installShaders("src\\shader\\Blur.vshader.txt", "src\\shader\\Blur.fshader.txt") != SHADER_LOAD_SUCCESS) {
		cerr << "Failed loading shader(s)." << endl;
	}
	else {
		cout << "Succeeded loading shader(s)." << endl;
	}
	glutMainLoop();
	return 0;
}