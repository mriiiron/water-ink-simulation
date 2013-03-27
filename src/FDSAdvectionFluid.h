#pragma once

class FDSAdvectionFluid: public FDSFluid {

public:

	static const GLint ADVECTION_COMMON			= 0;
	static const GLint ADVECTION_STAM			= 1;
	static const GLint ADVECTION_INTERGRATED	= 2;

	GLfloat stamizing;
	GLint interactRange;
	GLfloat ratioP2V;

	FDSAdvectionFluid(GLint dimension, GLint width, GLint height, GLint depth);
	~FDSAdvectionFluid();
	GLvoid stimulate();

	GLvoid update();
	GLvoid render();

private:

	// GLvoid flowExec(GLint x, GLint y, GLfloat sx, GLfloat sy, GLfloat dx, GLfloat dy, GLfloat inflow, GLfloat outflow);

	GLvoid commonAdvection(GLfloat* src, GLfloat* dst, GLfloat intensity);
	GLvoid stamAdvection(GLfloat* src, GLfloat* dst, GLfloat intensity);
	GLvoid intergratedAdvection(GLfloat* src, GLfloat* dst, GLfloat stamizing);

};