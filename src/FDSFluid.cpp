#include <GL\glut.h>
#include "FDSFluid.h"

#define NULL							0

FDSFluid::FDSFluid(GLint dimension, GLint width, GLint height, GLint depth) {
	size = width * height * depth;
	density = new GLfloat[size];
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
	if (density != NULL) delete density;
	if (vx != NULL) delete vx;
	if (vy != NULL) delete vy;
	if (vz != NULL) delete vz;
}