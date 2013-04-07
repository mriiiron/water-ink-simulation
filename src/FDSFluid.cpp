#include <GL\glut.h>
#include "FDSFluid.h"

FDSFluid::FDSFluid(GLint _dimension, GLint _width, GLint _height, GLint _depth) {
	dimension = _dimension;  width = _width;  height = _height;  depth = _depth;
	size = width * height * depth;
	p = new GLfloat[size];  zeroField(p, size);
	vx = new GLfloat[size];  zeroField(vx, size);
	vy = new GLfloat[size];  zeroField(vy, size);
	if (dimension == FLUID_2D) {
		vz = NULL;
	}
	else if (dimension == FLUID_3D) {
		vz = new GLfloat[size];
	}
	else {
		// Error
	}
}

FDSFluid::~FDSFluid() {
	if (p != NULL) { delete p; }
	if (vx != NULL) { delete vx; }
	if (vy != NULL) { delete vy; }
	if (vz != NULL) { delete vz; }
}

GLvoid FDSFluid::copyField(GLfloat* src, GLfloat* dst, GLint size) {
	for (GLint i = 0; i < size; i++) {
		dst[i] = src[i];
	}
}

GLvoid FDSFluid::zeroField(GLfloat* src, GLint size) {
	for (GLint i = 0; i < size; i++) {
		src[i] = 0.0f;
	}
}