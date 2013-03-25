#pragma once


class FDSFluid {

public:

	static const GLint FLUID_2D = 0;
	static const GLint FLUID_3D = 1;

	FDSFluid(GLint dimension = FLUID_2D, GLint width = 100, GLint height = 100, GLint depth = 1);
	~FDSFluid();

private:

	GLint width, height, depth, size;

	GLfloat* density;
	GLfloat* vx;
	GLfloat* vy;
	GLfloat* vz;

};