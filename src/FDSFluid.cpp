#include <GL\glut.h>
#include "FDSFluid.h"

FDSFluid::FDSFluid(GLint dimension, GLint width, GLint height, GLint depth) {
	size = width * height * depth;
	p = new GLfloat[size];
	vx = new GLfloat[size];
	vy = new GLfloat[size];
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