#include <GL\glew.h>
#include <GL\glut.h>
#include <cmath>
#include "FDSFluid.h"

FDSFluid::FDSFluid(GLint _dimension, GLint _width, GLint _height, GLint _depth, GLint _locationX, GLint _locationY) {
	dimension = _dimension;  width = _width;  height = _height;  depth = _depth;
	locationX = _locationX;  locationY = _locationY;
	size = width * height * depth;
	p = new GLfloat[size];  setFieldValue(p, 10.0f, size);
	vx = new GLfloat[size];  zeroField(vx, size);
	vy = new GLfloat[size];  zeroField(vy, size);
	if (dimension == FLUID_2D) {
		vz = NULL;
	}
	else if (dimension == FLUID_3D) {
		vz = new GLfloat[size];  zeroField(vz, size);
	}
	else {
		// Error
	}
	ink = new GLfloat[size];  zeroField(ink, size);
	fluidDensityVertices = new GLfloat[2 * size];
	fluidDensityColors = new GLfloat[3 * size];
	fluidVelocityVertices = new GLfloat[4 * size];
	fluidVelocityColors = new GLfloat[6 * size];
	for (GLint y = 0; y < height; y++) {
		for (GLint x = 0; x < width; x++) {
			GLint cell = getCell(x, y);
			fluidDensityVertices[2 * cell] = fluidVelocityVertices[4 * cell] = (locationX + x) * GRID_SIZE;
			fluidDensityVertices[2 * cell + 1] = fluidVelocityVertices[4 * cell + 1] = (locationY + y) * GRID_SIZE;
		}
	}
	for (GLint cell = 0; cell < 2 * size; cell++) {
		fluidVelocityColors[3 * cell] = 0.0f;
		fluidVelocityColors[3 * cell + 1] = 1.0f;
		fluidVelocityColors[3 * cell + 2] = 0.0f;
	}
	inkVertices = new GLfloat[8 * size];
	GLfloat halfgrid = GRID_SIZE / 2.0f;
	for (GLint cell = 0; cell < size; cell++) {
		inkVertices[8 * cell] = fluidDensityVertices[2 * cell] - halfgrid;
		inkVertices[8 * cell + 1] = fluidDensityVertices[2 * cell + 1] - halfgrid;
		inkVertices[8 * cell + 2] = fluidDensityVertices[2 * cell] + halfgrid;
		inkVertices[8 * cell + 3] = fluidDensityVertices[2 * cell + 1] - halfgrid;
		inkVertices[8 * cell + 4] = fluidDensityVertices[2 * cell] + halfgrid;
		inkVertices[8 * cell + 5] = fluidDensityVertices[2 * cell + 1] + halfgrid;
		inkVertices[8 * cell + 6] = fluidDensityVertices[2 * cell] - halfgrid;
		inkVertices[8 * cell + 7] = fluidDensityVertices[2 * cell + 1] + halfgrid;
	}
	inkColors = new GLfloat[12 * size];
	ink_decay_open = GL_TRUE;
}

FDSFluid::~FDSFluid() {
	if (p != NULL) { delete p; }
	if (vx != NULL) { delete vx; }
	if (vy != NULL) { delete vy; }
	if (vz != NULL) { delete vz; }
	if (ink != NULL) { delete ink; }
	if (fluidDensityVertices != NULL) { delete fluidDensityVertices; }
	if (fluidDensityColors != NULL) { delete fluidDensityColors; }
	if (fluidVelocityVertices != NULL) { delete fluidVelocityVertices; }
	if (fluidVelocityColors != NULL) { delete fluidVelocityColors; }
	if (inkColors != NULL) { delete inkColors; }
	if (inkVertices != NULL) { delete inkVertices; }
}

GLvoid FDSFluid::reset() {
	setFieldValue(p, 10.0f, size);
	zeroField(vx, size);
	zeroField(vy, size);
	if (dimension == FLUID_3D) {
		zeroField(vz, size);
	}
	zeroField(ink, size);
	ink_decay_open = GL_TRUE;
}

extern GLfloat scale;
extern GLvoid drawString(GLvoid* font, GLint rasterX, GLint rasterY, GLint hudStyle, const char* _Format, ...);

extern GLfloat viewpoint_x;
extern GLfloat viewpoint_y;

extern GLuint fbo;
extern GLuint blurTargetTexture;
extern GLuint shaderProgram;

GLvoid FDSFluid::inkDropAt(GLint x, GLint y, GLfloat amount) {
	inkPointAt(x, y, 0.8f * amount);
	inkPointAt(x - 1, y, 0.7f * amount);
	inkPointAt(x + 1, y, 0.7f * amount);
	inkPointAt(x, y - 1, 0.7f * amount);
	inkPointAt(x, y + 1, 0.7f * amount);
	inkPointAt(x - 1, y - 1, 0.3f * amount);
	inkPointAt(x - 1, y + 1, 0.3f * amount);
	inkPointAt(x + 1, y - 1, 0.3f * amount);
	inkPointAt(x + 1, y + 1, 0.3f * amount);
}

GLvoid FDSFluid::inkLine(GLint x0, GLint y0, GLint x1, GLint y1, GLfloat amount) {
	if (x0 > x1) {
		GLint t;
		t = x0;  x0 = x1;  x1 = t;
		t = y0;  y0 = y1;  y1 = t;
	}
	if (x0 == x1) {
		if (y0 == y1) return;
		GLint step = y0 < y1 ? 1 : -1;
		for (GLint y = y0; y != y1; y += step) {
			inkDropAt(x0, y, amount * 0.8f);
		}
	}
	else if (y0 == y1) {
		GLint step = x0 < x1 ? 1 : -1;
		for (GLint x = x0; x != x1; x += step) {
			inkDropAt(x, y0, amount * 0.8f);
		}
	}
	else {
		if (x1 - x0 > abs(y1 - y0)) {
			GLfloat fstep = GLfloat(y1 - y0) / GLfloat(x1 - x0);
			GLfloat base = GLfloat(y0);
			for (GLint x = x0; x <= x1; x++) {
				inkDropAt(x, GLint(base), amount * 0.8f);
				base += fstep;
			}
		}
		else {
			GLfloat fstep = GLfloat(x1 - x0) / abs(GLfloat(y1 - y0));
			GLfloat base = GLfloat(x0);
			GLint step = y0 < y1 ? 1 : -1;
			for (GLint y = y0; y != y1; y += step) {
				inkDropAt(GLint(base), y, amount * 0.8f);
				base += fstep;
			}
		}
	}
}

GLvoid FDSFluid::inkPointAt(GLint x, GLint y, GLfloat amount) {
	if (x < 0 || x >= width || y < 0 || y >= height) {
		return;
	}
	GLint cell = getCell(x, y);
	ink[cell] += amount;
	if (ink[cell] > (GLfloat)MAX_CELL_INK) {
		ink[cell] = (GLfloat)MAX_CELL_INK;
	}
}

GLvoid FDSFluid::velocityClamp(float maxVelocity) {
	float maxVelocity2 = maxVelocity * maxVelocity;
	for (GLint cell = 0; cell < size; cell++) {
		GLfloat v2 = vx[cell] * vx[cell] + vy[cell] * vy[cell];
		GLfloat v = sqrtf(v2);
		if (v2 > maxVelocity2) {
			vx[cell] = vx[cell] / v * maxVelocity;
			vy[cell] = vy[cell] / v * maxVelocity;
		}
	}
}

GLvoid FDSFluid::render(GLint field) {
	if (field == RENDER_INK) {
		for (GLint cell = 0; cell < 12 * size; cell++) {
			// ºÚÉ«Ä«Ö­
			inkColors[cell] = (ink[cell / 12] - 2.0f) * (ink[cell / 12] - 2.0f) / 4.0f;
			// °×É«Ä«Ö­£¬DebugÓÃ
			// inkColors[cell] = 1.0f - (ink[cell / 12] - 2.0f) * (ink[cell / 12] - 2.0f) / 4.0f;
		}
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
		glVertexPointer(2, GL_FLOAT, 0, inkVertices);
		glColorPointer(3, GL_FLOAT, 0, inkColors);
		glDrawArrays(GL_QUADS, 0, 4 * size);
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
		glUseProgramObjectARB(shaderProgram);
		glBindTexture(GL_TEXTURE_2D, blurTargetTexture);
		glBegin(GL_QUADS);	
			glTexCoord2f(0.0f, 0.0f);
			glVertex2f(-320.0f, -240.0f);
			glTexCoord2f(1.0f, 0.0f);
			glVertex2f(320.0f, -240.0f);
			glTexCoord2f(1.0f, 1.0f);
			glVertex2f(320.0f, 240.0f);
			glTexCoord2f(0.0f, 1.0f);
			glVertex2f(-320.0f, 240.0f);
		glEnd();
		glBindTexture(GL_TEXTURE_2D, 0);
		glUseProgramObjectARB(0);
	}
	else if (field == RENDER_DENSITY) {
		glPointSize(3.0f);
		for (GLint cell = 0; cell < size; cell++) {
			fluidDensityColors[3 * cell] = fluidDensityColors[3 * cell + 1] = fluidDensityColors[3 * cell + 2] = p[cell] / 20.0f;
		}
		glVertexPointer(2, GL_FLOAT, 0, fluidDensityVertices);
		glColorPointer(3, GL_FLOAT, 0, fluidDensityColors);
		glDrawArrays(GL_POINTS, 0, size);
		glPointSize(1.0f);
	}
	else if (field == RENDER_VELOCITY) {
		for (GLint cell = 0; cell < size; cell++) {
			fluidDensityColors[3 * cell] = fluidDensityColors[3 * cell + 1] = fluidDensityColors[3 * cell + 2] = p[cell] / 50.0f;
		}
		glVertexPointer(2, GL_FLOAT, 0, fluidDensityVertices);
		glColorPointer(3, GL_FLOAT, 0, fluidDensityColors);
		glDrawArrays(GL_POINTS, 0, size);
		for (GLint cell = 0; cell < size; cell++) {
			fluidVelocityVertices[4 * cell + 2] = fluidVelocityVertices[4 * cell] + vx[cell] * GRID_SIZE;
			fluidVelocityVertices[4 * cell + 3] = fluidVelocityVertices[4 * cell + 1] + vy[cell] * GRID_SIZE;
		}
		glVertexPointer(2, GL_FLOAT, 0, fluidVelocityVertices);
		glColorPointer(3, GL_FLOAT, 0, fluidVelocityColors);
		glDrawArrays(GL_LINES, 0, 2 * size);
	}
	else {
		// Error
	}
	if (scale >= 32.0f) {
		for (GLint cell = 0; cell < size; cell++) {
			GLint y = cell / width, x = cell % width;
			if (abs((locationX + x) * GRID_SIZE - viewpoint_x) < 10.0f && abs((locationY + y) * GRID_SIZE - viewpoint_y) < 10.0f) {
				if (field == RENDER_INK) {
					drawString(GLUT_BITMAP_9_BY_15, GLint(fluidDensityVertices[2 * cell]), GLint(fluidDensityVertices[2 * cell + 1]) - 1, 0, "ink = %.3f", ink[cell]);
				}
				else {
					drawString(GLUT_BITMAP_9_BY_15, GLint(fluidDensityVertices[2 * cell]), GLint(fluidDensityVertices[2 * cell + 1]) - 1, 0, "p = %.3f", p[cell]);
					drawString(GLUT_BITMAP_9_BY_15, GLint(fluidDensityVertices[2 * cell]), GLint(fluidDensityVertices[2 * cell + 1]) - 2, 0, "vx = %.3f", vx[cell]);
					drawString(GLUT_BITMAP_9_BY_15, GLint(fluidDensityVertices[2 * cell]), GLint(fluidDensityVertices[2 * cell + 1]) - 3, 0, "vy = %.3f", vy[cell]);
				}
			}
		}
	}
}